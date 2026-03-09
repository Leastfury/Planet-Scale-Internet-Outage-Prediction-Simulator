#ifndef NODE_H
#define NODE_H

#define SUBMARINE_CABLE 0
#define DATACENTER      1
#define SATELLITE       2

#define REGION_NA  0
#define REGION_EU  1
#define REGION_AS  2
#define REGION_SA  3
#define REGION_AF  4
#define REGION_OC  5
#define REGION_COUNT 6

typedef struct Node {
    int   id;
    char  name[64];
    char  region[32];
    int   region_id;        // REGION_NA .. REGION_OC
    float latitude;
    float longitude;
    int   type;             // SUBMARINE_CABLE, DATACENTER, SATELLITE
    int   capacity;         // Mbps
    int   current_load;     // Mbps currently used
    int   is_active;        // 1=online 0=failed
    float risk_score;       // 0.0-100.0 set by AI
    int   backup_links;     // count of redundant edges
} Node;

int get_region_id(const char* region_name);

#endif
