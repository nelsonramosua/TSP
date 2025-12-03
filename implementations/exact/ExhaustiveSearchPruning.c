// ExhaustiveSearchPruning.c  - Solves TSP via Brute-force method with Pruning.
//
// Still O(N!).
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit.

#include "../../TravelingSalesmanProblem.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h> 
#include <limits.h>

// Helper function to swap two elements in an array.
static void swap(unsigned int* a, unsigned int* b);

// Recursive Helper with pruning.
static void recursiveTspHelper(unsigned int* fullPath, int startIndex, unsigned int N, const Graph* g, Tour* bestTour, double currentCost, unsigned int lastVertexInPath);

Tour* ExhaustiveSearchPruning_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices < 2) return NULL; // must have at least 3 vert.

    // create Best Tour struct.
    Tour* bestTour = TourCreate(numVertices);
    if (!bestTour) return NULL;

    // init best cost to biggest double (DBL_MAX)
    bestTour->cost = DBL_MAX;

    // ready array for permutation
    unsigned int* fullPath = (unsigned int*) malloc(numVertices * sizeof(unsigned int));
    if (!fullPath) { TourDestroy(&bestTour); return NULL; }

    // init path
    fullPath[0] = 0; 
    for (unsigned int i = 1; i < numVertices; i++) fullPath[i] = i; 
        // fullPath starts as [0, 1, 2, ..., N-1]
    
    // recursively solve. starts at index 1 (fixing the first vert.)
    recursiveTspHelper(fullPath, 1, numVertices, g, bestTour, 0.0, 0);

    free(fullPath);

    // graph connect?
    if (bestTour->cost == DBL_MAX) {
        fprintf(stderr, "Warning: Brute Force failed to find a path (graph disconnected?).\n");
        TourDestroy(&bestTour);
        return NULL;
    }
    
    return bestTour;
}

static void swap(unsigned int* a, unsigned int* b) {
    unsigned int temp = *a;
    *a = *b;
    *b = temp;
}

static void recursiveTspHelper(unsigned int* fullPath, int startIndex, unsigned int N, const Graph* g, Tour* bestTour, double currentCost, unsigned int lastVertexInPath) {
    // pruning Check (Branch and Bound)
    if (currentCost >= bestTour->cost) return; // PRUNED!

    if ((unsigned int)startIndex == N) { 
        unsigned int u = lastVertexInPath; // last city added
        unsigned int v = fullPath[0];      // starting city (0)
        double closingEdgeWeight = GetEdgeWeight(g, u, v);

        if (closingEdgeWeight == DBL_MAX) return; 

        double finalCost = currentCost + closingEdgeWeight;

        // update bestTour
        if (finalCost < bestTour->cost) {
            // new optimal Tour
            bestTour->cost = finalCost;
            for (unsigned int i = 0; i < N; i++) bestTour->path[i] = fullPath[i];
            bestTour->path[N] = fullPath[0]; // close loop
        }
        
        return;
    }

    // gen perms by swapping elements
    for (unsigned int i = startIndex; i < N; i++) {
        // edge to check:
        unsigned int nextVertex = fullPath[i];
        double edgeWeight = GetEdgeWeight(g, lastVertexInPath, nextVertex);

        // check if valid
        if (edgeWeight == DBL_MAX) continue; // skip
        
        // swap the element at startIndex with element at i
        swap(&fullPath[startIndex], &fullPath[i]);
        
        // move to the next index
        recursiveTspHelper(fullPath, startIndex + 1, N, g, bestTour, currentCost + edgeWeight, fullPath[startIndex]);
        
        // backtrack
        swap(&fullPath[startIndex], &fullPath[i]);
    }
}