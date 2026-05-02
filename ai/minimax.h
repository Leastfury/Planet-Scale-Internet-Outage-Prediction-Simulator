#ifndef MINIMAX_H
#define MINIMAX_H

#include "../graph/topology.h"

#define MAX_MINIMAX_NODES 1000

typedef struct GameState {
    int failed_nodes[MAX_MINIMAX_NODES];
    int failed_count;
    int depth;
    int is_attacker_turn;
} GameState;

float evaluate_state(GameState* state, Graph* g);
float minimax(GameState* state, Graph* g, int depth, float alpha, float beta, int is_maximizing);
void run_adversarial_simulation(Graph* g);

#endif
