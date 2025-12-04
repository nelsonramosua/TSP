// Metaheuristics.h - Defines some configuration for metaheuristic implementations.
//
// Nelson Ramos, 124921.
//
// November, 2025.
// 
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit. 

#ifndef METAHEURISTICS_H
#define METAHEURISTICS_H

#include "../TravelingSalesmanProblem.h"

// Configuration for Simulated Annealing (SA)
#define SA_MIN_TEMP 1e-6 // when to stop algorithm
#define SA_COOLING_RATE 0.90 // how quickly temperature decreases (0 - 1)
#define SA_MULTIPLIER 50 // # iterations = SA_MULTIPLIER * numVertices

// Configuration for Ant Colony Optimization (ACO)
#define ACO_ALPHA 1.0 // pheromone influence
#define ACO_BETA 4.0  // heuristic influence (1/distance)
#define ACO_RHO 0.2   // pheromone evaporation rate
#define ACO_Q 100.0   // pheromone deposit factor
#define ACO_ITERATIONS 200
#define ACO_MULTIPLIER 1.0 // # ants = ACO_MULTIPLIER * numVertices

// Configuration for Genetic Algorithm (GA)
#define GA_POPULATION_SIZE 200 // # individuals in the population (for each generation)
#define GA_NUM_GENERATIONS 2500 // # generations 
#define GA_MUTATION_RATE 0.05 // probability of mutation in individual (0 - 1)
#define GA_ELITISM_COUNT 5 // # top-performing individuals preserved for next gen
#define GA_TOURNAMENT_SIZE 7 // # individuals in tournament selection

#endif // METAHEURISTICS_H