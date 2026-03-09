#include "real_topology.h"
#include <string.h>

void load_hardcoded(Graph* g) {
    struct {
        int id; char name[64]; char region[64]; float lat; float lon; int cap; int type;
    } default_nodes[] = {
        {1, "SEA-ME-WE-4", "Asia-Pacific", 13.0, 80.0, 1280000, SUBMARINE_CABLE},
        {2, "SEA-ME-WE-5", "Asia-Pacific", 11.0, 77.0, 24000000, SUBMARINE_CABLE},
        {3, "FLAG-Cable", "Europe", 51.5, 0.1, 600000, SUBMARINE_CABLE},
        {4, "APCN-2", "Asia-Pacific", 35.6, 139.6, 800000, SUBMARINE_CABLE},
        {5, "TAT-14", "North America", 40.7, -74.0, 640000, SUBMARINE_CABLE},
        {6, "ACS-Atlantic", "North America", 38.0, -75.0, 1280000, SUBMARINE_CABLE},
        {7, "SEACOM", "Africa", -26.0, 28.0, 1280000, SUBMARINE_CABLE},
        {8, "EASSy", "Africa", -4.0, 39.6, 1400000, SUBMARINE_CABLE},
        {9, "SAm-1", "South America", -23.5, -46.6, 340000, SUBMARINE_CABLE},
        {10, "Southern-Cross", "Oceania", -33.8, 151.2, 620000, SUBMARINE_CABLE},
        {11, "Asia-America-GW", "Asia-Pacific", 1.3, 103.8, 2000000, SUBMARINE_CABLE},
        {12, "Hibernia-Atlantic", "Europe", 53.3, -6.2, 8800000, SUBMARINE_CABLE},
        {13, "AWS-us-east", "North America", 39.0, -77.0, 999999, DATACENTER},
        {14, "AWS-eu-frankfurt", "Europe", 50.1, 8.6, 999999, DATACENTER},
        {15, "Google-asia-tokyo", "Asia-Pacific", 35.6, 139.6, 999999, DATACENTER},
        {16, "Azure-brazil", "South America", -23.5, -46.6, 999999, DATACENTER},
        {17, "AWS-af-capetown", "Africa", -33.9, 18.4, 999999, DATACENTER},
        {18, "AWS-ap-sydney", "Oceania", -33.8, 151.2, 999999, DATACENTER},
        {19, "Starlink-NA", "North America", 0.0, 0.0, 500000, SATELLITE},
        {20, "Starlink-EU", "Europe", 0.0, 0.0, 500000, SATELLITE},
        {21, "Intelsat-Asia", "Asia-Pacific", 0.0, 0.0, 200000, SATELLITE},
        {22, "SES-Africa", "Africa", 0.0, 0.0, 150000, SATELLITE},
        {23, "Starlink-SA", "South America", 0.0, 0.0, 300000, SATELLITE},
        {24, "Optus-Oceania", "Oceania", 0.0, 0.0, 180000, SATELLITE}
    };

    struct {
        char from[64]; char to[64]; int bw; int lat; float fp;
    } default_edges[] = {
        {"SEA-ME-WE-4", "AWS-eu-frankfurt", 640000, 120, 0.05f},
        {"SEA-ME-WE-4", "Google-asia-tokyo", 640000, 60, 0.05f},
        {"SEA-ME-WE-5", "AWS-eu-frankfurt", 5000000, 110, 0.03f},
        {"FLAG-Cable", "AWS-us-east", 300000, 80, 0.04f},
        {"FLAG-Cable", "AWS-eu-frankfurt", 300000, 20, 0.03f},
        {"APCN-2", "Google-asia-tokyo", 400000, 40, 0.04f},
        {"APCN-2", "AWS-ap-sydney", 400000, 90, 0.04f},
        {"TAT-14", "AWS-us-east", 320000, 70, 0.03f},
        {"TAT-14", "AWS-eu-frankfurt", 320000, 75, 0.03f},
        {"ACS-Atlantic", "AWS-us-east", 640000, 65, 0.03f},
        {"ACS-Atlantic", "FLAG-Cable", 640000, 85, 0.03f},
        {"SEACOM", "AWS-af-capetown", 640000, 50, 0.06f},
        {"SEACOM", "AWS-eu-frankfurt", 640000, 100, 0.05f},
        {"EASSy", "AWS-af-capetown", 700000, 55, 0.06f},
        {"EASSy", "SEA-ME-WE-4", 500000, 110, 0.05f},
        {"SAm-1", "Azure-brazil", 170000, 30, 0.07f},
        {"SAm-1", "AWS-us-east", 170000, 100, 0.06f},
        {"Southern-Cross", "AWS-ap-sydney", 310000, 20, 0.04f},
        {"Southern-Cross", "AWS-us-east", 310000, 150, 0.05f},
        {"Asia-America-GW", "Google-asia-tokyo", 1000000, 30, 0.03f},
        {"Asia-America-GW", "AWS-us-east", 1000000, 140, 0.04f},
        {"Hibernia-Atlantic", "AWS-us-east", 4400000, 60, 0.02f},
        {"Hibernia-Atlantic", "AWS-eu-frankfurt", 4400000, 15, 0.02f},
        {"Starlink-NA", "AWS-us-east", 250000, 600, 0.08f},
        {"Starlink-EU", "AWS-eu-frankfurt", 250000, 600, 0.08f},
        {"Intelsat-Asia", "Google-asia-tokyo", 100000, 600, 0.10f},
        {"SES-Africa", "AWS-af-capetown", 75000, 600, 0.12f},
        {"Starlink-SA", "Azure-brazil", 150000, 600, 0.09f},
        {"Optus-Oceania", "AWS-ap-sydney", 90000, 600, 0.10f}
    };

    for(int i=0; i < 24; i++) {
        Node n;
        n.id = g->node_count;
        strcpy(n.name, default_nodes[i].name);
        strcpy(n.region, default_nodes[i].region);
        n.region_id = get_region_id(n.region);
        n.latitude = default_nodes[i].lat;
        n.longitude = default_nodes[i].lon;
        n.capacity = default_nodes[i].cap;
        n.type = default_nodes[i].type;
        n.current_load = n.capacity / 2;
        n.is_active = 1;
        n.risk_score = 0;
        n.backup_links = 0;
        add_node(g, n);
    }

    for(int i=0; i < 29; i++) {
        int f_id = find_node_by_name(g, default_edges[i].from);
        int t_id = find_node_by_name(g, default_edges[i].to);
        if (f_id != -1 && t_id != -1) {
            Edge e;
            e.id = g->edge_count;
            e.from = f_id;
            e.to = t_id;
            e.bandwidth = default_edges[i].bw;
            e.latency = default_edges[i].lat;
            e.failure_prob = default_edges[i].fp;
            e.is_active = 1;
            e.current_load = e.bandwidth / 2;
            add_edge(g, e);
        }
    }
}
