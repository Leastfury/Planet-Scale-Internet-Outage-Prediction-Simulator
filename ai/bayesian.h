#ifndef BAYESIAN_H
#define BAYESIAN_H

#include "../graph/topology.h"

#define MAX_BAYES_NODES 1000

typedef struct BayesNode {
    int network_node_id;
    float prior;
    float cpt[16]; // Max 4 parents -> 2^4 = 16 entries
    int parents[4];
    int parent_count;
} BayesNode;

typedef struct BayesNetwork {
    BayesNode nodes[MAX_BAYES_NODES];
    int node_count;
} BayesNetwork;

BayesNetwork build_bayesian_network(Graph* g);
void build_cpt_from_topology(Graph* g, int node_id, float* cpt);
float bayesian_query(BayesNetwork* bn, int target, int* evidence, int ev_count);
int* get_top_risk_nodes(BayesNetwork* bn, int* failed_evidence, int ev_count);

#endif
