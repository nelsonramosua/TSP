// DistanceMatrix.h - dense N*N edge-weight cache for a Graph.
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// GetEdgeWeight() is an O(degree) adjacency-list scan (O(N) on a dense graph), too slow to call in a tight inner loop.
// Algorithms that hammer edge weights (2-Opt, Or-Opt, Simulated Annealing, ...) build this flat N*N cache once for O(1) lookups. Index it with DIST_AT(D, n, a, b); the caller keeps N.
// (Held-Karp keeps its own 2-D matrix inside its DP table, so it does not use this.)

#ifndef _DISTANCE_MATRIX_H_
#define _DISTANCE_MATRIX_H_

#include "Graph.h"

// O(1) access into a matrix built by DistanceMatrixBuild for a graph of n vertices.
#define DIST_AT(D, n, a, b) ((D)[(size_t)(a) * (n) + (b)])

// Builds a flat numVertices*numVertices matrix of edge weights: DBL_MAX where there is no edge, 0 on the diagonal.
// Reads each vertex's adjacency row once (no per-entry allocation).
// Returns a malloc'd array of numVertices*numVertices doubles that the caller frees, or NULL on failure.
double* DistanceMatrixBuild(const Graph* g);

#endif // _DISTANCE_MATRIX_H_