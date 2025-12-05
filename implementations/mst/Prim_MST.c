// Prim_MST.c - Implements Prim's algorithm to find the Minimum Spanning Tree (MST).
//
// O(N^2).
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// This could be improved for O(E * log N) if a priority queue / min-heap was used... For simplicty purposes, it wasn't. Try!

#include "../../TravelingSalesmanProblem.h"

#include <stdlib.h>
#include <float.h>
#include <stdio.h>

// Prim helper
static double Prim_Internal(const Graph* g, int* parentOut /* may be NULL */);
double Prim_CalculateMSTCost(const Graph* g);

Graph* MST_PrimGraph(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    // MST is only useful for numVertices >= 2
    if (numVertices <= 1) return NULL;

    int* parent = malloc(numVertices * sizeof(int));
    if (!parent) return NULL;

    // run Prim algorithm
    Prim_Internal(g, parent);

    Graph* mst = GraphCreate(numVertices, 0, 1);

    if (!mst) { free(parent); return NULL; }

    // add all MST edges
    for (unsigned int v = 1; v < numVertices; v++) {
        if (parent[v] != -1) {
            // only add edges that were part of MST
            double w = GetEdgeWeight(g, parent[v], v);
            GraphAddWeightedEdge(mst, parent[v], v, w);
        }
    }

    free(parent);
    return mst;
}

// Prim helper
static double Prim_Internal(const Graph* g, int* parentOut /* may be NULL */) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices == 0) return 0.0;

    // key[v] stores weight of cheapest edge connecting v to MST
    double* key = malloc(numVertices * sizeof(double));
    // inMST[v] true (1) if v is in MST already
    int* inMST = malloc(numVertices * sizeof(int));
    int* parent = NULL;

    if (!key || !inMST) {
        if (key) free(key);
        if (inMST) free(inMST);
        return -1.0;
    }

    // determine where to save parent arr
    if (parentOut) parent = parentOut;
    else {
        parent = malloc(numVertices * sizeof(int));
        if (!parent) { free(key); free(inMST); return -1.0; }
    }

    // Init
    for (unsigned int i = 0; i < numVertices; i++) {
        key[i] = DBL_MAX;
        inMST[i] = 0;
        parent[i] = -1; // root or unreached vert.
    }

    // start at vert 0 (arbitrary). cost is 0 at first too
    key[0] = 0.0;
    double mstCost = 0.0;

    // prim loop
    for (unsigned int step = 0; step < numVertices; step++) {
        double minKey = DBL_MAX;
        int u = -1;

        // find next vertex u to add to MST (chooses cheapest incoming edge)
        for (unsigned int v = 0; v < numVertices; v++)
            if (!inMST[v] && key[v] < minKey) { minKey = key[v]; u = v; }

        if (u == -1) break;

        // add vert. to MST
        inMST[u] = 1;
        // add weight of edge connecting parent[u] to u
        mstCost += (parent[u] == -1 ? 0.0 : key[u]);

        unsigned int* adj = GraphGetAdjacentsTo(g, u);
        double* dist = GraphGetDistancesToAdjacents(g, u);

        if (adj && dist) {
            unsigned int num = adj[0];
            for (unsigned int i = 1; i <= num; i++) {
                unsigned int v = adj[i];
                double w = dist[i];

                // if V not in MST and edge (U, V) cheaper than V's current key
                if (!inMST[v] && w < key[v]) {
                    key[v] = w; // update V's min connection cost
                    parent[v] = u; // set U as V's new connection parent
                }
            }
        }

        free(adj); free(dist);
    }

    free(key); free(inMST);

    if (!parentOut) free(parent);

    return mstCost;
}

// calcs MST cost
double Prim_CalculateMSTCost(const Graph* g) {
    // prim's requires a weighted, undirected graph
    if (!GraphIsWeighted(g) || GraphIsDigraph(g)) {
        fprintf(stderr, "Error: Prim's requires a weighted, undirected graph.\n");
        return -1.0;
    }
    return Prim_Internal(g, NULL);
}