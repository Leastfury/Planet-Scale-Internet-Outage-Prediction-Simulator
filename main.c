#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph/topology.h"
#include "graph/node.h"
#include "graph/edge.h"
#include "failures/failure_engine.h"
#include "failures/cascade.h"
#include "ai/astar.h"
#include "ai/expert_system.h"
#include "ai/heuristics.h"
#include "ai/hmm.h"
#include "ai/bayesian.h"
#include "ai/qlearning.h"
#include "ai/genetic.h"
#include "ai/minimax.h"
#include "simulation/scheduler.h"
#include "simulation/metrics.h"
#include "output/visualizer.h"
#include "output/report.h"
#include "data/loader.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdbool.h>


void show_menu() {
    printf("\n");
    printf("+------------------------------------------------------+\n");
    printf("|       PLANET-SCALE OUTAGE SIMULATOR              |\n");
    printf("+------------------------------------------------------+\n");
    printf("|   1.  Simulate Submarine Cable Cut                   |\n");
    printf("|   2.  Simulate Data Center Failure                   |\n");
    printf("|   3.  Simulate Satellite Outage                      |\n");
    printf("|   4.  Simulate Solar Storm  [Multi-Failure Scenario] |\n");
    printf("|   5.  Run AI Vulnerability Scan                      |\n");
    printf("|   6.  Generate AI Prediction Report                  |\n");
    printf("|   7.  Generate Resilience Recommendations            |\n");
    printf("|   8.  View Current Network Topology                  |\n");
    printf("|   9.  View Region Metrics Table                      |\n");
    printf("|   10. Load New Dataset File                          |\n");
    printf("|   11. Reset Simulation                               |\n");
    printf("|   12. Run Genetic Algorithm (Optimize Hardening)     |\n");
    printf("|   13. Run Adversarial Stress Test (Minimax)          |\n");
    printf("|   0.  Exit                                           |\n");
    printf("+------------------------------------------------------+\n");
    printf("   Select option: ");
}

void print_nodes_by_type(Graph* g, int type) {
    for (int i = 0; i < g->node_count; i++) {
        if (g->nodes[i].type == type) {
            printf("ID: %3d | %s (%s)\n", i, g->nodes[i].name, g->nodes[i].region);
        }
    }
}

void sim_failure(Graph* g, Scheduler* s, int type, const char* name) {
    printf("--- %s ---\n", name);
    print_nodes_by_type(g, type);
    printf("Enter node ID: ");
    int id;
    if (scanf("%d", &id) == 1) {
        char desc[128];
        snprintf(desc, sizeof(desc), "Manual failure injected for node %d", id);
        add_event(s, s->current_time + 1, EVT_FAILURE, id, -1, desc);
        run_simulation(s, g);
    }
}

int main(int argc, char** argv) {
    Graph* g = init_graph(50, 50);
    Scheduler* s = init_scheduler();
    
    // Check for CLI flags
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--train-rl") == 0) {
            train_episodes(g, 100);
            free_graph(g);
            free_scheduler(s);
            return 0;
        }
    }
    
    printf("═══════════════════════════════════════\n");
    printf("  Loading topology data...\n");
    int src = load_topology(g);
    if (src == FROM_CSV)       printf("  Loaded from topology.csv\n");
    else if (src == FROM_JSON) printf("  Loaded from topology.json\n");
    else                       printf("  Using built-in topology data\n");
    printf("  Nodes: %d  Edges: %d\n", g->node_count, g->edge_count);
    printf("═══════════════════════════════════════\n");
    
    update_metrics(g);
    
    int choice = -1;
    while (1) {
        show_menu();
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); 
            continue;
        }
        
        if (choice == 0) {
            free_graph(g);
            free_scheduler(s);
            break;
        }
        
        switch (choice) {
            case 1:
                sim_failure(g, s, SUBMARINE_CABLE, "Submarine Cables");
                break;
            case 2:
                sim_failure(g, s, DATACENTER, "Data Centers");
                break;
            case 3:
                sim_failure(g, s, SATELLITE, "Satellites");
                break;
            case 4: {
                add_event(s, 0, EVT_FAILURE, find_node_by_name(g, "SEA-ME-WE-4"), -1, "Earthquake severs SEA-ME-WE-4 cable");
                add_event(s, 3, EVT_REROUTE, find_node_by_name(g, "APCN-2"), -1, "Asia-Pacific traffic rerouting via APCN-2");
                
                int apcn2_edge = -1;
                for (int i=0; i<g->edge_count; i++) {
                    if (g->edges[i].from == find_node_by_name(g, "APCN-2") || g->edges[i].to == find_node_by_name(g, "APCN-2")) {
                        apcn2_edge = i; break;
                    }
                }
                add_event(s, 8, EVT_OVERLOAD, -1, apcn2_edge, "APCN-2 reaches 95%% capacity");
                add_event(s, 15, EVT_CASCADE, find_node_by_name(g, "APCN-2"), -1, "APCN-2 fails under load");
                
                int sat_edge = -1;
                for(int i=0; i<g->edge_count; i++) {
                    if (g->edges[i].from == find_node_by_name(g, "Intelsat-Asia") || g->edges[i].to == find_node_by_name(g, "Intelsat-Asia")) {
                        sat_edge = i; break;
                    }
                }
                add_event(s, 20, EVT_OVERLOAD, -1, sat_edge, "Satellite link saturated");
                add_event(s, 25, EVT_FAILURE, find_node_by_name(g, "Google-asia-tokyo"), -1, "Tokyo DC loses primary uplink");
                add_event(s, 35, EVT_CASCADE, find_node_by_name(g, "SEA-ME-WE-5"), -1, "SEA-ME-WE-5 takes overload, degrades");
                add_event(s, 45, EVT_FAILURE, find_node_by_name(g, "FLAG-Cable"), -1, "Solar storm disrupts FLAG cable amplifiers");
                add_event(s, 60, EVT_REROUTE, find_node_by_name(g, "Hibernia-Atlantic"), -1, "Europe rerouting via Hibernia");
                add_event(s, 75, EVT_RECOVERY, find_node_by_name(g, "SEA-ME-WE-4"), -1, "Repair ship reaches cable break");
                add_event(s, 120, EVT_RECOVERY, find_node_by_name(g, "APCN-2"), -1, "APCN-2 restored at 60%% capacity");
                add_event(s, 180, EVT_RECOVERY, find_node_by_name(g, "Google-asia-tokyo"), -1, "Tokyo DC back online");
                
                run_simulation(s, g);
                break;
            }
            case 5:
                score_all_nodes(g);
                run_expert_system(g);
                printf("\n--- AI Vulnerability Scan Complete ---\n");
                for (int i = 0; i < g->node_count; i++) {
                    printf("Node %-20s Risk Score: %.0f\n", g->nodes[i].name, g->nodes[i].risk_score);
                }
                break;
            case 6:
                generate_prediction_report(g);
                break;
            case 7:
                generate_resilience_report(g);
                break;
            case 8:
                printf("\n--- Nodes ---\n");
                for (int i = 0; i < g->node_count; i++) {
                    Node n = g->nodes[i];
                    printf("[%d] %s (%s) Cap:%d Status:%s Risk:%.0f\n", n.id, n.name, n.region, n.capacity, n.is_active?"ACTIVE":"FAILED", n.risk_score);
                }
                printf("\n--- Edges ---\n");
                for (int i = 0; i < g->edge_count; i++) {
                    Edge e = g->edges[i];
                    printf("[%d] %s <-> %s BW:%d Latency:%d Status:%s\n", 
                        e.id, g->nodes[e.from].name, g->nodes[e.to].name, e.bandwidth, e.latency, e.is_active?"ACTIVE":"FAILED");
                }
                break;
            case 9:
                update_metrics(g);
                print_metrics_table(g);
                break;
            case 10: {
                char filename[256];
                printf("Enter filename: ");
                scanf("%255s", filename);
                free_graph(g);
                g = init_graph(50, 50);
                if (detect_format(filename) == FROM_CSV) load_from_csv(g, filename);
                else load_from_json(g, filename);
                update_metrics(g);
                print_ascii_map(g);
                break;
            }
            case 11:
                free_graph(g);
                free_scheduler(s);
                g = init_graph(50, 50);
                s = init_scheduler();
                load_topology(g);
                update_metrics(g);
                print_ascii_map(g);
                break;
            case 12: {
                int budget = 5000;
                int gens = 50;
                printf("Enter budget for upgrades (Mbps): ");
                scanf("%d", &budget);
                printf("Enter number of generations: ");
                scanf("%d", &gens);
                optimize_hardening(g, gens, budget);
                break;
            }
            case 13:
                run_adversarial_simulation(g);
                break;
        }
    }
    
    return 0;
}
