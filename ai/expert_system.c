#include "expert_system.h"
#include <stdio.h>
#include <math.h>

// Classical AI technique: Expert System (Rule-Based Forward Chaining)
// Evaluates simple IF-THEN rules to predict vulnerability.

Rule rules[8] = {
  {
    "Single point of failure with no redundancy",
    "CRITICAL: Region isolation imminent if this node fails",
    "Add at least 2 redundant cables to bypass this node",
    30
  },
  {
    "Capacity usage > 80% with zero backup links",
    "HIGH: Overload cascade failure likely within 15 minutes",
    "Deploy load balancing and provision backup bandwidth",
    25
  },
  {
    "Satellite coverage < 30% and submarine cable failed",
    "HIGH: Region will lose all connectivity",
    "Emergency satellite uplink activation required",
    20
  },
  {
    "Data center offline with no CDN backup in region",
    "MEDIUM: Service degradation for 2-4 hours expected",
    "Activate CDN failover and reroute DNS records",
    15
  },
  {
    "Two or more cables failed in same region",
    "CRITICAL: Full regional blackout in progress",
    "Activate emergency satellite and cross-region rerouting",
    35
  },
  {
    "Node in high seismic/weather risk zone (|lat|>30)",
    "HIGH: Physical infrastructure damage risk elevated",
    "Harden cable landing stations and add armored segments",
    20
  },
  {
    "Traffic spike > 150% of normal and single active route",
    "HIGH: Route saturation causing packet loss > 40%",
    "Implement traffic shaping and QoS prioritization",
    25
  },
  {
    "Recovery time > 60 minutes in critical region",
    "MEDIUM: Extended outage affecting business continuity",
    "Pre-position repair ships and negotiate SLA with carriers",
    10
  }
};

int evaluate_node(Graph* g, int node_id) {
    Node n = g->nodes[node_id];
    int score = 0;
    
    // Rule 0
    if (g->adj_count[node_id] <= 1) {
        score += rules[0].risk_boost;
    }
    
    // Rule 1
    if (n.capacity > 0 && n.current_load > n.capacity * 0.8 && n.backup_links == 0) {
        score += rules[1].risk_boost;
    }
    
    // Rule 5
    if (fabs(n.latitude) > 30.0f) {
        score += rules[5].risk_boost;
    }
    
    if (score > 100) score = 100;
    return score;
}

void run_expert_system(Graph* g) {
    for (int i = 0; i < g->node_count; i++) {
        int extra_risk = evaluate_node(g, i);
        g->nodes[i].risk_score += extra_risk;
        if (g->nodes[i].risk_score > 100.0f) {
            g->nodes[i].risk_score = 100.0f;
        }
    }
}
