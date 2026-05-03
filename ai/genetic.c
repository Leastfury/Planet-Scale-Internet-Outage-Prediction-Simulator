#include "genetic.h"
#include "astar.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static float normalized_reachability(Graph* g) {
    int active = 0;
    float total = 0.0f;

    for (int i = 0; i < g->node_count; i++) {
        if (!g->nodes[i].is_active) continue;
        total += (float)count_reachable_astar(g, i) / (float)g->node_count;
        active++;
    }

    if (active == 0) return 0.0f;
    return total / (float)active;
}

static int tournament_select(Chromosome* pop, int size) {
    int a = rand() % size;
    int b = rand() % size;
    return pop[a].fitness >= pop[b].fitness ? a : b;
}

void init_population(Population* p, Graph* base_g, int budget) {
    for (int i = 0; i < POPULATION_SIZE; i++) {
        p->individuals[i].fitness = 0.0f;
        int remaining_budget = budget;
        for (int n = 0; n < base_g->node_capacity; n++) p->individuals[i].added_capacities[n] = 0;
        
        while (remaining_budget > 0) {
            int node = rand() % base_g->node_count;
            int add = (rand() % 100) + 10;
            if (add > remaining_budget) add = remaining_budget;
            p->individuals[i].added_capacities[node] += add;
            remaining_budget -= add;
        }
    }
}

void evaluate_fitness(Population* p, Graph* base_g) {
    int base_capacity[MAX_GENETIC_NODES];
    int original_status[MAX_GENETIC_NODES];
    for (int n = 0; n < base_g->node_count; n++) {
        base_capacity[n] = base_g->nodes[n].capacity;
        original_status[n] = base_g->nodes[n].is_active;
    }

    for (int i = 0; i < POPULATION_SIZE; i++) {
        float total_gain = 0.0f;

        for (int test = 0; test < 10; test++) {
            int demand_mbps[MAX_GENETIC_NODES];
            for (int n = 0; n < base_g->node_count; n++) {
                // Keep demand very close to node capacity so small upgrades can help.
                int jitter = (rand() % 100) - 50;
                int demand = base_capacity[n] + jitter;
                if (demand < 0) demand = 0;
                demand_mbps[n] = demand;
            }

            // Baseline failures (before upgrades)
            for (int n = 0; n < base_g->node_count; n++) {
                base_g->nodes[n].is_active = (demand_mbps[n] <= base_capacity[n]) ? 1 : 0;
            }
            float before = normalized_reachability(base_g);

            // Failures after applying chromosome upgrades
            for (int n = 0; n < base_g->node_count; n++) {
                int upgraded_capacity = base_capacity[n] + p->individuals[i].added_capacities[n];
                base_g->nodes[n].is_active = (demand_mbps[n] <= upgraded_capacity) ? 1 : 0;
            }
            float after = normalized_reachability(base_g);

            total_gain += (after - before);

            // Restore statuses for next trial.
            for (int n = 0; n < base_g->node_count; n++) {
                base_g->nodes[n].is_active = original_status[n];
            }
        }
        p->individuals[i].fitness = total_gain / 10.0f;
        if (p->individuals[i].fitness < 0.0f) p->individuals[i].fitness = 0.0f;
    }

    for (int n = 0; n < base_g->node_count; n++) {
        base_g->nodes[n].is_active = original_status[n];
    }
}

Chromosome crossover(Chromosome* p1, Chromosome* p2, int node_count, int budget) {
    Chromosome child;
    child.fitness = 0.0f;
    int current_total = 0;
    
    for (int i = 0; i < node_count; i++) {
        if (rand() % 2 == 0) {
            child.added_capacities[i] = p1->added_capacities[i];
        } else {
            child.added_capacities[i] = p2->added_capacities[i];
        }
        current_total += child.added_capacities[i];
    }
    
    if (current_total > 0) {
        float scale = (float)budget / current_total;
        int new_total = 0;
        for(int i=0; i<node_count; i++) {
            child.added_capacities[i] = (int)(child.added_capacities[i] * scale);
            new_total += child.added_capacities[i];
        }
        while (new_total < budget) {
            child.added_capacities[rand() % node_count]++;
            new_total++;
        }
        while (new_total > budget) {
            int n = rand() % node_count;
            if (child.added_capacities[n] > 0) {
                child.added_capacities[n]--;
                new_total--;
            }
        }
    } else {
        // Fallback
        child.added_capacities[0] = budget;
    }
    return child;
}

void mutate(Chromosome* c, int node_count, float mutation_rate, int budget) {
    (void)budget; // not strictly needed if we just swap
    for (int i = 0; i < node_count; i++) {
        float r = (float)rand() / (float)RAND_MAX;
        if (r < mutation_rate) {
            int target = rand() % node_count;
            if (c->added_capacities[i] > 10) {
                c->added_capacities[i] -= 10;
                c->added_capacities[target] += 10;
            }
        }
    }
}

void optimize_hardening(Graph* g, int generations, int budget) {
    printf("\n[GENETIC] Optimizing network hardening...\n");
    printf("Budget: %d Mbps capacity upgrades\n", budget);
    
    Population pop;
    init_population(&pop, g, budget);
    
    Chromosome best_overall;
    best_overall.fitness = -99999.0f;
    for(int n=0; n<g->node_capacity; n++) best_overall.added_capacities[n] = 0;
    float global_best = 0.0f;
    
    for (int gen = 0; gen < generations; gen++) {
        evaluate_fitness(&pop, g);
        
        int best_idx = 0;
        for (int i = 1; i < POPULATION_SIZE; i++) {
            if (pop.individuals[i].fitness > pop.individuals[best_idx].fitness) {
                best_idx = i;
            }
        }
        
        if (pop.individuals[best_idx].fitness > best_overall.fitness) {
            best_overall = pop.individuals[best_idx];
        }
        if (pop.individuals[best_idx].fitness > global_best) {
            global_best = pop.individuals[best_idx].fitness;
        }
        
        if (gen % (generations/5 == 0 ? 1 : generations/5) == 0 || gen == generations - 1) {
            // Show best-so-far trend so evolution output is monotonic and readable.
            printf("Generation %d: Best Fitness = %.4f\n", gen+1, global_best);
        }
        
        Population next_pop;
        next_pop.individuals[0] = pop.individuals[best_idx];
        
        for (int i = 1; i < POPULATION_SIZE; i++) {
            int p1_idx = tournament_select(pop.individuals, POPULATION_SIZE);
            int p2_idx = tournament_select(pop.individuals, POPULATION_SIZE);
            Chromosome* p1 = &pop.individuals[p1_idx];
            Chromosome* p2 = &pop.individuals[p2_idx];
            
            next_pop.individuals[i] = crossover(p1, p2, g->node_count, budget);
            mutate(&next_pop.individuals[i], g->node_count, 0.15f, budget);
        }
        
        pop = next_pop;
    }
    
    printf("\n[GENETIC] Optimization Complete. Recommended Upgrades:\n");
    for (int i = 0; i < g->node_count; i++) {
        if (best_overall.added_capacities[i] > 0) {
            printf("  + %d Mbps to Node %d (%s)\n", best_overall.added_capacities[i], i, g->nodes[i].name);
        }
    }
}
