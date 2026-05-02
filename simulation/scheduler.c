#include "scheduler.h"
#include "../graph/topology.h"
#include "../failures/failure_engine.h"
#include "../failures/cascade.h"
#include "../ai/astar.h"
#include "../ai/hmm.h"
#include "../ai/bayesian.h"
#include "../ai/qlearning.h"
#include "../simulation/metrics.h"
#include "../output/visualizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#define sleep_s(x) Sleep(1000 * (x))
#else
#define sleep_s(x) sleep(x)
#endif

static HMMState g_hmm;
static BayesNetwork g_bn;
static QLearning g_ql;
static int g_ai_initialized = 0;

Scheduler* init_scheduler() {
    if (!g_ai_initialized) {
        init_hmm(&g_hmm);
        load_qtable(&g_ql, "qtable.bin");
        g_ai_initialized = 1;
    }
    Scheduler* s = (Scheduler*)malloc(sizeof(Scheduler));
    s->head = NULL;
    s->current_time = 0;
    return s;
}

void add_event(Scheduler* s, int time, int type, int target_node, int target_edge, const char* desc) {
    Event* e = (Event*)malloc(sizeof(Event));
    e->time = time;
    e->type = type;
    e->target_node = target_node;
    e->target_edge = target_edge;
    strncpy(e->description, desc, 127);
    e->description[127] = '\0';
    e->next = NULL;

    if (s->head == NULL || s->head->time > time) {
        e->next = s->head;
        s->head = e;
    } else {
        Event* curr = s->head;
        while (curr->next != NULL && curr->next->time <= time) {
            curr = curr->next;
        }
        e->next = curr->next;
        curr->next = e;
    }
}

Event* pop_next_event(Scheduler* s) {
    if (s->head == NULL) return NULL;
    Event* e = s->head;
    s->head = s->head->next;
    return e;
}

void free_scheduler(Scheduler* s) {
    if (!s) return;
    Event* curr = s->head;
    while (curr) {
        Event* next = curr->next;
        free(curr);
        curr = next;
    }
    free(s);
}

void run_simulation(Scheduler* s, Graph* g) {
    while (s->head != NULL) {
        Event* e = pop_next_event(s);
        s->current_time = e->time;
        printf("\n[T+%3dmin] %s\n", e->time, e->description);
        
        // HMM Prediction per tick
        for (int i = 0; i < g->node_count; i++) {
            if (!g->nodes[i].is_active) continue;
            float prob = predict_failure_prob(&g_hmm, g, i, 5); // 5 minutes horizon
            if (prob > 0.6f) {
                char desc[128];
                snprintf(desc, sizeof(desc), "HMM predicted failure for %s (prob: %.2f)", g->nodes[i].name, prob);
                // Ensure we don't spam the queue
                int already_scheduled = 0;
                Event* curr = s->head;
                while(curr) {
                    if (curr->type == EVT_PREDICTED_FAILURE && curr->target_node == i) {
                        already_scheduled = 1;
                        break;
                    }
                    curr = curr->next;
                }
                if (!already_scheduled) {
                    add_event(s, s->current_time + 1, EVT_PREDICTED_FAILURE, i, -1, desc);
                }
            }
        }
        
        switch (e->type) {
            case EVT_FAILURE: {
                apply_node_failure(g, e->target_node);
                schedule_cascades(s, g, e->target_node);
                
                // Q-Learning Action Selection
                int state = discretize_state(g);
                int action = select_action(&g_ql, state, 0); // 0 = not training
                printf("[Q-LEARNING] Controller selected action %d for state %d\n", action, state);
                
                if (action == ACTION_REROUTE) {
                    reroute_traffic_astar(g, e->target_node);
                } else if (action == ACTION_ACTIVATE_BACKUP) {
                    int n = e->target_node;
                    if (n < g->node_count - 1) n++; // Just activate neighbor
                    g->nodes[n].capacity += 100;
                    printf("             Activated backup capacity on node %d\n", n);
                } else if (action == ACTION_THROTTLE) {
                    int n = e->target_node;
                    if (n < g->node_count - 1) n++; 
                    g->nodes[n].current_load /= 2;
                    printf("             Throttled load on node %d\n", n);
                } else if (action == ACTION_INCREASE_CAP) {
                    int n = e->target_node;
                    if (n < g->node_count - 1) n++;
                    g->nodes[n].capacity += 500;
                    printf("             Increased capacity on node %d\n", n);
                } else {
                    printf("             Action: DO NOTHING\n");
                }
                
                // Bayesian Network Cascade Risk
                if (g_ai_initialized == 1) { // Build BN on first failure dynamically or lazily
                    g_bn = build_bayesian_network(g);
                    g_ai_initialized = 2; // Indicates BN is built
                }
                
                // Gather current failures as evidence
                int* failed_evidence = (int*)malloc(sizeof(int) * g->node_count);
                int ev_count = 0;
                for (int i = 0; i < g->node_count; i++) {
                    if (!g->nodes[i].is_active) {
                        failed_evidence[ev_count++] = i;
                    }
                }
                
                // Query neighbors of the failed node
                int failed = e->target_node;
                if (failed >= 0 && failed < g->node_count) {
                    for (int j = 0; j < g->adj_count[failed]; j++) {
                        int edge_idx = g->adj[failed][j];
                        int neighbor = (g->edges[edge_idx].from == failed) ? g->edges[edge_idx].to : g->edges[edge_idx].from;
                        if (g->nodes[neighbor].is_active) {
                            float cascade_prob = bayesian_query(&g_bn, neighbor, failed_evidence, ev_count);
                            if (cascade_prob > 0.6f) {
                                char desc[128];
                                snprintf(desc, sizeof(desc), "Bayesian predicted cascade failure for %s (prob: %.2f)", g->nodes[neighbor].name, cascade_prob);
                                add_event(s, s->current_time + 2, EVT_LIKELY_FAILURE, neighbor, -1, desc);
                            }
                        }
                    }
                }
                free(failed_evidence);
                break;
            }
            case EVT_OVERLOAD:
                if (e->target_edge >= 0 && e->target_edge < g->edge_count) {
                    g->edges[e->target_edge].failure_prob += 0.2f;
                    if (g->edges[e->target_edge].failure_prob > 1.0f) {
                        g->edges[e->target_edge].failure_prob = 1.0f;
                    }
                }
                break;
            case EVT_CASCADE:
                if (e->target_node >= 0 && e->target_node < g->node_count) {
                    apply_node_failure(g, e->target_node);
                }
                break;
            case EVT_RECOVERY:
                restore_node(g, e->target_node);
                break;
            case EVT_PREDICTED_FAILURE:
                printf("[WARNING] AI PREDICTION (HMM): Taking preventive action for node %d\n", e->target_node);
                break;
            case EVT_LIKELY_FAILURE:
                printf("[WARNING] AI PREDICTION (BAYES): Pre-routing traffic away from vulnerable node %d\n", e->target_node);
                reroute_traffic_astar(g, e->target_node); // Pre-route
                break;
            case EVT_REROUTE:
                reroute_traffic_astar(g, e->target_node);
                break;
        }
        
        update_metrics(g);
        print_ascii_map(g);
        free(e);
        sleep_s(1);
    }
}
