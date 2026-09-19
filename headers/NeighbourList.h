// NeighbourList.h - k-nearest-neighbour candidate lists for a graph.
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// For each vertex, precomputes its k nearest neighbours (by edge weight), sorted nearest-first.
// Local searches (Lin-Kernighan, and 2-Opt / Or-Opt if you extend them) use these "candidate lists" to only consider promising moves instead of all O(N) partners, which is the standard way to make them fast on larger instances.
// Built with the PriorityQueue ADT: for each vertex we push all reachable partners keyed by distance and pop the k smallest.

#ifndef _NEIGHBOUR_LIST_H_
#define _NEIGHBOUR_LIST_H_

#include "Graph.h"

typedef struct _NeighbourList {
    unsigned int numVertices;
    unsigned int k;             // requested neighbours per vertex
    unsigned int* neighbours;   // flattened [numVertices * k]; neighbours[v*k + i] = i-th nearest of v
    unsigned int* counts;       // counts[v] = valid neighbours stored for v (<= k; fewer if sparse)
} NeighbourList;

// Builds the lists (k is clamped to numVertices - 1). Returns NULL on failure.
NeighbourList* NeighbourListBuild(const Graph* g, unsigned int k);
void NeighbourListDestroy(NeighbourList** nl);

// Number of candidates stored for v, and the i-th nearest neighbour of v (0-based, i < count).
unsigned int NeighbourListCount(const NeighbourList* nl, unsigned int v);
unsigned int NeighbourListGet(const NeighbourList* nl, unsigned int v, unsigned int i);

#endif // _NEIGHBOUR_LIST_H_