// GRASP.c - Greedy Randomized Adaptive Search Procedure for TSP.
//
// O(GRASP_ITERATIONS * (N^2 construction + 2-Opt per pass)).
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// GRASP is a multi-start metaheuristic. Each restart does two things:
//   1. Construction: build a tour greedily but with randomness -- at each step, instead of always taking the nearest unvisited city, pick uniformly from a Restricted Candidate List (RCL) of the cities within GRASP_ALPHA of the best (value-based RCL).
//      ALPHA = 0 is pure greedy (deterministic Nearest Neighbour), ALPHA = 1 is a random tour.
//      In between gives varied but still-good starting tours.
//   2. Local search: improve that tour to a 2-Opt local optimum.
//      The best tour over all restarts is returned.
//      The randomness in construction lets the many local searches land in different basins, so GRASP typically beats a single greedy-then-2-Opt run.
//
// The 2-Opt here is a quiet, self-contained copy of the neighbour-list 2-Opt (see TwoOpt.c): it is called once per restart, so it reuses ONE prebuilt NeighbourList and DistanceMatrix across all restarts instead of rebuilding them (and printing) every time, as the public TwoOpt_ImproveTour would.

// Resources used:
// https://en.wikipedia.org/wiki/Greedy_randomized_adaptive_search_procedure
// Feo & Resende, "Greedy Randomized Adaptive Search Procedures" (1995).

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/NeighbourList.h"
#include "../../headers/DistanceMatrix.h"
#include "../../headers/Metaheuristics.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// reverses path[lo..hi] (inclusive) and keeps the position index in sync (lo >= 1, so path[0] is fixed).
static void reverseSegment(unsigned int* path, unsigned int* pos, unsigned int lo, unsigned int hi) {
    while (lo < hi) {
        unsigned int a = path[lo], b = path[hi];
        path[lo] = b; path[hi] = a;
        pos[b] = lo; pos[a] = hi;
        lo++; hi--;
    }
}

// cost of the closed cyclic tour path[0..n-1] (edge n-1 -> 0 included). DBL_MAX if any edge is missing.
static double tourCost(const double* D, unsigned int n, const unsigned int* path) {
    double cost = 0.0;
    for (unsigned int i = 0; i < n; i++) {
        double w = DIST_AT(D, n, path[i], path[(i + 1) % n]);
        if (w == DBL_MAX) return DBL_MAX;
        cost += w;
    }
    return cost;
}

// One randomized-greedy nearest-neighbour construction into path[0..n-1]; visited and rcl are scratch.
static void constructRandomizedGreedy(const double* D, unsigned int n, unsigned int* path,
                                      int* visited, unsigned int* rcl) {
    for (unsigned int v = 0; v < n; v++) visited[v] = 0;

    unsigned int current = (unsigned int)(rand() % (int)n);
    path[0] = current;
    visited[current] = 1;

    for (unsigned int step = 1; step < n; step++) {
        // range of edge costs from `current` to the unvisited cities
        double cmin = DBL_MAX, cmax = -1.0;
        for (unsigned int v = 0; v < n; v++) {
            if (visited[v]) continue;
            double w = DIST_AT(D, n, current, v);
            if (w == DBL_MAX) continue;
            if (w < cmin) cmin = w;
            if (w > cmax) cmax = w;
        }

        unsigned int rclCount = 0;
        if (cmin == DBL_MAX) {
            // no reachable unvisited city (disconnected graph): take any unvisited, tour cost stays DBL_MAX
            for (unsigned int v = 0; v < n; v++) if (!visited[v]) rcl[rclCount++] = v;
        } else {
            double threshold = cmin + GRASP_ALPHA * (cmax - cmin);
            for (unsigned int v = 0; v < n; v++) {
                if (visited[v]) continue;
                double w = DIST_AT(D, n, current, v);
                if (w != DBL_MAX && w <= threshold) rcl[rclCount++] = v;
            }
        }

        unsigned int next = rcl[rand() % (int)rclCount]; // rclCount >= 1 always
        path[step] = next;
        visited[next] = 1;
        current = next;
    }
}

// Quiet neighbour-list 2-Opt (same neighbourhood as TwoOpt.c) using prebuilt nl + D, improving path in place.
// pos is scratch (pos[city] = index in path); *cost is kept in sync.
static void twoOptQuiet(const NeighbourList* nl, const double* D, unsigned int n,
                        unsigned int* path, unsigned int* pos, double* cost) {
    for (unsigned int i = 0; i < n; i++) pos[path[i]] = i;

    int improved = 1;
    while (improved) {
        improved = 0;
        for (unsigned int i = 0; i < n; i++) {
            unsigned int c1 = path[i];
            unsigned int c2 = path[(i + 1) % n];
            double dC1C2 = DIST_AT(D, n, c1, c2);
            if (dC1C2 == DBL_MAX) continue;

            for (int source = 0; source < 2; source++) {
                unsigned int anchor = (source == 0) ? c1 : c2;
                unsigned int count = NeighbourListCount(nl, anchor);
                for (unsigned int t = 0; t < count; t++) {
                    unsigned int cand = NeighbourListGet(nl, anchor, t);
                    double dAnchorCand = DIST_AT(D, n, anchor, cand);
                    if (dAnchorCand >= dC1C2) break; // sorted nearest-first: no gain beyond here

                    unsigned int c3 = (source == 0) ? cand : path[(pos[cand] + n - 1) % n];
                    unsigned int j = pos[c3];
                    unsigned int c4 = path[(j + 1) % n];
                    if (c3 == c1 || c3 == c2 || c4 == c1) continue;

                    double dC3C4 = DIST_AT(D, n, c3, c4);
                    double dC1C3 = DIST_AT(D, n, c1, c3);
                    double dC2C4 = DIST_AT(D, n, c2, c4);
                    if (dC3C4 == DBL_MAX || dC1C3 == DBL_MAX || dC2C4 == DBL_MAX) continue;

                    double gain = (dC1C2 + dC3C4) - (dC1C3 + dC2C4);
                    if (gain > DBL_EPSILON) {
                        unsigned int lo = (i < j ? i : j) + 1;
                        unsigned int hi = (i < j ? j : i);
                        reverseSegment(path, pos, lo, hi);
                        *cost -= gain;
                        improved = 1;
                        goto nextEdge;
                    }
                }
            }
            nextEdge:;
        }
    }
}

Tour* GRASP_FindTour(const Graph* g) {
    unsigned int n = GraphGetNumVertices(g);
    if (n < 2) return NULL;

    unsigned int k = n - 1 < LS_NEIGHBOURS ? n - 1 : LS_NEIGHBOURS;
    NeighbourList* nl = NeighbourListBuild(g, k);
    double* D = DistanceMatrixBuild(g);
    unsigned int* path = malloc(n * sizeof(unsigned int));
    unsigned int* pos = malloc(n * sizeof(unsigned int));
    unsigned int* rcl = malloc(n * sizeof(unsigned int));
    int* visited = malloc(n * sizeof(int));
    unsigned int* bestPath = malloc(n * sizeof(unsigned int));
    if (!nl || !D || !path || !pos || !rcl || !visited || !bestPath) {
        NeighbourListDestroy(&nl); free(D); free(path); free(pos); free(rcl); free(visited); free(bestPath);
        return NULL;
    }

    double bestCost = DBL_MAX;
    for (unsigned int iter = 0; iter < GRASP_ITERATIONS; iter++) {
        constructRandomizedGreedy(D, n, path, visited, rcl);
        double cost = tourCost(D, n, path);
        if (cost != DBL_MAX) twoOptQuiet(nl, D, n, path, pos, &cost);

        if (cost < bestCost) {
            bestCost = cost;
            for (unsigned int i = 0; i < n; i++) bestPath[i] = path[i];
        }
    }

    Tour* tour = NULL;
    if (bestCost != DBL_MAX) {
        tour = TourCreate(n);
        if (tour) {
            for (unsigned int i = 0; i < n; i++) tour->path[i] = bestPath[i];
            tour->path[n] = tour->path[0]; // close the cycle
            tour->cost = bestCost;
        }
    } else {
        fprintf(stderr, "Warning: GRASP found no valid tour (graph disconnected?).\n");
    }

    NeighbourListDestroy(&nl);
    free(D); free(path); free(pos); free(rcl); free(visited); free(bestPath);
    return tour;
}