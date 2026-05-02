#ifndef GENETIC_H
#define GENETIC_H

#include "../graph/topology.h"

#define MAX_GENETIC_NODES 1000
#define POPULATION_SIZE 50

typedef struct Chromosome {
    int added_capacities[MAX_GENETIC_NODES];
    float fitness;
} Chromosome;

typedef struct Population {
    Chromosome individuals[POPULATION_SIZE];
} Population;

void init_population(Population* p, Graph* base_g, int budget);
void evaluate_fitness(Population* p, Graph* base_g);
Chromosome crossover(Chromosome* p1, Chromosome* p2, int node_count, int budget);
void mutate(Chromosome* c, int node_count, float mutation_rate, int budget);
void optimize_hardening(Graph* g, int generations, int budget);

#endif
