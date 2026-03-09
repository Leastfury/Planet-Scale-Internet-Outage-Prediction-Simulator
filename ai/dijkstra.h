#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "../graph/topology.h"

int* dijkstra(Graph* g, int src);
void reroute_traffic(Graph* g, int failed_node);
int count_reachable(Graph* g, int src);

#endif
