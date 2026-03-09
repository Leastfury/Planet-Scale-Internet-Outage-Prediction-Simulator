#include "cascade.h"
#include <stdlib.h>
#include <stdio.h>

void schedule_cascades(Scheduler* s, Graph* g, int failed_node) {
    if (failed_node < 0 || failed_node >= g->node_count) return;
    
    int load_to_redistribute = g->nodes[failed_node].capacity; 
    
    for (int i = 0; i < g->adj_count[failed_node]; i++) {
        int edge_idx = g->adj[failed_node][i];
        int neighbor = (g->edges[edge_idx].from == failed_node) ? g->edges[edge_idx].to : g->edges[edge_idx].from;
        
        if (g->nodes[neighbor].is_active) {
            for (int j = 0; j < g->adj_count[neighbor]; j++) {
                int n_edge_idx = g->adj[neighbor][j];
                if (g->edges[n_edge_idx].is_active) {
                    g->edges[n_edge_idx].current_load += load_to_redistribute / 4; 
                    
                    float load_ratio = (float)g->edges[n_edge_idx].current_load / (float)g->edges[n_edge_idx].bandwidth;
                    
                    if (load_ratio > 1.0f) {
                        char desc[128];
                        snprintf(desc, sizeof(desc), "Cascade failure: Edge %d overwhelmed", n_edge_idx);
                        add_event(s, s->current_time + (rand() % 10) + 10, EVT_CASCADE, neighbor, n_edge_idx, desc);
                    } else if (load_ratio > 0.8f) {
                        char desc[128];
                        snprintf(desc, sizeof(desc), "Overload detected on edge %d", n_edge_idx);
                        add_event(s, s->current_time + (rand() % 10) + 5, EVT_OVERLOAD, neighbor, n_edge_idx, desc);
                    }
                }
            }
        }
    }
}
