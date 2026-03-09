#ifndef CASCADE_H
#define CASCADE_H

#include "../graph/topology.h"
#include "../simulation/scheduler.h"

void schedule_cascades(Scheduler* s, Graph* g, int failed_node);

#endif
