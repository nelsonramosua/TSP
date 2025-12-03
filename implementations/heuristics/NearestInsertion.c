// NearestInsertion.c - Implements the Nearest Insertion constructive heuristic for TSP.
//
// O(N^3).
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit.

// Resources Used:
// https://youtu.be/W3n6p58mClI?si=bakuahmf1Xnmt9Su

#include "../../TravelingSalesmanProblem.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// forward declarations:

// finds best pos to insert a new vertex into partial tour to minimize the resulting cost delta
static unsigned int findOptimalInsertionPoint(const Graph* g, unsigned int* tour, unsigned int pathLength, unsigned int vK);
// shift arr elems to "make space" for insertion at a specified index
static void shiftAndInsert(unsigned int* tour, unsigned int insertIndex, unsigned int pathLength, unsigned int vK);
// calculates the total cost of a given tour
static double calculateTourCost(const Graph* g, const Tour* tour);

Tour* NearestInsertion_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices < 2) return NULL;

    Tour* finalTour = TourCreate(numVertices);
    if (!finalTour) return NULL;

    unsigned int* partialPath = malloc(numVertices * sizeof(unsigned int));
    if (!partialPath) { TourDestroy(&finalTour); return NULL; }
    
    int* isVisited = (int*) calloc(numVertices, sizeof(int)); // 0 is unvisited, 1 is visited
    if (!isVisited) { free(partialPath); TourDestroy(&finalTour); return NULL; }

    // init: start with two closest verts
    unsigned int vU = 0, vV = 1; 
    double minCost = DBL_MAX;

    // find pair of closest verts (vU, vV)
    for (unsigned int i = 0; i < numVertices; i++) 
        for (unsigned int j = i + 1; j < numVertices; j++) {
            double cost = GetEdgeWeight(g, i, j);
            if (cost < minCost) {
                minCost = cost;
                vU = i;
                vV = j;
            }
        }


    // init partial tour
    partialPath[0] = vU;
    partialPath[1] = vV;
    isVisited[vU] = 1;
    isVisited[vV] = 1;
    unsigned int pathLength = 2;

    // insert remaining N-2 verts
    while (pathLength < numVertices) {
        double minDistance = DBL_MAX;
        unsigned int vK = UINT_MAX; // next vert to insert

        // find unvisited vK that is closest to ANY vert in partial tour
        for (unsigned int k = 0; k < numVertices; k++) {
            if (!isVisited[k]) {
                double nearestDist = DBL_MAX;
                // find dist from vK to nearest vert in cur tour
                for (unsigned int i = 0; i < pathLength; i++) {
                    double dist = GetEdgeWeight(g, k, partialPath[i]);
                    if (dist < nearestDist) nearestDist = dist;
                }

                if (nearestDist < minDistance) {
                    minDistance = nearestDist;
                    vK = k;
                }
            }
        }

        // no unvisited vert found -> break (discon. graph)
        if (vK == UINT_MAX) break;

        // find best placement ind for vK
        unsigned int bestIndex = findOptimalInsertionPoint(g, partialPath, pathLength, vK);

        // shift arr elems,  insert vK
        shiftAndInsert(partialPath, bestIndex, pathLength, vK);
        
        isVisited[vK] = 1;
        pathLength++;
    }

    // copy path to final Tour struct.
    for (unsigned int i = 0; i < numVertices; i++) finalTour->path[i] = partialPath[i];
    finalTour->path[numVertices] = finalTour->path[0]; // close cycle
    
    // calc. final cost
    finalTour->cost = calculateTourCost(g, finalTour); 

    free(partialPath);
    free(isVisited);
    return finalTour;
}

// finds best pos to insert a new vertex into partial tour to minimize the resulting cost delta
static unsigned int findOptimalInsertionPoint(const Graph* g, unsigned int* tour, unsigned int pathLength, unsigned int vK) {
    double minCostIncrease = DBL_MAX;
    unsigned int bestIndex = 0;

    for (unsigned int i = 0; i < pathLength; i++) {
        unsigned int vA = tour[i];
        // index of next vertex (wrapping around for closing edge!)
        unsigned int vB = tour[(i + 1) % pathLength]; 
        
        // cost of removed edge
        double costAB = GetEdgeWeight(g, vA, vB);
        // added edges cost: (A, K) + (K, B)
        double costAK = GetEdgeWeight(g, vA, vK);
        double costKB = GetEdgeWeight(g, vK, vB);
        
        if (costAB == DBL_MAX || costAK == DBL_MAX || costKB == DBL_MAX) continue; // invalid edges

        // costDelta = (Cost_new - Cost_old)
        double costDelta = (costAK + costKB) - (costAB);

        if (costDelta < minCostIncrease) {
            minCostIncrease = costDelta;
            // best ind is 'i+1' (vK inserted between vA and vB)
            bestIndex = (i + 1) % pathLength; 
        }
    }
    return bestIndex;
}


// shift arr elems to "make space" for insertion at a specified index
static void shiftAndInsert(unsigned int* tour, unsigned int insertIndex, unsigned int pathLength, unsigned int vK) {
    // shift elems from end to insertIndex
    for (unsigned int i = pathLength; i > insertIndex; i--) tour[i] = tour[i - 1];
    // place new vertex
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