// Metaheuristics.h - Defines some configuration for metaheuristic implementations.
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

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

// Configuration for neighbour-list local search (2-Opt / Or-Opt)
#define LS_NEIGHBOURS 16  // candidate-list size per vertex; for N <= LS_NEIGHBOURS+1 the list is the whole graph, so the search is a full (unrestricted) 2-Opt / Or-Opt.

// Configuration for Lin-Kernighan (LK)
#define LK_NEIGHBOURS 10  // candidate-list size per vertex
#define LK_MAX_DEPTH  6   // maximum chain length
#define LK_EPS        1e-9

// Configuration for Tabu Search (TS)
#define TABU_TENURE 15         // # iterations a reversed edge stays tabu
#define TABU_MULTIPLIER 40     // # iterations = TABU_MULTIPLIER * numVertices

// Configuration for GRASP (Greedy Randomized Adaptive Search Procedure)
#define GRASP_ITERATIONS 50    // # of randomized-greedy construction + 2-Opt restarts
#define GRASP_ALPHA 0.3        // restricted-candidate-list greediness: 0 = pure greedy, 1 = fully random

// Configuration for ISPO (discrete PSO with "mobile operators" + SA neighbourhood; Wang, Mu & Zhu 2013)
#define ISPO_PARTICLES 40      // swarm size (paper: 40)
#define ISPO_ITERATIONS 120    // main-loop iterations (each descends every particle to a 2-Opt optimum)
#define ISPO_W_MAX 0.9         // inertia weight at iteration 0 (retain prob for the old velocity)
#define ISPO_W_MIN 0.4         // inertia weight at the last iteration (linearly decreased)
#define ISPO_C1 1.0            // cognitive coeff: retain each pbest-difference operator w.p. C1*rand
#define ISPO_C2 1.0            // social coeff: retain each gbest-difference operator w.p. C2*rand
#define ISPO_STAGNATION 10     // m: re-initialize velocities after this many stagnant iterations

#endif // METAHEURISTICS_H