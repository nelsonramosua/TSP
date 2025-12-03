// SimulatedAnnealing.c - Implements the Simulated Annealing (SA) metaheuristic for TSP.
// 
// O(N^2 * iterations) = O(N^3 * SA_MULTIPLIER).
//
// Nelson Ramos, 124921.
//
// November, 2025.
// 
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit.

// Resources used:
// https://youtu.be/1kgbwosVUPs?si=mo5kmlKF9NMWHPg2

// Tune parameters in headers/Metaheuristics.h.

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/Metaheuristics.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <string.h> 

static void swap(unsigned int* a, unsigned int* b);
static void twoOptSwap(unsigned int* tour, unsigned int i, unsigned int j);
static double tourCost(const Graph* g, unsigned int* tour, unsigned int numVertices);

Tour* SimulatedAnnealing_FindTour(const Graph* g, unsigned int* initialTour) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices < 2) return NULL; // Need at least 2 vertices

    Tour* tour = TourCreate(numVertices);
    if (!tour) return NULL;

    srand((unsigned int)time(NULL)); 

    unsigned int* current = malloc(numVertices * sizeof(unsigned int));
    unsigned int* bestTour = malloc(numVertices * sizeof(unsigned int));
    if (!current || !bestTour) { 
        if (current) free(current); 
        if (bestTour) free(bestTour); 
        TourDestroy(&tour); 
        return NULL; 
    }

    memcpy(current, initialTour, numVertices * sizeof(unsigned int));
    memcpy(bestTour, initialTour, numVertices * sizeof(unsigned int));

    double currentCost = tourCost(g, current, numVertices);
    double bestCost = currentCost;

    if (bestCost == DBL_MAX) {
        fprintf(stderr, "[SA ERROR] Initial tour is invalid (DBL_MAX cost).\n");
        free(current);
        free(bestTour);
        TourDestroy(&tour);
        return NULL;
    }

    double T = currentCost / 10.0; // start T is the init "heat" of the system
    if (T < 100.0) T = 100.0;     // ensure min starting T
    
    double Tmin = SA_MIN_TEMP; // when is the system frozen? (stop crit.)
    double alpha = SA_COOLING_RATE; // cooling rate
    unsigned int iterations = SA_MULTIPLIER * numVertices; // swap num at each temperature

    while (T > Tmin) {
        for (unsigned int k = 0; k < iterations; k++) {
            unsigned int i, j;
            
            // gen 2 distinct inds.
            do {
                i = rand() % numVertices;
                j = rand() % numVertices;
                // segment to be reversed is between the edge (i, i+1) and (j, j+1)
            } while (i == j || i == (j+1)%numVertices || j == (i+1)%numVertices); // make sure of non-adjacent edges

            if (i > j) swap(&i, &j); // make sure i < j for simpler segment reversal
            
            unsigned int a = current[i]; // left node of first removed edge (a)
            unsigned int b = current[(i + 1) % numVertices]; // right node of first removed edge (b)
            unsigned int c = current[j]; // left node of second removed edge (C)
            unsigned int d = current[(j + 1) % numVertices]; // right node of second removed edge (D)
            
            // removed edges: (A, B) and (C, D)
            double weightAB = GetEdgeWeight(g, a, b);
            double weightCD = GetEdgeWeight(g, c, d);
            // added edges: (A, C) and (B, D)
            double weightAC = GetEdgeWeight(g, a, c);
            double weightBD = GetEdgeWeight(g, b, d);

            if (weightAB == DBL_MAX || weightCD == DBL_MAX || weightAC == DBL_MAX || weightBD == DBL_MAX) continue;

            double delta = -weightAB - weightCD + weightAC + weightBD;

            // swap with Metropolis?(hastings?) criterion
            if (delta < 0 || exp(-delta / T) > (double)rand() / RAND_MAX) {
                // 2-opt swap on segment [i+1, j]
                unsigned int segmentStart = (i + 1) % numVertices;
                unsigned int segmentEnd = j;
                
                twoOptSwap(current, segmentStart, segmentEnd);

                currentCost += delta;

                // update best
                if (currentCost < bestCost) {
                    bestCost = currentCost;
                    memcpy(bestTour, current, numVertices * sizeof(unsigned int));
                }
            }
        }
        T *= alpha; // cool down
    }

    for (unsigned int i = 0; i < numVertices; i++) tour->path[i] = bestTour[i];
    tour->path[numVertices] = bestTour[0]; // close cycle
    tour->cost = bestCost;

    free(current);
    free(bestTour);
    return tour;
}

// swap two unsigned int
static void swap(unsigned int* a, unsigned int* b) {
    unsigned int t = *a;
    *a = *b;
    *b = t;
}

// does a 2-opt swap on a tour by reversing the segment between indices i and j
static void twoOptSwap(unsigned int* tour, unsigned int i, unsigned int j) {
    while (i < j) swap(&tour[i++], &tour[j--]);
}

// calc tour cost
static double tourCost(const Graph* g, unsigned int* tour, unsigned int numVertices) {
    double cost = 0.0;
    // Iterate N times (N edges in the cycle)
    for (unsigned int i = 0; i < numVertices; i++) {
        // Edge is (tour[i], tour[(i + 1) % N])
        double w = GetEdgeWeight(g, tour[i], tour[(i + 1) % numVertices]);
        if (w == DBL_MAX) return DBL_MAX; // invalid edge
        cost += w;
    }
    return cost;
}