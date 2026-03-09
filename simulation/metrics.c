#include "metrics.h"
#include <stdio.h>
#include <string.h>

RegionStatus g_regions[6];

const char* region_names[] = {
    "North America",
    "Europe",
    "Asia-Pacific",
    "South America",
    "Africa",
    "Oceania"
};

void init_metrics() {
    for (int i = 0; i < 6; i++) {
        strncpy(g_regions[i].name, region_names[i], 31);
        g_regions[i].name[31] = '\0';
        g_regions[i].connectivity_pct = 100;
        g_regions[i].active_nodes = 0;
        g_regions[i].total_nodes = 0;
        g_regions[i].avg_latency = 0;
        g_regions[i].packet_loss_pct = 0;
        g_regions[i].time_to_restore = 0;
    }
}

void update_metrics(Graph* g) {
    if (g_regions[0].total_nodes == 0) { // first time init
        init_metrics();
    }
    
    int active_counts[6] = {0};
    int total_counts[6] = {0};
    int total_latency[6] = {0};
    int active_edges[6] = {0};
    int overloaded_edges[6] = {0};
    int total_edges[6] = {0};
    
    for (int i = 0; i < g->node_count; i++) {
        int r = g->nodes[i].region_id;
        if (r >= 0 && r < 6) {
            total_counts[r]++;
            if (g->nodes[i].is_active) {
                active_counts[r]++;
            }
        }
    }
    
    for (int i = 0; i < g->edge_count; i++) {
        int from_r = g->nodes[g->edges[i].from].region_id;
        int to_r = g->nodes[g->edges[i].to].region_id;
        
        if (from_r >= 0 && from_r < 6) {
            total_edges[from_r]++;
            if (g->edges[i].is_active) {
                active_edges[from_r]++;
                total_latency[from_r] += g->edges[i].latency;
            }
            if (g->edges[i].current_load > g->edges[i].bandwidth * 0.8) {
                overloaded_edges[from_r]++;
            }
        }
        
        if (to_r >= 0 && to_r < 6 && from_r != to_r) {
            total_edges[to_r]++;
            if (g->edges[i].is_active) {
                active_edges[to_r]++;
                total_latency[to_r] += g->edges[i].latency;
            }
            if (g->edges[i].current_load > g->edges[i].bandwidth * 0.8) {
                overloaded_edges[to_r]++;
            }
        }
    }
    
    for (int r = 0; r < 6; r++) {
        g_regions[r].total_nodes = total_counts[r];
        g_regions[r].active_nodes = active_counts[r];
        if (total_counts[r] > 0) {
            g_regions[r].connectivity_pct = (active_counts[r] * 100) / total_counts[r];
        } else {
            g_regions[r].connectivity_pct = 0;
        }
        
        if (active_edges[r] > 0) {
            g_regions[r].avg_latency = total_latency[r] / active_edges[r];
        } else {
            g_regions[r].avg_latency = 0;
        }
        
        if (total_edges[r] > 0) {
            g_regions[r].packet_loss_pct = (overloaded_edges[r] * 100) / total_edges[r];
        } else {
            g_regions[r].packet_loss_pct = 0;
        }
        
        if (g_regions[r].connectivity_pct < 100) {
            g_regions[r].time_to_restore += 1;
        } else {
            g_regions[r].time_to_restore = 0;
        }
    }
}

void print_metrics_table(Graph* g) {
    (void)g;
    printf("\n\n");
    printf("+---------------+----------+---------+----------+---------+\n");
    printf("| Region        | Online %% | Latency | Pkt Loss | TTR     |\n");
    printf("+---------------+----------+---------+----------+---------+\n");
    
    for (int r = 0; r < 6; r++) {
        char latency_str[16];
        if (g_regions[r].avg_latency == 0 && g_regions[r].connectivity_pct == 0) {
            strcpy(latency_str, "---");
        } else {
            sprintf(latency_str, "%dms", g_regions[r].avg_latency);
        }
        
        char ttr_str[16];
        if (g_regions[r].time_to_restore == 0) {
            strcpy(ttr_str, "-");
        } else {
            sprintf(ttr_str, "%dmin", g_regions[r].time_to_restore);
        }
        
        printf("| %-13s | %3d%%     | %-7s | %3d%%     | %-7s |\n",
               g_regions[r].name,
               g_regions[r].connectivity_pct,
               latency_str,
               g_regions[r].packet_loss_pct,
               ttr_str);
    }
    printf("+---------------+----------+---------+----------+---------+\n\n");
}
