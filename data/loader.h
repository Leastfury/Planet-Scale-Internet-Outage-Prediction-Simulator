#ifndef LOADER_H
#define LOADER_H

#include "../graph/topology.h"

#define FROM_CSV       1
#define FROM_JSON      2
#define FROM_HARDCODED 3

int load_topology(Graph* g);
int load_from_csv(Graph* g, const char* filename);
int load_from_json(Graph* g, const char* filename);
int detect_format(const char* filename);

#endif
