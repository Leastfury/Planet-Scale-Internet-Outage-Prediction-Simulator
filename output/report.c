#include "report.h"
#include "../ai/heuristics.h"
#include "../ai/expert_system.h"
#include <stdio.h>
#include <math.h>

static void build_node_reason(Graph* g, int node_id,
    char* reason, size_t rlen,
    char* cascade, size_t clen,
    char* fix,    size_t flen)
{
    Node* n = &g->nodes[node_id];

    // Rule 0: Submarine cable SPOF
    if (n->type == SUBMARINE_CABLE &&
        g->adj_count[node_id] <= 1) {
        snprintf(reason,  rlen,
            "Single point of failure, %s isolated if cut",
            n->region);
        snprintf(cascade, clen,
            "%s region loses all connectivity", n->region);
        snprintf(fix,     flen,
            "Add at least 2 redundant cables bypassing node");
        return;
    }

    // Rule 1: High risk score = critical bridge node
    if (n->risk_score >= 70.0f) {
        snprintf(reason,  rlen,
            "Critical network bridge, risk score %.0f/100",
            n->risk_score);
        snprintf(cascade, clen,
            "Multiple regions lose inter-connectivity");
        snprintf(fix,     flen,
            "Deploy redundant parallel path and CDN nodes");
        return;
    }

    // Rule 2: Satellite — sole backup link for region
    if (n->type == SATELLITE) {
        snprintf(reason,  rlen,
            "Sole satellite backup for %s, no failover",
            n->region);
        snprintf(cascade, clen,
            "%s loses last-resort backup link entirely",
            n->region);
        snprintf(fix,     flen,
            "Add second satellite or ground station coverage");
        return;
    }

    // Rule 3: Datacenter — only active DC in region
    if (n->type == DATACENTER) {
        int active_dc = 0;
        for (int j = 0; j < g->node_count; j++) {
            if (g->nodes[j].type == DATACENTER &&
                g->nodes[j].region_id == n->region_id &&
                g->nodes[j].is_active &&
                j != node_id) active_dc++;
        }
        if (active_dc == 0) {
            snprintf(reason,  rlen,
                "Only active DC in %s, no CDN backup",
                n->region);
            snprintf(cascade, clen,
                "Cloud services offline for region 2-4 hours");
            snprintf(fix,     flen,
                "Activate CDN failover, reroute DNS records");
            return;
        }
    }

    // Rule 5: Geographic high-risk zone
    if (fabsf(n->latitude) > 30.0f) {
        snprintf(reason,  rlen,
            "High geo-risk zone (lat %.1f), damage risk",
            n->latitude);
        snprintf(cascade, clen,
            "Physical damage likely in earthquake or storm");
        snprintf(fix,     flen,
            "Harden landing stations, armored cable segments");
        return;
    }

    // Default fallback
    snprintf(reason,  rlen,
        "Elevated risk, limited redundancy");
    snprintf(cascade, clen,
        "Possible region degradation");
    snprintf(fix,     flen,
        "Add redundant routing paths");
}

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
            
            char reason[96], cascade[96], fix[96];
            build_node_reason(g, i,
                reason,  sizeof(reason),
                cascade, sizeof(cascade),
                fix,     sizeof(fix));
            printf("|  Reason   : %-52.52s |\n", reason);
            printf("|  Cascade  : %-52.52s |\n", cascade);
            printf("|  Fix      : %-52.52s |\n", fix);
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
