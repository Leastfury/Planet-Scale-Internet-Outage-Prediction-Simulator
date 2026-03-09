#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "node.h"
#include "edge.h"

typedef struct Graph {
    Node*  nodes;
    Edge*  edges;
    int    node_count;
    int    edge_count;
    int    node_capacity;   // malloc'd capacity
    int    edge_capacity;
    int**  adj;             // adjacency list (adj[i] = array of edge ids)
    int*   adj_count;       // how many edges per node
} Graph;

Graph* init_graph(int initial_node_capacity, int initial_edge_capacity);
int add_node(Graph* g, Node n);
int add_edge(Graph* g, Edge e);
int find_node_by_name(Graph* g, const char* name);
void free_graph(Graph* g);

#endif
