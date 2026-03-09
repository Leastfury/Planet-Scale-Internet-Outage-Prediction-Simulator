#ifndef EXPERT_SYSTEM_H
#define EXPERT_SYSTEM_H

#include "../graph/topology.h"

typedef struct Rule {
    char  condition_desc[128];
    char  prediction[256];
    char  recommendation[256];
    int   risk_boost;
} Rule;

extern Rule rules[8];

int evaluate_node(Graph* g, int node_id);
void run_expert_system(Graph* g);

#endif
