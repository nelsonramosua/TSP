// NeighbourList.c - k-nearest-neighbour candidate lists for a graph.
//
// O(N^2 * log N) to build (per vertex: push O(N) into a heap, pop k).
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// See NeighbourList.h: this is the concrete "candidate list" builder used by Lin-Kernighan.

#include "../../headers/NeighbourList.h"
#include "../../headers/PriorityQueue.h"
#include "../../TravelingSalesmanProblem.h"

#include <stdlib.h>
#include <float.h>

NeighbourList* NeighbourListBuild(const Graph* g, unsigned int k) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices < 2) return NULL;
    if (k > numVertices - 1) k = numVertices - 1; // cannot have more neighbours than that

    NeighbourList* nl = malloc(sizeof(NeighbourList));
    if (!nl) return NULL;
    nl->numVertices = numVertices;
    nl->k = k;
    nl->neighbours = malloc((size_t)numVertices * k * sizeof(unsigned int));
    nl->counts = calloc(numVertices, sizeof(unsigned int));
    if (!nl->neighbours || !nl->counts) {
        free(nl->neighbours); free(nl->counts); free(nl);
        return NULL;
    }

    PriorityQueue* pq = PQCreate(numVertices);
    if (!pq) { NeighbourListDestroy(&nl); return NULL; }

    for (unsigned int v = 0; v < numVertices; v++) {
        // push every reachable partner keyed by distance, then pop the k nearest
        for (unsigned int u = 0; u < numVertices; u++) {
            if (u == v) continue;
            double w = GetEdgeWeight(g, v, u);
            if (w == DBL_MAX) continue; // unreachable: not a candidate
            PQInsert(pq, u, w);
        }

        unsigned int stored = 0;
        while (stored < k && !PQIsEmpty(pq)) {
            unsigned int u = PQExtractMin(pq);
            nl->neighbours[(size_t)v * k + stored] = u;
            stored++;
        }
        nl->counts[v] = stored;

        // drain any leftovers so the queue is empty for the next vertex
        while (!PQIsEmpty(pq)) PQExtractMin(pq);
    }

    PQDestroy(&pq);
    return nl;
}

void NeighbourListDestroy(NeighbourList** nl) {
    if (!nl || !*nl) return;
    free((*nl)->neighbours);
    free((*nl)->counts);
    free(*nl);
    *nl = NULL;
}

unsigned int NeighbourListCount(const NeighbourList* nl, unsigned int v) {
    return (nl && v < nl->numVertices) ? nl->counts[v] : 0;
}

unsigned int NeighbourListGet(const NeighbourList* nl, unsigned int v, unsigned int i) {
    return nl->neighbours[(size_t)v * nl->k + i];
}