#include "minimax.h"
#include "astar.h"
#include "heuristics.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int attacked_history[MAX_MINIMAX_NODES];
static int attacked_history_count = 0;

static int count_reachable_active_undirected(Graph* g, int src) {
    if (src < 0 || src >= g->node_count) return 0;
    if (!g->nodes[src].is_active) return 0;

    int* visited = (int*)calloc(g->node_count, sizeof(int));
    int* queue = (int*)malloc(sizeof(int) * g->node_count);
    if (!visited || !queue) {
        free(visited);
        free(queue);
        return 0;
    }

    int head = 0;
    int tail = 0;
    int count = 0;

    visited[src] = 1;
    queue[tail++] = src;

    while (head < tail) {
        int u = queue[head++];
        count++;

        for (int eidx = 0; eidx < g->edge_count; eidx++) {
            Edge e = g->edges[eidx];
            if (!e.is_active) continue;
            if (!g->nodes[e.from].is_active || !g->nodes[e.to].is_active) continue;

            int v = -1;
            if (e.from == u) v = e.to;
            else if (e.to == u) v = e.from;
            if (v == -1 || visited[v]) continue;

            visited[v] = 1;
            queue[tail++] = v;
        }
    }

    free(visited);
    free(queue);
    return count;
}

static float network_reachability(Graph* g) {
    int active_count = 0;
    float sum = 0.0f;

    for (int i = 0; i < g->node_count; i++) {
        if (!g->nodes[i].is_active) continue;
        active_count++;
    }

    if (active_count == 0) return 0.0f;

    for (int i = 0; i < g->node_count; i++) {
        if (!g->nodes[i].is_active) continue;
        int reachable = count_reachable_active_undirected(g, i);
        sum += (float)reachable / (float)active_count;
    }

    return sum / (float)active_count;
}

float evaluate_state(GameState* state, Graph* g) {
    int temp_status[MAX_MINIMAX_NODES];
    for (int i = 0; i < g->node_count; i++) temp_status[i] = g->nodes[i].is_active;
    
    for (int i = 0; i < state->failed_count; i++) {
        int fn = state->failed_nodes[i];
        if (fn >= 0 && fn < g->node_count) {
            g->nodes[fn].is_active = 0;
        }
    }
    
    int reachable = count_reachable_astar(g, 0);
    
    for (int i = 0; i < g->node_count; i++) g->nodes[i].is_active = temp_status[i];
    
    return -(float)reachable;
}

float minimax(GameState* state, Graph* g, int depth, float alpha, float beta, int is_maximizing) {
    if (depth == 0) {
        return evaluate_state(state, g);
    }
    
    if (is_maximizing) {
        float max_eval = -999999.0f;
        for (int i = 0; i < g->node_count; i++) {
            int already_failed = 0;
            for (int j = 0; j < state->failed_count; j++) {
                if (state->failed_nodes[j] == i) { already_failed = 1; break; }
            }
            if (already_failed) continue;
            
            state->failed_nodes[state->failed_count++] = i;
            float eval = minimax(state, g, depth - 1, alpha, beta, 0);
            state->failed_count--;
            
            if (eval > max_eval) max_eval = eval;
            if (eval > alpha) alpha = eval;
            if (beta <= alpha) break;
        }
        return max_eval;
    } else {
        float min_eval = 999999.0f;
        if (state->failed_count == 0) {
            return evaluate_state(state, g);
        }
        for (int i = 0; i < state->failed_count; i++) {
            int fixed_node = state->failed_nodes[i];
            state->failed_nodes[i] = state->failed_nodes[state->failed_count - 1];
            state->failed_count--;
            
            float eval = minimax(state, g, depth - 1, alpha, beta, 1);
            
            state->failed_count++;
            state->failed_nodes[state->failed_count - 1] = state->failed_nodes[i];
            state->failed_nodes[i] = fixed_node;
            
            if (eval < min_eval) min_eval = eval;
            if (eval < beta) beta = eval;
            if (beta <= alpha) break;
        }
        return min_eval;
    }
}

void run_adversarial_simulation(Graph* g) {
    printf("\n[MINIMAX] Starting Adversarial Stress Test...\n");
    printf("Role 1: Attacker (AI) aims to maximize network partition.\n");
    printf("Role 2: Defender (AI) aims to restore critical nodes.\n");

    int original_status[MAX_MINIMAX_NODES];
    int original_edge_status[MAX_MINIMAX_NODES * 4];
    for (int i = 0; i < g->node_count; i++) {
        original_status[i] = g->nodes[i].is_active;
    }
    for (int i = 0; i < g->edge_count && i < (MAX_MINIMAX_NODES * 4); i++) {
        original_edge_status[i] = g->edges[i].is_active;
    }
    for (int i = 0; i < g->node_count; i++) {
        g->nodes[i].is_active = 1;
    }
    for (int i = 0; i < g->edge_count; i++) {
        g->edges[i].is_active = 1;
    }
    printf("  [Note: Simulation runs on clean network state]\n");
    
    GameState state;
    state.failed_count = 0;
    attacked_history_count = 0;
    
    for (int turn = 1; turn <= 6; turn++) {
        printf("\n--- Turn %d ---\n", turn);

        if (turn % 2 == 1) {
            int best_attack = -1;
            float best_b = -1.0f;
            float best_reach_if_failed = 2.0f;

            for (int i = 0; i < g->node_count; i++) {
                if (!g->nodes[i].is_active) continue;

                int already_attacked = 0;
                for (int k = 0; k < attacked_history_count; k++) {
                    if (attacked_history[k] == i) {
                        already_attacked = 1;
                        break;
                    }
                }
                if (already_attacked) continue;

                float b = compute_betweenness(g, i);

                int prev = g->nodes[i].is_active;
                g->nodes[i].is_active = 0;
                float reach_if_failed = network_reachability(g);
                g->nodes[i].is_active = prev;

                if (best_attack == -1 || b > best_b || (b == best_b && reach_if_failed < best_reach_if_failed)) {
                    best_attack = i;
                    best_b = b;
                    best_reach_if_failed = reach_if_failed;
                }
            }

            if (best_attack != -1) {
                g->nodes[best_attack].is_active = 0;
                state.failed_nodes[state.failed_count++] = best_attack;
                attacked_history[attacked_history_count++] = best_attack;
                float reach = network_reachability(g);
                printf("[ATTACKER] Cut node %s (score: %.0f) -> Reachability: %.2f\n",
                       g->nodes[best_attack].name,
                       best_b,
                       reach);
            } else {
                printf("[ATTACKER] No active node available to cut.\n");
            }
        } else {
            if (state.failed_count == 0) {
                printf("[DEFENDER] No failed node to restore.\n");
                continue;
            }

            int best_restore_pos = -1;
            float best_reach = -1.0f;
            float best_criticality = -1.0f;

            for (int i = 0; i < state.failed_count; i++) {
                int node_id = state.failed_nodes[i];
                g->nodes[node_id].is_active = 1;
                float reach = network_reachability(g);
                float criticality = compute_betweenness(g, node_id);
                g->nodes[node_id].is_active = 0;

                if (best_restore_pos == -1 || reach > best_reach || (reach == best_reach && criticality > best_criticality)) {
                    best_restore_pos = i;
                    best_reach = reach;
                    best_criticality = criticality;
                }
            }

            if (best_restore_pos != -1) {
                int restored = state.failed_nodes[best_restore_pos];
                g->nodes[restored].is_active = 1;
                state.failed_nodes[best_restore_pos] = state.failed_nodes[state.failed_count - 1];
                state.failed_count--;
                float reach = network_reachability(g);
                printf("[DEFENDER] Restored %s -> Reachability: %.2f\n",
                       g->nodes[restored].name,
                       reach);
            }
        }
    }
    printf("\n[MINIMAX] Adversarial Simulation Complete.\n");

    for (int i = 0; i < g->node_count; i++) {
        g->nodes[i].is_active = original_status[i];
    }
    for (int i = 0; i < g->edge_count && i < (MAX_MINIMAX_NODES * 4); i++) {
        g->edges[i].is_active = original_edge_status[i];
    }
    printf("[MINIMAX] Network state restored to pre-simulation.\n");
}
