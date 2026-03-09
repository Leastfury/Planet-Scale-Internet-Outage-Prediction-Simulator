#include "visualizer.h"
#include "../simulation/metrics.h"
#include <stdio.h>

const char* get_status_label(int pct) {
    if (pct > 70) return "ONLINE  ";
    if (pct > 30) return "DEGRADED";
    return "OFFLINE ";
}

const char* get_bar(int pct) {
    static char bar[16];
    int filled = (int)((pct / 100.0f) * 6);
    if (filled > 6) filled = 6;
    if (filled < 0) filled = 0;
    
    for (int i = 0; i < 6; i++) {
        if (i < filled) bar[i] = '#';
        else bar[i] = '.';
    }
    bar[6] = '\0';
    return bar;
}

void print_ascii_map(Graph* g) {
    (void)g;
    
    char na_bar[8], eu_bar[8], sa_bar[8], af_bar[8], as_bar[8], oc_bar[8];
    sprintf(na_bar, "%s", get_bar(g_regions[REGION_NA].connectivity_pct));
    sprintf(eu_bar, "%s", get_bar(g_regions[REGION_EU].connectivity_pct));
    sprintf(sa_bar, "%s", get_bar(g_regions[REGION_SA].connectivity_pct));
    sprintf(af_bar, "%s", get_bar(g_regions[REGION_AF].connectivity_pct));
    sprintf(as_bar, "%s", get_bar(g_regions[REGION_AS].connectivity_pct));
    sprintf(oc_bar, "%s", get_bar(g_regions[REGION_OC].connectivity_pct));
    
    printf("\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|          PLANET-SCALE OUTAGE SIMULATOR  v1.0                 |\n");
    printf("|                  GLOBAL NETWORK STATUS                       |\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|  Network Map Updated Live                                    |\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|                                                              |\n");
    printf("|   [NA %s %s %3d%%]    [EU %s %s %3d%%]   |\n",
        na_bar, get_status_label(g_regions[REGION_NA].connectivity_pct), g_regions[REGION_NA].connectivity_pct,
        eu_bar, get_status_label(g_regions[REGION_EU].connectivity_pct), g_regions[REGION_EU].connectivity_pct);
    printf("|          \\                          /                        |\n");
    printf("|           ~~~~~~~~~~ATLANTIC~~~~~~~~                         |\n");
    printf("|          /                          \\                        |\n");
    printf("|  [SA %s %s %3d%%]   [AF %s %s %3d%%]     |\n",
        sa_bar, get_status_label(g_regions[REGION_SA].connectivity_pct), g_regions[REGION_SA].connectivity_pct,
        af_bar, get_status_label(g_regions[REGION_AF].connectivity_pct), g_regions[REGION_AF].connectivity_pct);
    printf("|                  \\                  /                        |\n");
    printf("|                   ~~~~~~INDIAN~~~~~~                         |\n");
    printf("|                                    \\                         |\n");
    printf("|                    [AS %s %s %3d%%]                |\n",
        as_bar, get_status_label(g_regions[REGION_AS].connectivity_pct), g_regions[REGION_AS].connectivity_pct);
    printf("|                              |                               |\n");
    printf("|                       ~~~~~PACIFIC~~~~~                      |\n");
    printf("|                              |                               |\n");
    printf("|                    [OC %s %s %3d%%]                 |\n",
        oc_bar, get_status_label(g_regions[REGION_OC].connectivity_pct), g_regions[REGION_OC].connectivity_pct);
    printf("|                                                              |\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|  # = Active node   . = Failed node                           |\n");
    printf("|  ONLINE >70%%   DEGRADED 30-70%%   OFFLINE <30%%                |\n");
    printf("+--------------------------------------------------------------+\n");
}
