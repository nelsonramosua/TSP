// TravelingSalesmanProblem.h
#ifndef _TRAVELING_SALESMAN_PROBLEM_H_
#define _TRAVELING_SALESMAN_PROBLEM_H_

#include "Graph.h"      // The provided Graph ADT interface
#include "SortedList.h" // The provided SortedList ADT interface
#include <stdio.h>
#include <limits.h>
#include <float.h>

// --- TOUR ADT DEFINITION ---
typedef struct _Tour {
    unsigned int* path;      // Array of vertices in the tour (V1, V2, ..., VN, V1)
    unsigned int numVertices; // Number of vertices in the path (N+1 for closed tour)
    double cost;            // Total cost (length) of the tour
} Tour; 

// Function prototypes for the Tour ADT (Implemented in Tour.c)
Tour* TourCreate(unsigned int numVertices);
void TourDestroy(Tour** p);
void TourDisplay(const Tour* t);
double TourGetCost(const Tour* t);

// --- ALGORITHM INTERFACES ---

// Lower Bounding / MST (Implemented in LowerBound_MST.c)
double LowerBound_MST(const Graph* g);

// Brute-force method (Implemented in ExhaustiveSearch.c)
Tour* ExhaustiveSearch_FindTour(const Graph* g);

// Heuristics (Implemented in their respective .c files)
Tour* NearestNeighbour_FindTour(const Graph* g, unsigned int startVertex);
Tour* Greedy_FindTour(const Graph* g);
Tour* Christofides_FindTour(const Graph* g);

// Improvement (Implemented in TwoOpt.c)
Tour* TwoOpt_ImproveTour(const Graph* g, Tour* initialTour);

// Metaheuristics (Implemented in their respective .c files)
Tour* SimulatedAnnealing_FindTour(const Graph* g, unsigned int* initialTour);
Tour* AntColony_FindTour(const Graph* g);

// Graph Helpers
double GetEdgeWeight(const Graph* g, unsigned int v, unsigned int w);

#endif // _TRAVELING_SALESMAN_PROBLEM_H_