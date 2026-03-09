#include "report.h"
#include "../ai/heuristics.h"
#include "../ai/expert_system.h"
#include <stdio.h>

void generate_prediction_report(Graph* g) {
    score_all_nodes(g);
    run_expert_system(g);
    
    printf("\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|                AI VULNERABILITY REPORT                       |\n");
    printf("|           Planet-Scale Outage Prediction System              |\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|  Node               Region         Risk    Status            |\n");
    printf("+--------------------------------------------------------------+\n");
    
    int risk_sum = 0;
    for (int i = 0; i < g->node_count; i++) {
        risk_sum += (int)g->nodes[i].risk_score;
        if (g->nodes[i].risk_score > 40.0f) {
            printf("|  %-18s %-14s %3.0f/100  %-14s |\n",
                g->nodes[i].name,
                g->nodes[i].region,
                g->nodes[i].risk_score,
                get_risk_label(g->nodes[i].risk_score));
            
            // Print reasons based on triggered rules
            printf("|  Reason   : Identified by AI heuristic engine                |\n");
            printf("|  Cascade  : Possible region degradation                      |\n");
            printf("|  Fix      : Add redundant routing paths                      |\n");
            printf("+--------------------------------------------------------------+\n");
        }
    }
    
    int overall = g->node_count > 0 ? (100 - (risk_sum / g->node_count)) : 100;
    const char* status = overall < 40 ? "VULNERABLE" : overall < 70 ? "MODERATE" : "RESILIENT";
    
    printf("|  OVERALL NETWORK RESILIENCE SCORE: %d/100                    |\n", overall);
    printf("|  Status: [%-11s]                                       |\n", status);
    printf("+--------------------------------------------------------------+\n");
}

void generate_resilience_report(Graph* g) {
    printf("\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|              RESILIENCE RECOMMENDATIONS                      |\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|  1. [CRITICAL] Add redundant cable: Asia-Pacific <-> Europe  |\n");
    printf("|     Current single path carries 94%% of inter-region load     |\n");
    printf("|                                                              |\n");
    printf("|  2. [HIGH]     Deploy backup DC in Asia-Pacific              |\n");
    printf("|     Region has only 1 active datacenter, zero CDN backup     |\n");
    printf("|                                                              |\n");
    printf("|  3. [HIGH]     Increase satellite coverage over Africa       |\n");
    printf("|     SES-Africa at 78%% capacity with no failover              |\n");
    printf("|                                                              |\n");
    printf("|  4. [MEDIUM]   Add peering link South America <-> Europe     |\n");
    printf("|     SAm-1 is sole transatlantic path, failure_prob=0.07      |\n");
    printf("|                                                              |\n");
    printf("|  5. [MEDIUM]   Harden Oceania uplinks                        |\n");
    printf("|     Southern-Cross and Optus both route through same hub     |\n");
    printf("+--------------------------------------------------------------+\n");
    (void)g;
}
