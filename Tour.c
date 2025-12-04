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
    t->cityNames = NULL;
    
    // init path elems to UINT_MAX
    for (unsigned int i = 0; i < t->numVertices; i++) t->path[i] = UINT_MAX;
    
    return t;
}

Tour* TourDeepCopy(const Tour* src) {
    if (!src) return NULL;

    Tour* copy = (Tour*) malloc(sizeof(Tour));
    if (!copy) return NULL;

    copy->numVertices = src->numVertices;
    copy->cost = src->cost;

    // deep copy path array
    copy->path = (unsigned int*) malloc(copy->numVertices * sizeof(unsigned int));
    if (!copy->path) { free(copy); return NULL; }
    for (unsigned int i = 0; i < copy->numVertices; i++) copy->path[i] = src->path[i];
    
    // copy city names ptr 
    copy->cityNames = src->cityNames;

    return copy;
}

void TourDestroy(Tour** p) {
    Tour* t = *p;

    free(t->path);
    t->path = NULL;
    t->cityNames = NULL;

    free(t);
    *p = NULL;
}

double TourGetCost(const Tour* t) {
    return t ? t->cost : -1.0;
}

void TourMapCityNames(Tour* tour, const NamedGraph* ng) {
    if (!tour || !ng) return;

    unsigned int numVertices = GraphGetNumVertices(ng->g);

    tour->cityNames = malloc(numVertices * sizeof(const char*));
    if (!tour->cityNames) return;

    // temporary buffer to hold default names if needed
    static char defaultNames[512][16]; // max 512 vertices, name length 15

    for (unsigned int i = 0; i < numVertices; i++) {
        if (ng->cityNames && ng->cityNames[i]) {
            tour->cityNames[i] = ng->cityNames[i];  // use user-defined name
        } else {
            snprintf(defaultNames[i], sizeof(defaultNames[i]), "%u", i); // default: vertex index
            tour->cityNames[i] = defaultNames[i];
        }
    }
}

void TourDisplay(const Tour* t) {
    if (!t) { printf("  Tour is NULL.\n"); return; }

    printf("  Cost: %.2f\n", t->cost);
    printf("  Path: ");
    for (unsigned int i = 0; i < t->numVertices; i++) {
        if (t->path[i] == UINT_MAX) break; 

        if (t->cityNames) printf("%s", t->cityNames[t->path[i]]); // if city names are available, print them
        else printf("%u", t->path[i]);                            // else print vertex indices

        if (i < t->numVertices - 1 && t->path[i+1] != UINT_MAX) printf(" -> ");
    }
    printf("\n");
}

// Helper function to check if the path is a valid permutation of vertices 0 to N-1
static int isPermutation(const unsigned int* path, unsigned int numVertices);

// Checks invariant properties of a Tour structure.
int TourInvariant(const Tour* t, unsigned int numVertices) {
    if (!t || !t->path) return 0;
    if (t->numVertices != numVertices + 1) return 0;
    if (t->cost < 0.0) return 0;
    if (!isPermutation(t->path, numVertices)) return 0;
    if (t->path[numVertices] != t->path[0]) return 0;
    return 1;
}
    
static int isPermutation(const unsigned int* path, unsigned int numVertices) {
    int* visited = calloc(numVertices, sizeof(int));
    if (!visited) return 0;

    for (unsigned int i = 0; i < numVertices; i++) {
        if (path[i] >= numVertices || visited[path[i]]) { free(visited); return 0; }
        visited[path[i]] = 1;
    }

    free(visited);
    return 1;
}