#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "../graph/topology.h"

float compute_betweenness(Graph* g, int node_id);
void score_all_nodes(Graph* g);
const char* get_risk_label(float score);

#endif
