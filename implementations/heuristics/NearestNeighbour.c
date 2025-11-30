// NearestNeighbour.c
#include "../../TravelingSalesmanProblem.h"
#include <stdlib.h>
#include <float.h>
#include <string.h> // For memset

Tour* NearestNeighbour_FindTour(const Graph* g, unsigned int startVertex) {
    unsigned int N = GraphGetNumVertices(g);
    if (N == 0 || startVertex >= N) return NULL;
    
    Tour* tour = TourCreate(N);
    if (!tour) return NULL;

    int* visited = (int*) calloc(N, sizeof(int));
    if (!visited) { TourDestroy(&tour); return NULL; }

    unsigned int currentV = startVertex;
    tour->path[0] = startVertex;
    visited[startVertex] = 1;
    tour->cost = 0.0;
    unsigned int pathIndex = 1;

    // 1. Build the path
    while (pathIndex < N) {
        double minWeight = DBL_MAX;
        unsigned int nextV = UINT_MAX;

        unsigned int* adjacents = GraphGetAdjacentsTo((Graph*)g, currentV);
        double* distances = GraphGetDistancesToAdjacents((Graph*)g, currentV);

        unsigned int num_neighbors = adjacents[0];

        // printf("DEBUG: Current V=%u, Neighbors=%u\n", currentV, num_neighbors);
        if (adjacents && distances) {
            for (unsigned int i = 1; i <= num_neighbors; i++) {
                // printf("DEBUG: Neighbor %u: V=%u, Weight=%.2f, Visited=%d\n", i, adjacents[i], distances[i], visited[adjacents[i]]);

                unsigned int v = adjacents[i];
                double weight = distances[i];
                
                if (v < N && !visited[v] && weight < minWeight) { 
                    minWeight = weight;
                    nextV = v;
                }
            }
        }

        if (adjacents) free(adjacents);
        if (distances) free(distances);

        if (nextV != UINT_MAX) {
            tour->path[pathIndex++] = nextV;
            visited[nextV] = 1;
            tour->cost += minWeight;
            currentV = nextV;
        } else {
            // Should not happen for a complete graph
            fprintf(stderr, "Error: Nearest Neighbour failed to find next unvisited vertex.\n");
            TourDestroy(&tour);
            free(visited);
            return NULL;
        }
    }

    // 2. Close the tour (N -> startVertex)
    tour->path[pathIndex] = startVertex;
    // Find the weight of the closing edge
    // NOTE: Need an auxiliary function in Graph.c or rely on adjacents array structure
    unsigned int* finalAdj = GraphGetAdjacentsTo((Graph*)g, currentV);
    double* finalDist = GraphGetDistancesToAdjacents((Graph*)g, currentV);

    // FIX: Get the degree (casting away const) to control the loop safely
    unsigned int final_num_neighbors = finalAdj[0]; // <-- FIX HERE

    if (finalAdj && finalDist) {
        // Iterate safely using the degree
        for (unsigned int i = 1; i <= final_num_neighbors; i++) { // <-- FIX HERE
            if (finalAdj[i] == startVertex) {
                tour->cost += finalDist[i];
                break;
            }
        }
    }

    if (finalAdj) free(finalAdj);
    if (finalDist) free(finalDist);
    
    free(visited);
    return tour;
}