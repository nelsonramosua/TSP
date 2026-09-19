// FarthestInsertion.c - Implements the Farthest Insertion constructive heuristic for TSP.
//
// O(N^3).
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Sibling of NearestInsertion.c: same "insert at the cheapest position" machinery, but the vertex chosen to insert next is the one *farthest* from the current tour (max over unvisited of its distance to the nearest tour vertex).
// Grabbing outliers first tends to sketch the tour's outline early and usually beats Nearest Insertion in practice.
//
// (There is no separate "Cheapest Insertion" here: the Greedy heuristic already is cheapest insertion -- it inserts the globally cheapest (vertex, position) pair each step.)

// Resources Used:
// https://youtu.be/W3n6p58mClI?si=bakuahmf1Xnmt9Su

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/Trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// Forward declarations:

// finds best pos to insert a new vertex into partial tour to minimize the resulting cost delta
static unsigned int findOptimalInsertionPoint(const Graph* g, unsigned int* tour, unsigned int pathLength, unsigned int vK);
// shift arr elems to "make space" for insertion at a specified index
static void shiftAndInsert(unsigned int* tour, unsigned int insertIndex, unsigned int pathLength, unsigned int vK);
// calculates the total cost of a given tour
static double calculateTourCost(const Graph* g, const Tour* tour);

Tour* FarthestInsertion_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices < 2) return NULL;

    Tour* finalTour = TourCreate(numVertices);
    if (!finalTour) return NULL;

    unsigned int* partialPath = malloc(numVertices * sizeof(unsigned int));
    if (!partialPath) { TourDestroy(&finalTour); return NULL; }

    int* isVisited = (int*) calloc(numVertices, sizeof(int)); // 0 is unvisited, 1 is visited
    if (!isVisited) { free(partialPath); TourDestroy(&finalTour); return NULL; }

    // init: start with the two FARTHEST-apart vertices (vU, vV)
    unsigned int vU = 0, vV = 1;
    double maxCost = -1.0;
    for (unsigned int i = 0; i < numVertices; i++)
        for (unsigned int j = i + 1; j < numVertices; j++) {
            double cost = GetEdgeWeight(g, i, j);
            if (cost != DBL_MAX && cost > maxCost) {
                maxCost = cost;
                vU = i; vV = j;
            }
        }

    // init partial tour
    partialPath[0] = vU; partialPath[1] = vV;
    isVisited[vU] = 1; isVisited[vV] = 1;
    unsigned int pathLength = 2;

    // insert remaining N-2 verts
    while (pathLength < numVertices) {
        double maxDistance = -1.0;
        unsigned int vK = UINT_MAX; // next vert to insert

        // find unvisited vK whose distance to its NEAREST tour vertex is the largest
        for (unsigned int k = 0; k < numVertices; k++) {
            if (!isVisited[k]) {
                double nearestDist = DBL_MAX;
                for (unsigned int i = 0; i < pathLength; i++) {
                    double dist = GetEdgeWeight(g, k, partialPath[i]);
                    if (dist < nearestDist) nearestDist = dist;
                }

                // pick the farthest such vertex (the outlier)
                if (nearestDist != DBL_MAX && nearestDist > maxDistance) { maxDistance = nearestDist; vK = k; }
            }
        }

        // no reachable unvisited vert found -> break (discon. graph)
        if (vK == UINT_MAX) break;

        // find best placement ind for vK, then shift & insert
        unsigned int bestIndex = findOptimalInsertionPoint(g, partialPath, pathLength, vK);
        shiftAndInsert(partialPath, bestIndex, pathLength, vK);

        isVisited[vK] = 1;
        pathLength++;

        // frame: the growing closed subtour (only assembled when an animation sink is installed)
        if (TraceActive()) {
            unsigned int frame[pathLength + 1];
            double c = 0.0;
            for (unsigned int i = 0; i < pathLength; i++) {
                frame[i] = partialPath[i];
                double w = GetEdgeWeight(g, partialPath[i], partialPath[(i + 1) % pathLength]);
                if (w != DBL_MAX) c += w;
            }
            frame[pathLength] = partialPath[0]; // repeat first vertex => "closed" marker for the renderer
            TraceEmit(frame, pathLength + 1, numVertices, c);
        }
    }

    // copy path to final Tour struct.
    for (unsigned int i = 0; i < numVertices; i++) finalTour->path[i] = partialPath[i];
    finalTour->path[numVertices] = finalTour->path[0]; // close cycle

    finalTour->cost = calculateTourCost(g, finalTour);

    free(partialPath); free(isVisited);
    return finalTour;
}

// finds best pos to insert a new vertex into partial tour to minimize the resulting cost delta
static unsigned int findOptimalInsertionPoint(const Graph* g, unsigned int* tour, unsigned int pathLength, unsigned int vK) {
    double minCostIncrease = DBL_MAX;
    unsigned int bestIndex = 0;

    for (unsigned int i = 0; i < pathLength; i++) {
        unsigned int vA = tour[i];
        unsigned int vB = tour[(i + 1) % pathLength]; // wrap for the closing edge

        double costAB = GetEdgeWeight(g, vA, vB);
        double costAK = GetEdgeWeight(g, vA, vK);
        double costKB = GetEdgeWeight(g, vK, vB);

        if (costAB == DBL_MAX || costAK == DBL_MAX || costKB == DBL_MAX) continue; // invalid edges

        double costDelta = (costAK + costKB) - costAB;

        if (costDelta < minCostIncrease) {
            minCostIncrease = costDelta;
            bestIndex = (i + 1) % pathLength; // vK inserted between vA and vB
        }
    }
    return bestIndex;
}

// shift arr elems to "make space" for insertion at a specified index
static void shiftAndInsert(unsigned int* tour, unsigned int insertIndex, unsigned int pathLength, unsigned int vK) {
    for (unsigned int i = pathLength; i > insertIndex; i--) tour[i] = tour[i - 1];
    tour[insertIndex] = vK;
}

// calculates the total cost of a given tour
static double calculateTourCost(const Graph* g, const Tour* tour) {
    double totalCost = 0.0;
    unsigned int numEdges = tour->numVertices - 1;

    for (unsigned int i = 0; i < numEdges; i++) {
        unsigned int vA = tour->path[i];
        unsigned int vB = tour->path[i + 1];
        double edgeCost = GetEdgeWeight(g, vA, vB);

        if (edgeCost == DBL_MAX) return DBL_MAX;

        totalCost += edgeCost;
    }

    return totalCost;
}