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

    ng->nameToIndexMap = HashMapCreate(); 
    if (!ng->nameToIndexMap) { free(ng->cityNames); GraphDestroy(&ng->g); free(ng); return NULL; }

    ng->numNamedVertices = 0;

    return ng;
}

void NamedGraphDestroy(NamedGraph** ng) {
    if (!ng || !(*ng)) return;

    NamedGraph* namedGraph = *ng;

    if (namedGraph->nameToIndexMap) HashMapDestroy(&namedGraph->nameToIndexMap);

    if (namedGraph->cityNames) {
        for (unsigned int i = 0; i < GraphGetNumVertices(namedGraph->g); i++)
            if (namedGraph->cityNames[i]) free(namedGraph->cityNames[i]);
        free(namedGraph->cityNames);
    }

    if (namedGraph->g) GraphDestroy(&namedGraph->g);
    free(namedGraph);

    *ng = NULL;
}

int NamedGraphSetCityName(NamedGraph* ng, unsigned int vertex, const char* name) {
    if (!ng || vertex >= GraphGetNumVertices(ng->g)) return 0;
    
    // is there a name already (non-null)
    if (ng->cityNames[vertex]) {
        // if name is being altered, remove old mapping.
        HashMapRemove(ng->nameToIndexMap, ng->cityNames[vertex]); 
        free(ng->cityNames[vertex]);
        ng->cityNames[vertex] = NULL;
    }
    
    // alloc and attribute new name
    if (name) {
        ng->cityNames[vertex] = strdup(name); // buffer overflow... but i'm too lazy. :)
        if (ng->cityNames[vertex] == NULL) return 0; 
        // add/update new mapping in hashmap.
        if (HashMapPut(ng->nameToIndexMap, ng->cityNames[vertex], (int)vertex) == 0) {
            free(ng->cityNames[vertex]);
            ng->cityNames[vertex] = NULL;
            return 0;
        }
        return 1;
    } else { ng->cityNames[vertex] = NULL; return 1; } // set to null
}

const char* NamedGraphGetCityName(const NamedGraph* ng, unsigned int vertex) {
    if (!ng || vertex >= GraphGetNumVertices(ng->g)) return NULL;
    return ng->cityNames[vertex];
}

int NamedGraphGetOrAssignIndex(NamedGraph* namedGraph, const char* name) {
    if (!namedGraph || !name) return -1;
    
    int index = HashMapGet(namedGraph->nameToIndexMap, name);
    if (index != -1) return index; // found
    
    unsigned int newIndex = namedGraph->numNamedVertices;
    if (newIndex >= GraphGetNumVertices(namedGraph->g)) {
        fprintf(stderr, "Error: Cannot assign name '%s'. Max vertices (%u) reached.\n", name, GraphGetNumVertices(namedGraph->g)); return -1;
    }

    if (NamedGraphSetCityName(namedGraph, newIndex, name) == 0) return -1; 

    namedGraph->numNamedVertices++; 
    
    return (int)newIndex;
}