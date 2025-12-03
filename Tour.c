// Tour.c - Implements basic ADT Tour for TSP, defined in TravelingSalesmanProblem.h.
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit.

#include "TravelingSalesmanProblem.h"
#include <stdlib.h>
#include <stdio.h>

Tour* TourCreate(unsigned int numVertices) {
    Tour* t = (Tour*) malloc(sizeof(Tour));
    if (!t) return NULL;

    // path arr size is numVertices + 1 (to close cycle)
    t->path = (unsigned int*) calloc(numVertices + 1, sizeof(unsigned int));
    if (!t->path) { free(t); return NULL; }
    
    // first element is size
    t->numVertices = numVertices + 1; 
    t->cost = 0.0;
    
    // init path elems to UINT_MAX
    for (unsigned int i = 0; i < t->numVertices; i++) t->path[i] = UINT_MAX;
    
    return t;
}

void TourDestroy(Tour** p) {
    Tour* t = *p;

    free(t->path);
    free(t);

    *p = NULL;
}

double TourGetCost(const Tour* t) {
    if (!t) return -1.0;
    return t->cost;
}

void TourDisplay(const Tour* t) {
    if (!t) {
        printf("  Tour is NULL.\n");
        return;
    }

    printf("  Cost: %.2f\n", t->cost);
    printf("  Path: ");
    for (unsigned int i = 0; i < t->numVertices; i++) {
        if (t->path[i] == UINT_MAX) break; 
        printf("%u", t->path[i]);
        if (i < t->numVertices - 1 && t->path[i+1] != UINT_MAX) printf(" -> ");
    }
    printf("\n");
}

// Helper function to check if the path is a valid permutation of vertices 0 to N-1
static int isPermutation(const unsigned int* path, unsigned int numVertices);

// Checks invariant properties of a Tour structure.
int TourInvariant(const Tour* t, unsigned int numVertices) {
    if (!t || !t->path) return 0; 

    if (t->numVertices != (numVertices + 1)) {
        fprintf(stderr, "[Invariant Error] Path array size (%u) should be N+1 (%u).\n", t->numVertices, numVertices + 1);
        return 0;
    }

    if (t->cost < 0.0) {
        fprintf(stderr, "[Invariant Error] Tour cost cannot be negative (%.2f).\n", t->cost);
        return 0;
    }
    
    if (!isPermutation(t->path, numVertices)) {
        fprintf(stderr, "[Invariant Error] Path is not a valid permutation of vertices 0 to %u.\n", numVertices - 1);
        return 0;
    }

    if (t->path[numVertices] != t->path[0]) {
        fprintf(stderr, "[Invariant Error] Cycle not closed: Path[%u] (%u) != Path[0] (%u).\n", numVertices, t->path[numVertices], t->path[0]);
        return 0;
    }

    return 1;
}

static int isPermutation(const unsigned int* path, unsigned int numVertices) {
    if (numVertices == 0) return 1;

    int* visited = (int*) calloc(numVertices, sizeof(int));
    if (!visited) {
        fprintf(stderr, "[Invariant Error] Memory allocation failed for permutation check.\n");
        return 0;
    }

    for (unsigned int i = 0; i < numVertices; i++) {
        unsigned int vertice = path[i];

        if (vertice >= numVertices) {
            fprintf(stderr, "[Invariant Detail] Vertice %u out of range [0, %u].\n", vertice, numVertices - 1);
            free(visited);
            return 0;
        }

        if (visited[vertice]) {
            fprintf(stderr, "[Invariant Detail] Duplicate vertice found: %u.\n", vertice);
            free(visited);
            return 0;
        }

        visited[vertice] = 1;
    }

    free(visited);

    return 1;
}