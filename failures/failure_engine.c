#include "failure_engine.h"

void apply_node_failure(Graph* g, int node_id) {
    if (node_id < 0 || node_id >= g->node_count) return;
    g->nodes[node_id].is_active = 0;
    
    // Also disable connected edges
    for (int i = 0; i < g->adj_count[node_id]; i++) {
        int edge_idx = g->adj[node_id][i];
        g->edges[edge_idx].is_active = 0;
    }
}

void restore_node(Graph* g, int node_id) {
    if (node_id < 0 || node_id >= g->node_count) return;
    g->nodes[node_id].is_active = 1;
    
    // Also enable connected edges
    for (int i = 0; i < g->adj_count[node_id]; i++) {
        int edge_idx = g->adj[node_id][i];
        g->edges[edge_idx].is_active = 1;
    }
}
