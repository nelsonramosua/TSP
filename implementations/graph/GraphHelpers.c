// GraphHelpers.c (FINAL CORRECTED VERSION)
#include <stdio.h>
#include <float.h> // For DBL_MAX
#include <stdlib.h>
#include "Graph.h"
#include "SortedList.h" 

double GetEdgeWeight(const Graph* g, unsigned int v, unsigned int w) {
    if (v == w) return 0.0;  

    unsigned int* adj = GraphGetAdjacentsTo(g, v);
    double* dist = GraphGetDistancesToAdjacents(g, v); 

    unsigned int num_adj = (unsigned int)dist[0];
    double weight = DBL_MAX;

    for (unsigned int i = 1; i <= num_adj; i++) {
        if (adj[i] == w) {
            weight = dist[i]; 
            break;
        }
    }

    free(adj);
    free(dist);
    return weight;
}