#include "node.h"
#include <string.h>

int get_region_id(const char* region_name) {
    if (strcmp(region_name, "North America") == 0) return REGION_NA;
    if (strcmp(region_name, "Europe") == 0) return REGION_EU;
    if (strcmp(region_name, "Asia-Pacific") == 0) return REGION_AS;
    if (strcmp(region_name, "South America") == 0) return REGION_SA;
    if (strcmp(region_name, "Africa") == 0) return REGION_AF;
    if (strcmp(region_name, "Oceania") == 0) return REGION_OC;
    return REGION_NA; // default
}
