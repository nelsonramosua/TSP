// Prim_MST.c
#include "../../TravelingSalesmanProblem.h"
#include <stdlib.h>
#include <float.h>
#include <stdio.h>

// -----------------------------------------------------------
// INTERNAL: Shared Prim implementation (no duplication)
// -----------------------------------------------------------
static double Prim_Internal(
        const Graph* g,
        int* parentOut  // optional; may be NULL
) {
    unsigned int N = GraphGetNumVertices(g);
    if (N == 0) return 0.0;

    double* key = malloc(N * sizeof(double));
    int* inMST = malloc(N * sizeof(int));
    int* parent = NULL;

    if (!key || !inMST) return -1.0;

    if (parentOut)
        parent = parentOut;
    else {
        parent = malloc(N * sizeof(int));
        if (!parent) return -1.0;
    }

    // Init
    for (unsigned int i = 0; i < N; i++) {
        key[i] = DBL_MAX;
        inMST[i] = 0;
        parent[i] = -1;
    }

    key[0] = 0.0;

    double mstCost = 0.0;

    // Prim loop
    for (unsigned int step = 0; step < N; step++) {
        double minKey = DBL_MAX;
        int u = -1;

        for (unsigned int v = 0; v < N; v++)
            if (!inMST[v] && key[v] < minKey) {
                minKey = key[v];
                u = v;
            }

        if (u == -1) break;

        inMST[u] = 1;
        mstCost += (parent[u] == -1 ? 0.0 : key[u]);

        unsigned int* adj = GraphGetAdjacentsTo((Graph*)g, u);
        double* dist = GraphGetDistancesToAdjacents((Graph*)g, u);

        if (adj && dist) {
            unsigned int num = adj[0];
            for (unsigned int i = 1; i <= num; i++) {
                unsigned int v = adj[i];
                double w = dist[i];

                if (!inMST[v] && w < key[v]) {
                    key[v] = w;
                    parent[v] = u;
                }
            }
        }

        free(adj);
        free(dist);
    }

    free(key);
    free(inMST);

    // if parentOut wasn't supplied, free the temporary one
    if (!parentOut)
        free(parent);

    return mstCost;
}

// -----------------------------------------------------------
// PUBLIC FUNCTION #1 — Called by LowerBound_MST.c
// Computes MST cost only
// -----------------------------------------------------------
double Prim_CalculateMSTCost(const Graph* g) {
    if (!GraphIsWeighted(g) || GraphIsDigraph(g)) {
        fprintf(stderr, "Error: Prim's requires a weighted, undirected graph.\n");
        return -1.0;
    }
    return Prim_Internal(g, NULL);
}

// -----------------------------------------------------------
// PUBLIC FUNCTION #2 — Needed for Christofides
// Returns explicit list of MST edges
// -----------------------------------------------------------
typedef struct {
    unsigned int u;
    unsigned int v;
    double w;
} MSTEdge;

typedef struct {
    MSTEdge* edges;
    unsigned int count;
} MSTGraph;

Graph* MST_PrimGraph(const Graph* g) {
    unsigned int N = GraphGetNumVertices(g);
    if (N <= 1) return NULL;

    int* parent = malloc(N * sizeof(int));
    if (!parent) return NULL;

    Prim_Internal(g, parent);

    Graph* mst = GraphCreate(N, 0, 1);   // undirected, weighted

    // Add all MST edges
    for (unsigned int v = 1; v < N; v++) {
        if (parent[v] != -1) {
            double w = GetEdgeWeight(g, parent[v], v);
            GraphAddWeightedEdge(mst, parent[v], v, w);
        }
    }

    free(parent);
    return mst;
}