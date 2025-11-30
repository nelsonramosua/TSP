// Tour.c
#include "TravelingSalesmanProblem.h"
#include <stdlib.h>
#include <stdio.h>

Tour* TourCreate(unsigned int numVertices) {
    Tour* t = (Tour*) malloc(sizeof(Tour));
    if (!t) return NULL;

    // Path array size is numVertices + 1 to close the cycle
    t->path = (unsigned int*) calloc(numVertices + 1, sizeof(unsigned int));
    if (!t->path) { free(t); return NULL; }
    
    // Use the first element for size, assuming full path is N+1 long.
    t->numVertices = numVertices + 1; 
    t->cost = 0.0;
    
    // Initialize path elements to UINT_MAX as a sentinel for uninitialized
    for (unsigned int i = 0; i < t->numVertices; i++) {
        t->path[i] = UINT_MAX;
    }

    return t;
}

void TourDestroy(Tour** p) {
    if (p && *p) {
        free((*p)->path);
        free(*p);
        *p = NULL;
    }
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