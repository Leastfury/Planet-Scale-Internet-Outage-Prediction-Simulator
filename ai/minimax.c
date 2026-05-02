#include "minimax.h"
#include "astar.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    
    GameState state;
    state.failed_count = 0;
    
    for (int turn = 1; turn <= 5; turn++) {
        printf("\n--- Turn %d ---\n", turn);
        
        printf("Attacker is thinking...\n");
        int best_attack = -1;
        float max_eval = -999999.0f;
        
        for (int i = 0; i < g->node_count; i++) {
            int already_failed = 0;
            for (int j = 0; j < state.failed_count; j++) {
                if (state.failed_nodes[j] == i) { already_failed = 1; break; }
            }
            if (already_failed) continue;
            
            state.failed_nodes[state.failed_count++] = i;
            // depth 2 allows 1 defender response and 1 next attack eval
            float eval = minimax(&state, g, 2, -999999.0f, 999999.0f, 0); 
            state.failed_count--;
            
            if (eval > max_eval) {
                max_eval = eval;
                best_attack = i;
            }
        }
        
        if (best_attack != -1) {
            state.failed_nodes[state.failed_count++] = best_attack;
            printf("Attacker strikes Node %d (%s)!\n", best_attack, g->nodes[best_attack].name);
        }
        
        if (state.failed_count > 0) {
            printf("Defender is thinking...\n");
            int best_defense_idx = -1;
            float min_eval = 999999.0f;
            
            for (int i = 0; i < state.failed_count; i++) {
                int fixed_node = state.failed_nodes[i];
                state.failed_nodes[i] = state.failed_nodes[state.failed_count - 1];
                state.failed_count--;
                
                float eval = minimax(&state, g, 2, -999999.0f, 999999.0f, 1);
                
                state.failed_count++;
                state.failed_nodes[state.failed_count - 1] = state.failed_nodes[i];
                state.failed_nodes[i] = fixed_node;
                
                if (eval < min_eval) {
                    min_eval = eval;
                    best_defense_idx = i;
                }
            }
            
            if (best_defense_idx != -1) {
                int restored = state.failed_nodes[best_defense_idx];
                printf("Defender restores Node %d (%s)!\n", restored, g->nodes[restored].name);
                state.failed_nodes[best_defense_idx] = state.failed_nodes[state.failed_count - 1];
                state.failed_count--;
            }
        }
        
        float current_score = evaluate_state(&state, g);
        printf("Current Reachability Score: %.0f\n", -current_score);
    }
    printf("\n[MINIMAX] Adversarial Simulation Complete.\n");
}
