// NamedGraph.h - Graph wrapper with city names
//
// Nelson Ramos, 124921
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

#ifndef _NAMED_GRAPH_H_
#define _NAMED_GRAPH_H_

#include <stdlib.h>
#include "HashMap.h"
#include "Graph.h"

typedef struct _NamedGraph {
    Graph* g;
    char** cityNames;                   // size = GraphGetNumVertices(g)

    HashMap* nameToIndexMap;            // mapping: name -> index 
    unsigned int numNamedVertices;      // namedCities counter
} NamedGraph;

NamedGraph* NamedGraphCreate(unsigned int numVertices);
void NamedGraphDestroy(NamedGraph** namedGraph);
int NamedGraphSetCityName(NamedGraph* namedGraph, unsigned int vertex, const char* name);
const char* NamedGraphGetCityName(const NamedGraph* namedGraph, unsigned int vertex);
int NamedGraphGetOrAssignIndex(NamedGraph* namedGraph, const char* name);

#endif // _NAMED_GRAPH_H_