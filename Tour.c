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
        if (i < t->numVertices - 1 && t->path[i+1] != UINT_MAX) {
            printf(" -> ");
        }
    }
    printf("\n");
}