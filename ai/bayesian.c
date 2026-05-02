#include "bayesian.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void build_cpt_from_topology(Graph* g, int node_id, float* cpt) {
    (void)g;
    (void)node_id;
    for (int i = 0; i < 16; i++) {
        int failed_parents = 0;
        for (int b = 0; b < 4; b++) {
            if ((i >> b) & 1) failed_parents++;
        }
        
        switch (failed_parents) {
            case 0: cpt[i] = 0.01f; break; 
            case 1: cpt[i] = 0.30f; break;
            case 2: cpt[i] = 0.60f; break;
            case 3: cpt[i] = 0.85f; break;
            case 4: cpt[i] = 0.95f; break;
            default: cpt[i] = 0.01f; break;
        }
    }
}

BayesNetwork build_bayesian_network(Graph* g) {
    BayesNetwork bn;
    bn.node_count = g->node_count;
    if (bn.node_count > MAX_BAYES_NODES) bn.node_count = MAX_BAYES_NODES;
    
    for (int i = 0; i < bn.node_count; i++) {
        bn.nodes[i].network_node_id = i;
        bn.nodes[i].prior = 0.01f;
        bn.nodes[i].parent_count = 0;
        
        int parents_found = 0;
        for (int j = 0; j < g->adj_count[i] && parents_found < 4; j++) {
            int edge_idx = g->adj[i][j];
            int neighbor = (g->edges[edge_idx].from == i) ? g->edges[edge_idx].to : g->edges[edge_idx].from;
            if (neighbor < i) { // DAG condition
                bn.nodes[i].parents[parents_found++] = neighbor;
            }
        }
        bn.nodes[i].parent_count = parents_found;
        
        build_cpt_from_topology(g, i, bn.nodes[i].cpt);
    }
    
    return bn;
}

float bayesian_query(BayesNetwork* bn, int target, int* evidence, int ev_count) {
    int NUM_SAMPLES = 1000;
    int target_failed_count = 0;
    
    int* state = (int*)malloc(sizeof(int) * bn->node_count);
    
    for (int s = 0; s < NUM_SAMPLES; s++) {
        for (int i = 0; i < bn->node_count; i++) {
            int is_ev = 0;
            for (int e = 0; e < ev_count; e++) {
                if (evidence[e] == i) {
                    state[i] = 1; // Failed
                    is_ev = 1;
                    break;
                }
            }
            
            if (!is_ev) {
                if (bn->nodes[i].parent_count == 0) {
                    float r = (float)rand() / (float)RAND_MAX;
                    state[i] = (r < bn->nodes[i].prior) ? 1 : 0;
                } else {
                    int cpt_index = 0;
                    for (int p = 0; p < bn->nodes[i].parent_count; p++) {
                        int parent_id = bn->nodes[i].parents[p];
                        if (state[parent_id] == 1) {
                            cpt_index |= (1 << p);
                        }
                    }
                    float prob = bn->nodes[i].cpt[cpt_index];
                    float r = (float)rand() / (float)RAND_MAX;
                    state[i] = (r < prob) ? 1 : 0;
                }
            }
        }
        if (state[target] == 1) {
            target_failed_count++;
        }
    }
    
    free(state);
    return (float)target_failed_count / (float)NUM_SAMPLES;
}

int* get_top_risk_nodes(BayesNetwork* bn, int* failed_evidence, int ev_count) {
    float* risk = (float*)malloc(sizeof(float) * bn->node_count);
    int* ranked_list = (int*)malloc(sizeof(int) * (bn->node_count + 1));
    
    for (int i = 0; i < bn->node_count; i++) {
        risk[i] = bayesian_query(bn, i, failed_evidence, ev_count);
        ranked_list[i] = i;
    }
    ranked_list[bn->node_count] = -1; // terminator
    
    // Bubble sort for simplicity
    for (int i = 0; i < bn->node_count - 1; i++) {
        for (int j = 0; j < bn->node_count - i - 1; j++) {
            if (risk[ranked_list[j]] < risk[ranked_list[j+1]]) {
                int temp = ranked_list[j];
                ranked_list[j] = ranked_list[j+1];
                ranked_list[j+1] = temp;
            }
        }
    }
    
    free(risk);
    return ranked_list;
}
