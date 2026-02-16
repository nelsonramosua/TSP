// TwoOpt.c - Implements the 2-Opt local search algorithm.
//
// O(N^3).
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// This could be improved for O(E * log N) if a priority queue / min-heap was used... For simplicty purposes, it wasn't. Try!

// Resources used:
// https://en.wikipedia.org/wiki/2-opt

#include "../../TravelingSalesmanProblem.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// reverses segment of tour path between inds startIndex and endIndex
static void TwoOpt_Swap(Tour* tour, unsigned int startIndex, unsigned int endIndex);

// tries to improve existing TSP tour using 2-Opt local search method.
Tour* TwoOpt_ImproveTour(const Graph* g, Tour* initialTour) {
    if (!initialTour || initialTour->numVertices <= 2) return initialTour;

    unsigned int numVertices = GraphGetNumVertices(g);
    int improved = 1;

    printf("  Starting 2-Opt. Initial Cost: %.2f\n", initialTour->cost);

    // do until no improvement
    while (improved) {
        improved = 0;

        // iterate over all possible non-adjacent edge pairs (i-1, i) and (j, j+1)
        for (unsigned int i = 1; i < numVertices; i++) {
            for (unsigned int j = i + 1; j < numVertices; j++) {
                // skip adjacent edges and closing edge
                if (j == i || j == i + 1 || j == numVertices) continue;

                // four verts defining the two edges
                unsigned int v1 = initialTour->path[i-1];
                unsigned int v2 = initialTour->path[i];
                unsigned int v3 = initialTour->path[j];
                unsigned int v4 = initialTour->path[j+1]; // note: j+1 can be N (start vertex)

                // calc old costs
                double costV1V2 = GetEdgeWeight(g, v1, v2);
                double costV3V4 = GetEdgeWeight(g, v3, v4);
                double costOld = costV1V2 + costV3V4;

                // calc new costs
                double costV1V3 = GetEdgeWeight(g, v1, v3);
                double costV2V4 = GetEdgeWeight(g, v2, v4);
                double costNew = costV1V3 + costV2V4;

                // see improvement
                double costDelta = costNew - costOld;

                if (costDelta < -DBL_EPSILON) { // see if is significant (-ve delta)
                    // improv found! do swap and update cost
                    TwoOpt_Swap(initialTour, i, j);
                    initialTour->cost += costDelta;
                    improved = 1;

                    // restart outer loop after improv
                    goto restartLoops; // could have redesigned this to not use goto but i'm too lazy.
                }
            }
        }

        restartLoops:;
    }

    printf("  Final Cost after 2-Opt: %.2f\n", initialTour->cost);
    return initialTour;
}

// reverses segment of tour path between inds startIndex and endIndex
static void TwoOpt_Swap(Tour* tour, unsigned int startIndex, unsigned int endIndex) {
    while (startIndex < endIndex) {
        unsigned int temp = tour->path[startIndex];
        tour->path[startIndex] = tour->path[endIndex];
        tour->path[endIndex] = temp;
        startIndex++;
        endIndex--;
    }
}