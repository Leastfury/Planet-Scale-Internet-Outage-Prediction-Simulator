#include "scheduler.h"
#include "../graph/topology.h"
#include "../failures/failure_engine.h"
#include "../failures/cascade.h"
#include "../ai/dijkstra.h"
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

Scheduler* init_scheduler() {
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
        
        switch (e->type) {
            case EVT_FAILURE:
                apply_node_failure(g, e->target_node);
                schedule_cascades(s, g, e->target_node);
                reroute_traffic(g, e->target_node);
                break;
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
            case EVT_REROUTE:
                reroute_traffic(g, e->target_node);
                break;
        }
        
        update_metrics(g);
        print_ascii_map(g);
        free(e);
        sleep_s(1);
    }
}
