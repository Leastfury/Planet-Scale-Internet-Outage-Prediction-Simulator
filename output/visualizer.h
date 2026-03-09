#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "../graph/topology.h"

void print_ascii_map(Graph* g);
const char* get_status_label(int pct);
const char* get_bar(int pct);

#endif
