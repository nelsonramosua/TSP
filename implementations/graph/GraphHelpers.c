// GraphHelpers.c  - helpers for the project related to graphs.
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit.

#include <stdio.h>
#include <float.h> 
#include <stdlib.h>

#include "Graph.h"
#include "SortedList.h" 

double GetEdgeWeight(const Graph* g, unsigned int v, unsigned int w) {
    if (v == w) return 0.0; // should not happen

    unsigned int* adj = GraphGetAdjacentsTo(g, v);
    double* dist = GraphGetDistancesToAdjacents(g, v); 

    unsigned int num_adj = (unsigned int)dist[0]; // 1st element of arr is number of adj vertices
    double weight = DBL_MAX; // preset (vertex dne)

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