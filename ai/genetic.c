#include "genetic.h"
#include "astar.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    for (int i = 0; i < POPULATION_SIZE; i++) {
        float fitness = 0.0f;
        for (int test = 0; test < 5; test++) {
            for(int n=0; n<base_g->node_count; n++) {
                base_g->nodes[n].capacity += p->individuals[i].added_capacities[n];
            }
            
            int f1 = rand() % base_g->node_count;
            int f2 = rand() % base_g->node_count;
            int f3 = rand() % base_g->node_count;
            int temp1 = base_g->nodes[f1].is_active; base_g->nodes[f1].is_active = 0;
            int temp2 = base_g->nodes[f2].is_active; base_g->nodes[f2].is_active = 0;
            int temp3 = base_g->nodes[f3].is_active; base_g->nodes[f3].is_active = 0;
            
            int reachable = count_reachable_astar(base_g, 0); 
            
            int overloads = 0;
            for(int n=0; n<base_g->node_count; n++) {
                if (base_g->nodes[n].is_active && base_g->nodes[n].capacity > 0) {
                    if (base_g->nodes[n].current_load > base_g->nodes[n].capacity) {
                        overloads++;
                    }
                }
            }
            
            fitness += (float)reachable * 10.0f - (float)overloads * 5.0f;
            
            base_g->nodes[f1].is_active = temp1;
            base_g->nodes[f2].is_active = temp2;
            base_g->nodes[f3].is_active = temp3;
            for(int n=0; n<base_g->node_count; n++) {
                base_g->nodes[n].capacity -= p->individuals[i].added_capacities[n];
            }
        }
        p->individuals[i].fitness = fitness / 5.0f;
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
        
        if (gen % (generations/5 == 0 ? 1 : generations/5) == 0 || gen == generations - 1) {
            printf("Generation %d: Best Fitness = %.2f\n", gen+1, pop.individuals[best_idx].fitness);
        }
        
        Population next_pop;
        next_pop.individuals[0] = pop.individuals[best_idx];
        int second_best = (best_idx == 0) ? 1 : 0;
        next_pop.individuals[1] = pop.individuals[second_best];
        
        for (int i = 2; i < POPULATION_SIZE; i++) {
            int t1 = rand() % POPULATION_SIZE;
            int t2 = rand() % POPULATION_SIZE;
            Chromosome* p1 = (pop.individuals[t1].fitness > pop.individuals[t2].fitness) ? &pop.individuals[t1] : &pop.individuals[t2];
            
            t1 = rand() % POPULATION_SIZE;
            t2 = rand() % POPULATION_SIZE;
            Chromosome* p2 = (pop.individuals[t1].fitness > pop.individuals[t2].fitness) ? &pop.individuals[t1] : &pop.individuals[t2];
            
            next_pop.individuals[i] = crossover(p1, p2, g->node_count, budget);
            mutate(&next_pop.individuals[i], g->node_count, 0.1f, budget);
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
