#ifndef METRICS_H
#define METRICS_H

#include "../graph/topology.h"

typedef struct RegionStatus {
    char  name[32];
    int   connectivity_pct; // 0-100
    int   active_nodes;
    int   total_nodes;
    int   avg_latency;      // ms
    int   packet_loss_pct;  // 0-100
    int   time_to_restore;  // minutes since first failure in this region
} RegionStatus;

extern RegionStatus g_regions[6];

void update_metrics(Graph* g);
void print_metrics_table(Graph* g);
void init_metrics();

#endif
