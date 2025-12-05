// Tour.c - Implements basic ADT Tour for TSP, defined in TravelingSalesmanProblem.h.
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

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

void TourDestroy(Tour** p) {
    if (!p || !*p) return;
    Tour* t = *p;

    free(t->path); t->path = NULL;

    if (t->cityNames) {
        for (int i = 0; i < (int)t->numVertices; i++) if (t->cityNames[i]) { free(t->cityNames[i]); t->cityNames[i] = NULL; }
        free(t->cityNames); t->cityNames = NULL;
    }

    free(t); *p = NULL;
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
    
    // deep copy city names
    if (src->cityNames) {
        copy->cityNames = malloc(copy->numVertices * sizeof(char*));
        if (!copy->cityNames) { free(copy->path); free(copy); return NULL; }
        for (unsigned int i = 0; i < copy->numVertices; i++) copy->cityNames[i] = src->cityNames[i] ? strdup(src->cityNames[i]) : NULL;
    } else copy->cityNames = NULL;

    return copy;
}

double TourGetCost(const Tour* t) {
    return t ? t->cost : -1.0;
}

void TourMapCityNames(Tour* tour, const NamedGraph* ng) {
    if (!tour || !ng) return;

    // free previous city names if any
    if (tour->cityNames) {
        for (unsigned int i = 0; i < tour->numVertices; i++) 
            if (tour->cityNames[i]) { free(tour->cityNames[i]); tour->cityNames[i] = NULL; }
        free(tour->cityNames); tour->cityNames = NULL;
    }

    tour->cityNames = malloc(tour->numVertices * sizeof(char*));
    if (!tour->cityNames) return;

    // finally... actually map (with safeguards). I legit lost like 2h trying to fix a memory bug here.
    for (unsigned int i = 0; i < tour->numVertices; i++) {
        unsigned int index = tour->path[i];
        if (index == UINT_MAX || index >= tour->numVertices || !ng->cityNames[index]) {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%u", index);
            tour->cityNames[i] = strdup(buffer);
        } else tour->cityNames[i] = strdup(ng->cityNames[index]);
    }
}

void TourDisplay(const Tour* t) {
    if (!t) { printf("  Tour is NULL.\n"); return; }

    printf("  Cost: %.2f\n", t->cost);
    printf("  Path: ");
    for (unsigned int i = 0; i < t->numVertices; i++) {
        if (t->path[i] == UINT_MAX) break; 

        if (t->cityNames) printf("%s", t->cityNames[t->path[i]]); // if city names are available, print them
        else printf("%u", t->path[i]);                            // else print vertex indices // this is actually redundant (see mapping above)

        if (i < t->numVertices - 1 && t->path[i+1] != UINT_MAX) printf(" -> ");
    }
    printf("\n");
}

// helper function to check if the path is a valid permutation of vertices 0 to N-1
static int isPermutation(const unsigned int* path, unsigned int numVertices);

// checks invariant properties of a Tour structure.
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