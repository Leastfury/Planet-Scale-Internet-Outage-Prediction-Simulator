#ifndef ASTAR_H
#define ASTAR_H

#include "../graph/topology.h"

typedef struct AStarNode {
    int node_id;
    float g_cost;
    float h_cost;
    float f_cost;
    int parent;
} AStarNode;

typedef struct BinaryHeap {
    AStarNode* nodes;
    int size;
    int capacity;
} BinaryHeap;

extern int g_silent_mode;

float haversine_heuristic(Node* n1, Node* n2);
void heap_insert(BinaryHeap* h, AStarNode node);
AStarNode heap_extract_min(BinaryHeap* h);
void sift_up(BinaryHeap* h, int index);
void sift_down(BinaryHeap* h, int index);

// Returns an array of node IDs representing the path, terminated by -1.
// If no path is found, returns NULL.
int* astar_search(Graph* g, int source, int target);

// Reconstructs the path from the parent array
int* reconstruct_path(int* parents, int target);

// Utility functions that replace dijkstra usage
void reroute_traffic_astar(Graph* g, int failed_node);
int count_reachable_astar(Graph* g, int src);

#endif
