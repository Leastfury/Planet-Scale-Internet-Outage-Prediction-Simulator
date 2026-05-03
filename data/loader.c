#include "loader.h"
#include "real_topology.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int detect_format(const char* filename) {
    if (strstr(filename, ".csv")) return FROM_CSV;
    if (strstr(filename, ".json")) return FROM_JSON;
    return FROM_HARDCODED;
}

int load_from_csv(Graph* g, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;
    
    char line[256];
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return 0;
    }
    
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#') continue;
        
        char* token = strtok(line, ",\r\n");
        if (!token) continue;
        
        if (strcmp(token, "EDGE") == 0) {
            char* from = strtok(NULL, ",\r\n");
            char* to = strtok(NULL, ",\r\n");
            char* bw = strtok(NULL, ",\r\n");
            char* lat = strtok(NULL, ",\r\n");
            char* fp = strtok(NULL, ",\r\n");
            
            if (from && to && bw && lat && fp) {
                int f_id = find_node_by_name(g, from);
                int t_id = find_node_by_name(g, to);
                if (f_id != -1 && t_id != -1) {
                    Edge e;
                    e.id = g->edge_count;
                    e.from = f_id;
                    e.to = t_id;
                    e.bandwidth = atoi(bw);
                    e.latency = atoi(lat);
                    e.failure_prob = atof(fp);
                    e.is_active = 1;
                    e.current_load = e.bandwidth / 2;
                    add_edge(g, e);
                }
            }
        } else {
            char* name = strtok(NULL, ",\r\n");
            char* region = strtok(NULL, ",\r\n");
            char* lat = strtok(NULL, ",\r\n");
            char* lon = strtok(NULL, ",\r\n");
            char* cap = strtok(NULL, ",\r\n");
            
            if (name && region && lat && lon && cap) {
                Node n;
                n.id = g->node_count;
                strncpy(n.name, name, 63); n.name[63]='\0';
                strncpy(n.region, region, 31); n.region[31]='\0';
                n.region_id = get_region_id(region);
                n.latitude = atof(lat);
                n.longitude = atof(lon);
                n.capacity = atoi(cap);
                n.current_load = n.capacity / 2;
                n.is_active = 1;
                n.risk_score = 0;
                n.backup_links = 0;
                
                if (strcmp(token, "SUBMARINE") == 0) n.type = SUBMARINE_CABLE;
                else if (strcmp(token, "DATACENTER") == 0) n.type = DATACENTER;
                else n.type = SATELLITE;
                
                add_node(g, n);
            }
        }
    }
    
    fclose(f);
    return 1;
}

int load_from_json(Graph* g, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);
    string[fsize] = 0;
    
    char* nodes_start = strstr(string, "\"nodes\"");
    char* edges_start = strstr(string, "\"edges\"");
    
    if (nodes_start) {
        char* p = nodes_start;
        while ((p = strstr(p, "{\"name\":\""))) {
            if (edges_start && p > edges_start) break;
            
            p += 9;
            char name[64]={0}, type[32]={0}, region[32]={0};
            float lat=0, lon=0;
            int cap=0;
            
            sscanf(p, "%[^\"]", name);
            
            char* t = strstr(p, "\"type\":\"");
            if(t) sscanf(t+8, "%[^\"]", type);
            
            char* r = strstr(p, "\"region\":\"");
            if(r) sscanf(r+10, "%[^\"]", region);
            
            char* l = strstr(p, "\"lat\":");
            if(l) sscanf(l+6, "%f", &lat);
            
            char* ln = strstr(p, "\"lon\":");
            if(ln) sscanf(ln+6, "%f", &lon);
            
            char* c = strstr(p, "\"capacity\":");
            if(c) sscanf(c+11, "%d", &cap);
            
            Node n;
            n.id = g->node_count;
            strncpy(n.name, name, 63); n.name[63] = '\0';
            strncpy(n.region, region, 31); n.region[31] = '\0';
            n.region_id = get_region_id(region);
            n.latitude = lat;
            n.longitude = lon;
            n.capacity = cap;
            n.current_load = cap / 2;
            n.is_active = 1;
            n.risk_score = 0;
            n.backup_links = 0;
            
            if (strcmp(type, "SUBMARINE") == 0) n.type = SUBMARINE_CABLE;
            else if (strcmp(type, "DATACENTER") == 0) n.type = DATACENTER;
            else n.type = SATELLITE;
            
            add_node(g, n);
            p += 10;
        }
    }
    
    if (edges_start) {
        char* p = edges_start;
        while ((p = strstr(p, "{\"from\":\""))) {
            p += 9;
            char from[64]={0}, to[64]={0};
            int bw=0, lat=0;
            float fp=0;
            
            sscanf(p, "%[^\"]", from);
            
            char* t = strstr(p, "\"to\":\"");
            if(t) sscanf(t+6, "%[^\"]", to);
            
            char* b = strstr(p, "\"bandwidth\":");
            if(b) sscanf(b+12, "%d", &bw);
            
            char* l = strstr(p, "\"latency\":");
            if(l) sscanf(l+10, "%d", &lat);
            
            char* f = strstr(p, "\"failure_prob\":");
            if(f) sscanf(f+15, "%f", &fp);
            
            int f_id = find_node_by_name(g, from);
            int t_id = find_node_by_name(g, to);
            if (f_id != -1 && t_id != -1) {
                Edge e;
                e.id = g->edge_count;
                e.from = f_id;
                e.to = t_id;
                e.bandwidth = bw;
                e.latency = lat;
                e.failure_prob = fp;
                e.is_active = 1;
                e.current_load = bw / 2;
                add_edge(g, e);
            }
            p += 10;
        }
    }
    
    free(string);
    return 1;
}

int load_topology(Graph* g) {
    if (load_from_csv(g, "topology.csv")) return FROM_CSV;
    if (load_from_json(g, "topology.json")) return FROM_JSON;
    
    load_hardcoded(g);
    return FROM_HARDCODED;
}
