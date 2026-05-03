#include "qlearning.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../simulation/scheduler.h"
#include "../ai/astar.h"

void init_qlearning(QLearning* ql) {
    ql->learning_rate = 0.1f;
    ql->discount = 0.9f;
    ql->epsilon = 0.2f;
    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < 5; j++) {
            ql->Q[i][j] = 0.0f;
        }
    }
}

int discretize_state(Graph* g) {
    int failed = 0;
    float max_load = 0.0f;
    for (int i = 0; i < g->node_count; i++) {
        if (!g->nodes[i].is_active) failed++;
        if (g->nodes[i].capacity > 0) {
            float l = (float)g->nodes[i].current_load / g->nodes[i].capacity;
            if (l > max_load) max_load = l;
        }
    }
    
    int bucket_fail = failed;
    if (bucket_fail > 9) bucket_fail = 9;
    
    int bucket_load = (int)(max_load * 100);
    if (bucket_load > 99) bucket_load = 99;
    
    int state = (bucket_fail * 100) + bucket_load;
    if (state >= 1000) state = 999;
    return state;
}

int select_action(QLearning* ql, int state, int is_training) {
    if (is_training) {
        float r = (float)rand() / (float)RAND_MAX;
        if (r < ql->epsilon) {
            return rand() % 5;
        }
    }

    /* Fix: if all Q values are equal (e.g. all zero = untrained state),
       pick randomly instead of always defaulting to action 0 */
    int all_equal = 1;
    for (int a = 1; a < 5; a++) {
        if (ql->Q[state][a] != ql->Q[state][0]) { all_equal = 0; break; }
    }
    if (all_equal) return rand() % 5;

    int best_action = 0;
    float best_q = ql->Q[state][0];
    for (int a = 1; a < 5; a++) {
        if (ql->Q[state][a] > best_q) {
            best_q = ql->Q[state][a];
            best_action = a;
        }
    }
    return best_action;
}

void update_q(QLearning* ql, int state, int action, float reward, int next_state) {
    float max_next_q = ql->Q[next_state][0];
    for (int a = 1; a < 5; a++) {
        if (ql->Q[next_state][a] > max_next_q) max_next_q = ql->Q[next_state][a];
    }
    
    ql->Q[state][action] = ql->Q[state][action] + ql->learning_rate * (reward + ql->discount * max_next_q - ql->Q[state][action]);
}

void train_episodes(Graph* g, int episodes) {
    printf("\n[Q-LEARNING] Starting training for %d episodes...\n", episodes);
    QLearning ql;
    init_qlearning(&ql);
    g_silent_mode = 1;

    int print_interval = episodes / 10;
    if (print_interval == 0) print_interval = 1;

    for (int ep = 0; ep < episodes; ep++) {
        /* Reset all nodes/edges to active */
        for (int i = 0; i < g->node_count; i++) {
            g->nodes[i].is_active = 1;
            /* Fix 2: randomize starting load so load-based states get explored */
            if (g->nodes[i].capacity > 0) {
                g->nodes[i].current_load = (rand() % g->nodes[i].capacity);
            }
        }
        for (int i = 0; i < g->edge_count; i++) g->edges[i].is_active = 1;

        /* Fix 3: decay epsilon over episodes so agent exploits more as it learns */
        ql.epsilon = 0.8f * (1.0f - (float)ep / episodes) + 0.05f;

        int state = discretize_state(g);

        /* Fix 1: more steps per episode for better coverage */
        for (int step = 0; step < 100; step++) {
            /* Randomly break a node (20% chance — increased from 10%) */
            if (rand() % 5 == 0) {
                int target = rand() % g->node_count;
                g->nodes[target].is_active = 0;
            }

            /* Randomly vary load to explore load-based states */
            int ln = rand() % g->node_count;
            if (g->nodes[ln].capacity > 0) {
                g->nodes[ln].current_load = rand() % g->nodes[ln].capacity;
            }

            int action = select_action(&ql, state, 1);

            if (action == ACTION_REROUTE) {
                reroute_traffic_astar(g, rand() % g->node_count);
            } else if (action == ACTION_ACTIVATE_BACKUP) {
                int n = rand() % g->node_count;
                g->nodes[n].capacity += 100;
            } else if (action == ACTION_THROTTLE) {
                int n = rand() % g->node_count;
                if (g->nodes[n].current_load > 0)
                    g->nodes[n].current_load /= 2;
            } else if (action == ACTION_INCREASE_CAP) {
                int n = rand() % g->node_count;
                g->nodes[n].capacity += 500;
            }

            int next_state = discretize_state(g);

            /* Reward: penalize failures AND high load */
            int failed = 0;
            float max_load = 0.0f;
            for (int i = 0; i < g->node_count; i++) {
                if (!g->nodes[i].is_active) failed++;
                if (g->nodes[i].capacity > 0) {
                    float l = (float)g->nodes[i].current_load / g->nodes[i].capacity;
                    if (l > max_load) max_load = l;
                }
            }
            float reward = -((float)failed * 10.0f) - (max_load * 5.0f);

            update_q(&ql, state, action, reward, next_state);
            state = next_state;
        }

        if (ep % print_interval == 0) {
            printf("Episode %d/%d completed. (epsilon=%.2f)\n", ep, episodes, ql.epsilon);
        }
    }

    g_silent_mode = 0;
    save_qtable(&ql, "qtable.bin");
    printf("[Q-LEARNING] Training complete. Q-Table saved to 'qtable.bin'\n");
}

void save_qtable(QLearning* ql, const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (f) {
        fwrite(ql, sizeof(QLearning), 1, f);
        fclose(f);
    }
}

void load_qtable(QLearning* ql, const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (f) {
        size_t read_bytes = fread(ql, sizeof(QLearning), 1, f);
        if (read_bytes != 1) {
            init_qlearning(ql);
        }
        fclose(f);
    } else {
        init_qlearning(ql);
    }
}
