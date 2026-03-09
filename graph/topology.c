#include "topology.h"
#include <stdlib.h>
#include <string.h>

Graph* init_graph(int initial_node_cap, int initial_edge_cap) {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    g->node_capacity = initial_node_cap > 0 ? initial_node_cap : 10;
    g->edge_capacity = initial_edge_cap > 0 ? initial_edge_cap : 10;
    g->node_count = 0;
    g->edge_count = 0;
    
    g->nodes = (Node*)malloc(sizeof(Node) * g->node_capacity);
    g->edges = (Edge*)malloc(sizeof(Edge) * g->edge_capacity);
    
    g->adj = (int**)malloc(sizeof(int*) * g->node_capacity);
    for (int i = 0; i < g->node_capacity; i++) {
        g->adj[i] = NULL;
    }
    g->adj_count = (int*)calloc(g->node_capacity, sizeof(int));
    
    return g;
}

int add_node(Graph* g, Node n) {
    if (g->node_count >= g->node_capacity) {
        g->node_capacity *= 2;
        g->nodes = (Node*)realloc(g->nodes, sizeof(Node) * g->node_capacity);
        g->adj = (int**)realloc(g->adj, sizeof(int*) * g->node_capacity);
        for(int i = g->node_count; i < g->node_capacity; i++) {
            g->adj[i] = NULL;
        }
        g->adj_count = (int*)realloc(g->adj_count, sizeof(int) * g->node_capacity);
        for(int i = g->node_count; i < g->node_capacity; i++) {
            g->adj_count[i] = 0;
        }
    }
    g->nodes[g->node_count] = n;
    int cur_idx = g->node_count;
    g->node_count++;
    return cur_idx;
}

int add_edge(Graph* g, Edge e) {
    if (g->edge_count >= g->edge_capacity) {
        g->edge_capacity *= 2;
        g->edges = (Edge*)realloc(g->edges, sizeof(Edge) * g->edge_capacity);
    }
    g->edges[g->edge_count] = e;
    int edge_idx = g->edge_count;
    g->edge_count++;
    
    int from = e.from;
    int to = e.to;
    
    // Add to 'from' adj
    g->adj_count[from]++;
    g->adj[from] = (int*)realloc(g->adj[from], sizeof(int) * g->adj_count[from]);
    g->adj[from][g->adj_count[from]-1] = edge_idx;
    
    // Increment backup links for submarine cables
    if (g->nodes[from].type == SUBMARINE_CABLE) g->nodes[from].backup_links++;
    if (g->nodes[to].type == SUBMARINE_CABLE) g->nodes[to].backup_links++;
    
    return edge_idx;
}

int find_node_by_name(Graph* g, const char* name) {
    for (int i = 0; i < g->node_count; i++) {
        if (strcmp(g->nodes[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void free_graph(Graph* g) {
    if (!g) return;
    free(g->nodes);
    free(g->edges);
    for (int i = 0; i < g->node_capacity; i++) {
        if (g->adj[i]) free(g->adj[i]);
    }
    free(g->adj);
    free(g->adj_count);
    free(g);
}
