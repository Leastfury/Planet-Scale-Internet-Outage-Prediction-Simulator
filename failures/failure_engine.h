#ifndef FAILURE_ENGINE_H
#define FAILURE_ENGINE_H

#include "../graph/topology.h"

void apply_node_failure(Graph* g, int node_id);
void restore_node(Graph* g, int node_id);

#endif
