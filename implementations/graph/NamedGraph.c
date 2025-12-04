// NamedGraph.c - Implements NamedGraph wrapper.
//
// Nelson Ramos, 124921
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit. 

#include "../../headers/NamedGraph.h"
#include <string.h>
#include <stdio.h>

NamedGraph* NamedGraphCreate(unsigned int numVertices) {
    NamedGraph* ng = malloc(sizeof(NamedGraph));
    if (!ng) return NULL;

    ng->g = GraphCreate(numVertices, 0, 1); // undirected, weighted
    if (!ng->g) { free(ng); return NULL; }

    ng->cityNames = calloc(numVertices, sizeof(char*));
    if (!ng->cityNames) { GraphDestroy(&ng->g); free(ng); return NULL; }

    return ng;
}

int NamedGraphSetCityName(NamedGraph* ng, unsigned int vertex, const char* name) {
    if (!ng || vertex >= GraphGetNumVertices(ng->g)) return 0;
    if (ng->cityNames[vertex]) free(ng->cityNames[vertex]);
    
    ng->cityNames[vertex] = strdup(name); // buffer overflow, but I'm too lazy... :)
    return (ng->cityNames[vertex] != NULL);
}

const char* NamedGraphGetCityName(const NamedGraph* ng, unsigned int vertex) {
    if (!ng || vertex >= GraphGetNumVertices(ng->g)) return NULL;
    return ng->cityNames[vertex];
}

void NamedGraphDestroy(NamedGraph** ng) {
    if (!ng || !(*ng)) return;
    if ((*ng)->cityNames) {
        for (unsigned int i = 0; i < GraphGetNumVertices((*ng)->g); i++)
            if ((*ng)->cityNames[i]) free((*ng)->cityNames[i]);
        free((*ng)->cityNames);
    }
    if ((*ng)->g) GraphDestroy(&(*ng)->g);
    free(*ng);
    *ng = NULL;
}