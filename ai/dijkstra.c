#include "dijkstra.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Classical AI technique: Dijkstra's Algorithm for Uniform Cost Search
// Used to find the shortest path (lowest latency) between networking nodes to reroute traffic.

int* dijkstra(Graph* g, int src) {
    int* dist = (int*)malloc(g->node_capacity * sizeof(int));
    int* visited = (int*)calloc(g->node_capacity, sizeof(int));
    
    for (int i = 0; i < g->node_count; i++) {
        dist[i] = INT_MAX;
    }
    dist[src] = 0;
    
    for (int count = 0; count < g->node_count - 1; count++) {
        int u = -1;
        int min_dist = INT_MAX;
        
        for (int i = 0; i < g->node_count; i++) {
            if (!visited[i] && dist[i] < min_dist && g->nodes[i].is_active) {
                min_dist = dist[i];
                u = i;
            }
        }
        
        if (u == -1) break;
        visited[u] = 1;
        
        for (int i = 0; i < g->adj_count[u]; i++) {
            int edge_idx = g->adj[u][i];
            Edge e = g->edges[edge_idx];
            
            if (!e.is_active) continue;
            
            int v = (e.from == u) ? e.to : e.from;
            if (g->nodes[v].is_active && !visited[v] && dist[u] != INT_MAX) {
                if (dist[u] + e.latency < dist[v]) {
                    dist[v] = dist[u] + e.latency;
                }
            }
        }
    }
    
    free(visited);
    return dist;
}

void reroute_traffic(Graph* g, int failed_node) {
    if (failed_node < 0 || failed_node >= g->node_count) return;
    
    int r = g->nodes[failed_node].region_id;
    int dc = -1;
    for (int i = 0; i < g->node_count; i++) {
        if (g->nodes[i].region_id == r && g->nodes[i].type == DATACENTER && i != failed_node && g->nodes[i].is_active) {
            dc = i;
            break;
        }
    }
    
    if (dc != -1) {
        int* dist = dijkstra(g, dc);
        int other_dc = -1;
        for (int i = 0; i < g->node_count; i++) {
            if (g->nodes[i].region_id != r && g->nodes[i].type == DATACENTER && g->nodes[i].is_active) {
                other_dc = i;
                break;
            }
        }
        
        if (other_dc != -1) {
            if (dist[other_dc] != INT_MAX) {
                printf("[REROUTE] Traffic %s->%s:\n", g->nodes[dc].region, g->nodes[other_dc].region);
                printf("          New path via active nodes (%dms)\n", dist[other_dc]);
                printf("[WARNING] Latency increased due to rerouting\n");
            } else {
                printf("[ISOLATED] Region %s has no route to %s\n", g->nodes[dc].region, g->nodes[other_dc].region);
            }
        }
        free(dist);
    } else {
        printf("[ISOLATED] Region %s has no active datacenter for routing\n", g->nodes[failed_node].region);
    }
}

int count_reachable(Graph* g, int src) {
    if(src < 0 || src >= g->node_count) return 0;
    int* dist = dijkstra(g, src);
    int count = 0;
    for (int i = 0; i < g->node_count; i++) {
        if (dist[i] != INT_MAX) count++;
    }
    free(dist);
    return count;
}
