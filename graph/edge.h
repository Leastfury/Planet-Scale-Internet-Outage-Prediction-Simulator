#ifndef EDGE_H
#define EDGE_H

typedef struct Edge {
    int   id;
    int   from;
    int   to;
    int   bandwidth;        // Mbps
    int   latency;          // ms
    float failure_prob;     // 0.0-1.0
    int   is_active;
    int   current_load;     // Mbps currently flowing
    float pheromone;        // For ACO
} Edge;

#endif
