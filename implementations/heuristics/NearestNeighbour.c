// NearestNeighbour.c - Implements Nearest Neighbour approximation algorithm.
//
// O(N^2).
//
// Nelson Ramos, 124921.
//
// November, 2025.
// 
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit.

#include "../../TravelingSalesmanProblem.h"

#include <stdlib.h>
#include <float.h>
#include <string.h> 

Tour* NearestNeighbour_FindTour(const Graph* g, unsigned int startVertex) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices == 0 || startVertex >= numVertices) return NULL;
    
    Tour* tour = TourCreate(numVertices);
    if (!tour) return NULL;

    int* visited = (int*) calloc(numVertices, sizeof(int));
    if (!visited) { TourDestroy(&tour); return NULL; }

    unsigned int currentV = startVertex;
    tour->path[0] = startVertex;
    visited[startVertex] = 1;
    tour->cost = 0.0;
    unsigned int pathIndex = 1;

    // build path
    while (pathIndex < numVertices) {
        double minWeight = DBL_MAX;
        unsigned int nextV = UINT_MAX;

        for (unsigned int v = 0; v < numVertices; v++) {
            // skip cur & visited vert.
            if (v == currentV || visited[v]) {
                continue;
            }

            double weight = GetEdgeWeight(g, currentV, v);

            // see if edge is new shortest connection
            if (weight < minWeight) { 
                minWeight = weight;
                nextV = v;
            }
        }

        if (nextV != UINT_MAX) {
            // found nearest unvisited neighbour
            tour->path[pathIndex++] = nextV;
            visited[nextV] = 1;
            tour->cost += minWeight;
            currentV = nextV; // move to new vertex
        } else {
            // only happens if graph is disconnected (should not happen)
            fprintf(stderr, "Error: Nearest Neighbour failed to find next unvisited vertex.\n");
            TourDestroy(&tour);
            free(visited);
            return NULL;
        }
    }

    // close tour 
    tour->path[pathIndex] = startVertex;
    
    double finalEdgeWeight = GetEdgeWeight(g, currentV, startVertex);
    
    // check if final edge is valid
    if (finalEdgeWeight == DBL_MAX) {
        fprintf(stderr, "Error: Nearest Neighbour failed to close the tour (final edge missing).\n");
        TourDestroy(&tour);
        free(visited);
        return NULL;
    }

    tour->cost += finalEdgeWeight;
    
    free(visited);
    return tour;
}