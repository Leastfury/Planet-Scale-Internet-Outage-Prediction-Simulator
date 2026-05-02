#include "heuristics.h"
#include "astar.h"
#include <stdio.h>
#include <stdlib.h>

// Classical AI technique: Network Heuristics (Betweenness Centrality)
// Measures how often a node acts as a bridge along the shortest path between two other nodes.

float compute_betweenness(Graph* g, int node_id) {
    if(g->node_count == 0) return 0.0f;
    int paths_through_node = 0;
    
    int reach_with = count_reachable_astar(g, 0); 
    
    int temp = g->nodes[node_id].is_active;
    g->nodes[node_id].is_active = 0;
    int reach_without = count_reachable_astar(g, 0);
    g->nodes[node_id].is_active = temp;
    
    if (reach_with > reach_without) {
        paths_through_node = (reach_with - reach_without) + (g->node_count / 10);
    }
    
    int total_pairs = g->node_count;
    
    if (total_pairs == 0) return 0;
    return ((float)paths_through_node / total_pairs) * 100.0f;
}

void score_all_nodes(Graph* g) {
    for (int i = 0; i < g->node_count; i++) {
        float b = compute_betweenness(g, i);
        g->nodes[i].risk_score = b;
        
        if (g->adj_count[i] <= 1) {
            g->nodes[i].risk_score += 20;
        }
        if (g->nodes[i].capacity > 0 && g->nodes[i].current_load > g->nodes[i].capacity * 0.7) {
            g->nodes[i].risk_score += 15;
        }
        if (g->nodes[i].type == SUBMARINE_CABLE && g->nodes[i].backup_links < 2) {
            g->nodes[i].risk_score += 10;
        }
        
        if (g->nodes[i].risk_score > 100.0f) g->nodes[i].risk_score = 100.0f;
    }
}

const char* get_risk_label(float score) {
    if (score >= 75) return "CRITICAL";
    if (score >= 50) return "HIGH";
    if (score >= 25) return "MEDIUM";
    return "LOW";
}
