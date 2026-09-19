// DistanceMatrix.c - dense N*N edge-weight cache for a Graph.
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

#include "../../headers/DistanceMatrix.h"

#include <stdlib.h>
#include <float.h>

double* DistanceMatrixBuild(const Graph* g) {
    unsigned int n = GraphGetNumVertices(g);
    double* D = malloc((size_t)n * n * sizeof(double));
    if (!D) return NULL;

    for (size_t i = 0; i < (size_t)n * n; i++) D[i] = DBL_MAX;

    // one accessor call per vertex fills its whole row -- no GetEdgeWeight() call per entry
    for (unsigned int u = 0; u < n; u++) {
        unsigned int* adj = GraphGetAdjacentsTo(g, u);
        double* dst = GraphGetDistancesToAdjacents(g, u);
        unsigned int num = (unsigned int)dst[0];
        for (unsigned int i = 1; i <= num; i++) DIST_AT(D, n, u, adj[i]) = dst[i];
        DIST_AT(D, n, u, u) = 0.0;
        free(adj); free(dst);
    }

    return D;
}