// TravelingSalesmanProblem.h - Project Header.
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit.

// https://youtu.be/GiDsjIBOVoA?si=zEtD3WFBlUYQO_LN
// Read README.md.

#ifndef _TRAVELING_SALESMAN_PROBLEM_H_
#define _TRAVELING_SALESMAN_PROBLEM_H_

#include "headers/Graph.h"      // The Graph ADT interface (All Credit to AED Professors)
#include "headers/SortedList.h" // The SortedList ADT interface (All Credit to AED Professors)
#include "headers/NamedGraph.h"     // Optional display with city names

#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct _Tour {
    unsigned int* path;          // Array of vertices in the tour (V1, V2, ..., VN, V1)
    unsigned int numVertices;    // Number of vertices in the path 
    double cost;                 // Total cost of the tour

    const char** cityNames;      // Optional city names for display
} Tour; 

// Function prototypes for Tour ADT (Implemented in Tour.c)
Tour* TourCreate(unsigned int numVertices);
Tour* TourDeepCopy(const Tour* src);
void TourDestroy(Tour** p);
void TourDisplay(const Tour* t);
double TourGetCost(const Tour* t);
int TourInvariant(const Tour* t, unsigned int numVertices);

void TourMapCityNames(Tour* t, const NamedGraph* ng);

// Lower Boundings
double LowerBound_MST(const Graph* g);
double LowerBound_HeldKarp(const Graph* g);

// Brute-force method (Implemented in ExhaustiveSearch.c)
Tour* ExhaustiveSearch_FindTour(const Graph* g);
Tour* ExhaustiveSearchPruning_FindTour(const Graph* g);

Tour* HeldKarp_FindTour(const Graph* g);

// Heuristics (Implemented in their respective .c files)
Tour* NearestNeighbour_FindTour(const Graph* g, unsigned int startVertex);
Tour* Greedy_FindTour(const Graph* g);
Tour* NearestInsertion_FindTour(const Graph* g);
Tour* Christofides_FindTour(const Graph* g);

// Improvement (Implemented in TwoOpt.c)
Tour* TwoOpt_ImproveTour(const Graph* g, Tour* initialTour);

// Metaheuristics (Implemented in their respective .c files)
Tour* SimulatedAnnealing_FindTour(const Graph* g, unsigned int* initialTour);
Tour* AntColony_FindTour(const Graph* g);
Tour* GeneticAlgorithm_FindTour(const Graph* g);

// In the future, it could be interesting implementing IPSO:
// https://www.researchgate.net/publication/271285365_ISPO_A_New_Way_to_Solve_Traveling_Salesman_Problem

#endif // _TRAVELING_SALESMAN_PROBLEM_H_