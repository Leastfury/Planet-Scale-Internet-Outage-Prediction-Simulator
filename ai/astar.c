#include "astar.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int g_silent_mode = 0;

void sift_up(BinaryHeap* h, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (h->nodes[index].f_cost < h->nodes[parent].f_cost) {
            AStarNode temp = h->nodes[index];
            h->nodes[index] = h->nodes[parent];
            h->nodes[parent] = temp;
            index = parent;
        } else {
            break;
        }
    }
}

void sift_down(BinaryHeap* h, int index) {
    int min_index = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < h->size && h->nodes[left].f_cost < h->nodes[min_index].f_cost) {
        min_index = left;
    }
    if (right < h->size && h->nodes[right].f_cost < h->nodes[min_index].f_cost) {
        min_index = right;
    }

    if (index != min_index) {
        AStarNode temp = h->nodes[index];
        h->nodes[index] = h->nodes[min_index];
        h->nodes[min_index] = temp;
        sift_down(h, min_index);
    }
}

void heap_insert(BinaryHeap* h, AStarNode node) {
    if (h->size >= h->capacity) {
        h->capacity = h->capacity == 0 ? 16 : h->capacity * 2;
        h->nodes = (AStarNode*)realloc(h->nodes, sizeof(AStarNode) * h->capacity);
    }
    h->nodes[h->size] = node;
    sift_up(h, h->size);
    h->size++;
}

AStarNode heap_extract_min(BinaryHeap* h) {
    AStarNode min_node = h->nodes[0];
    h->size--;
    h->nodes[0] = h->nodes[h->size];
    sift_down(h, 0);
    return min_node;
}

float haversine_heuristic(Node* n1, Node* n2) {
    float lat1 = n1->latitude * M_PI / 180.0f;
    float lon1 = n1->longitude * M_PI / 180.0f;
    float lat2 = n2->latitude * M_PI / 180.0f;
    float lon2 = n2->longitude * M_PI / 180.0f;

    float dlat = lat2 - lat1;
    float dlon = lon2 - lon1;

    float a = sinf(dlat / 2) * sinf(dlat / 2) +
              cosf(lat1) * cosf(lat2) *
              sinf(dlon / 2) * sinf(dlon / 2);
    float c = 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));
    
    // Earth radius approx 6371 km
    float distance_km = 6371.0f * c;
    
    // Light in fiber travels at ~200 km/ms
    float latency_ms = distance_km / 200.0f;
    return latency_ms;
}

int* reconstruct_path(int* parents, int target) {
    int capacity = 16;
    int size = 0;
    int* path = (int*)malloc(sizeof(int) * capacity);
    
    int curr = target;
    while (curr != -1) {
        if (size >= capacity - 1) {
            capacity *= 2;
            path = (int*)realloc(path, sizeof(int) * capacity);
        }
        path[size++] = curr;
        curr = parents[curr];
    }
    
    // Reverse the path
    for (int i = 0; i < size / 2; i++) {
        int temp = path[i];
        path[i] = path[size - 1 - i];
        path[size - 1 - i] = temp;
    }
    
    path[size] = -1; // Terminate array
    return path;
}

int* astar_search(Graph* g, int source, int target) {
    float* g_score = (float*)malloc(sizeof(float) * g->node_capacity);
    int* parents = (int*)malloc(sizeof(int) * g->node_capacity);
    int* visited = (int*)calloc(g->node_capacity, sizeof(int));
    
    for (int i = 0; i < g->node_capacity; i++) {
        g_score[i] = 1e9f; // Infinity equivalent
        parents[i] = -1;
    }
    
    BinaryHeap h = {0};
    
    g_score[source] = 0.0f;
    AStarNode start = {
        .node_id = source, 
        .g_cost = 0.0f, 
        .h_cost = (target != -1) ? haversine_heuristic(&g->nodes[source], &g->nodes[target]) : 0.0f,
        .parent = -1
    };
    start.f_cost = start.g_cost + start.h_cost;
    heap_insert(&h, start);
    
    int* path = NULL;
    
    while (h.size > 0) {
        AStarNode current = heap_extract_min(&h);
        
        if (visited[current.node_id]) continue;
        visited[current.node_id] = 1;
        
        if (current.node_id == target) {
            path = reconstruct_path(parents, target);
            break;
        }
        
        for (int i = 0; i < g->adj_count[current.node_id]; i++) {
            int edge_idx = g->adj[current.node_id][i];
            Edge e = g->edges[edge_idx];
            if (!e.is_active) continue;
            
            int neighbor = (e.from == current.node_id) ? e.to : e.from;
            if (!g->nodes[neighbor].is_active) continue;
            
            float tentative_g = g_score[current.node_id] + e.latency;
            
            if (tentative_g < g_score[neighbor]) {
                parents[neighbor] = current.node_id;
                g_score[neighbor] = tentative_g;
                
                float h_cost = (target != -1) ? haversine_heuristic(&g->nodes[neighbor], &g->nodes[target]) : 0.0f;
                AStarNode next_node = {
                    .node_id = neighbor,
                    .g_cost = tentative_g,
                    .h_cost = h_cost,
                    .f_cost = tentative_g + h_cost,
                    .parent = current.node_id
                };
                heap_insert(&h, next_node);
            }
        }
    }
    
    free(h.nodes);
    free(g_score);
    free(parents);
    free(visited);
    
    return path;
}

// Replacement for old dijkstra count_reachable
int count_reachable_astar(Graph* g, int src) {
    if(src < 0 || src >= g->node_count) return 0;
    
    float* g_score = (float*)malloc(sizeof(float) * g->node_capacity);
    int* visited = (int*)calloc(g->node_capacity, sizeof(int));
    for (int i = 0; i < g->node_capacity; i++) g_score[i] = 1e9f;
    
    BinaryHeap h = {0};
    g_score[src] = 0.0f;
    
    AStarNode start = {src, 0.0f, 0.0f, 0.0f, -1};
    heap_insert(&h, start);
    
    int count = 0;
    while(h.size > 0) {
        AStarNode current = heap_extract_min(&h);
        if (visited[current.node_id]) continue;
        visited[current.node_id] = 1;
        count++;
        
        for (int i = 0; i < g->adj_count[current.node_id]; i++) {
            int edge_idx = g->adj[current.node_id][i];
            Edge e = g->edges[edge_idx];
            if (!e.is_active) continue;
            
            int neighbor = (e.from == current.node_id) ? e.to : e.from;
            if (!g->nodes[neighbor].is_active) continue;
            
            float tentative_g = g_score[current.node_id] + e.latency;
            if (tentative_g < g_score[neighbor]) {
                g_score[neighbor] = tentative_g;
                AStarNode next_node = {neighbor, tentative_g, 0.0f, tentative_g, current.node_id};
                heap_insert(&h, next_node);
            }
        }
    }
    
    free(h.nodes);
    free(g_score);
    free(visited);
    return count;
}

void reroute_traffic_astar(Graph* g, int failed_node) {
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
        int other_dc = -1;
        for (int i = 0; i < g->node_count; i++) {
            if (g->nodes[i].region_id != r && g->nodes[i].type == DATACENTER && g->nodes[i].is_active) {
                other_dc = i;
                break; // Just takes the first active datacenter outside the region
            }
        }
        
        if (other_dc != -1) {
            int* path = astar_search(g, dc, other_dc);
            if (path != NULL) {
                // calculate latency along the path to simulate the old output
                int total_latency = 0;
                for (int i = 0; path[i+1] != -1; i++) {
                    int u = path[i];
                    int v = path[i+1];
                    for (int j = 0; j < g->adj_count[u]; j++) {
                        int edge_idx = g->adj[u][j];
                        Edge e = g->edges[edge_idx];
                        if ((e.from == u && e.to == v) || (e.from == v && e.to == u)) {
                            total_latency += e.latency;
                            break;
                        }
                    }
                }
                
                if (!g_silent_mode) printf("[REROUTE] Traffic %s->%s:\n", g->nodes[dc].region, g->nodes[other_dc].region);
                if (!g_silent_mode) printf("          New path via active nodes (%dms)\n", total_latency);
                if (!g_silent_mode) printf("[WARNING] Latency increased due to rerouting\n");
                free(path);
            } else {
                if (!g_silent_mode) printf("[ISOLATED] Region %s has no route to %s\n", g->nodes[dc].region, g->nodes[other_dc].region);
            }
        }
    } else {
        if (!g_silent_mode) printf("[ISOLATED] Region %s has no active datacenter for routing\n", g->nodes[failed_node].region);
    }
}
