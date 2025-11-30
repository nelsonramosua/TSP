// ExhaustiveSearch.c  - Solves TSP via Brute-force method.
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

// Recursive Helper.
static void recursiveTspHelper(unsigned int* currentPath, int startIndex, unsigned int N, const Graph* g, Tour* bestTour);

Tour* ExhaustiveSearch_FindTour(const Graph* g) {
    unsigned int N = GraphGetNumVertices(g);
    if (N < 2) return NULL; // must have at least 3 vert.

    // create Best Tour struct.
    Tour* bestTour = TourCreate(N);
    if (!bestTour) return NULL;

    // init best cost to biggest double (DBL_MAX)
    bestTour->cost = DBL_MAX;

    // ready array for permutation (1 to N-1)
    unsigned int* innerVertices = (unsigned int*) malloc((N - 1) * sizeof(unsigned int));
    if (!innerVertices) { TourDestroy(&bestTour); return NULL; }

    for (unsigned int i = 0; i < N - 1; i++) {
        // it holds atfirst [1, 2, ..., N-1]
        innerVertices[i] = i + 1; 
    }

    // recursively solve. starts at index 0 (fixing the first vert.)
    recursiveTspHelper(innerVertices, 0, N, g, bestTour);

    free(innerVertices);

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

static void recursiveTspHelper(unsigned int* currentPath, int startIndex, unsigned int N, const Graph* g, Tour* bestTour) {
    // Base Case: All N-1 inner vertices have been placed in the permutation.
    if ((unsigned int)startIndex == N - 1) { 
        Tour* currentTour = TourCreate(N);
        if (!currentTour) return; 
        
        currentTour->path[0] = 0; // start node 0
        
        // copy permutation into tour path
        for (unsigned int i = 0; i < N - 1; i++) {
            currentTour->path[i + 1] = currentPath[i];
        }
        currentTour->path[N] = 0; // close the loop back to 0

        // calc cost of tour.
        double currentCost = 0.0;
        for (unsigned int i = 0; i < N; i++) {
            unsigned int u = currentTour->path[i];
            unsigned int v = currentTour->path[i + 1];
            
            currentCost += GetEdgeWeight(g, u, v);
        }
        currentTour->cost = currentCost;

        // update bestTour
        if (currentCost < bestTour->cost) {
            // new optimal Tour
            bestTour->cost = currentCost;
            for (unsigned int i = 0; i <= N; i++) {
                bestTour->path[i] = currentTour->path[i];
            }
        }
        
        TourDestroy(&currentTour);
        return;
    }

    // gen perms by swapping elements
    for (unsigned int i = startIndex; i < N - 1; i++) {
        // swap the element at startIndex with element at i
        swap(&currentPath[startIndex], &currentPath[i]);
        
        // move to the next index
        recursiveTspHelper(currentPath, startIndex + 1, N, g, bestTour);
        
        // backtrack
        swap(&currentPath[startIndex], &currentPath[i]);
    }
}