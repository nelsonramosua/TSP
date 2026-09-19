// LinKernighan.c - Implements a Lin-Kernighan-style variable-depth local search for TSP.
//
// O(N^2) per improving pass (each anchor rotates the tour; chains use candidate lists).
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Lin-Kernighan generalises 2-Opt/3-Opt: from a base edge (t1, t2) it builds a *chain* of edge exchanges guided by the gain criterion (only add an edge (t2, t3) shorter than the one just broken), going deeper even when a single step does not yet improve, and finally keeps the best-improving prefix of the chain.
// Each step is realised as a 2-Opt reversal, so every prefix is a valid tour: we apply the chain move-by-move, remember the best cumulative gain, and undo back to it.
//Candidate lists (NeighbourList) restrict t3 to each vertex's nearest neighbours.
//
// To keep the array logic simple and free of wrap-around, the anchor t1 is rotated to index 0 before each search; forward chains then reverse tour[1..q-1] and never wrap, and t1 stays fixed at 0.
// Rotating every city to the front in turn covers every tour edge as a base edge, so this is a superset of candidate-restricted 2-Opt.
// We try all positive-gain first moves and keep the best chain, committing only strictly-improving ones -- so the cost is monotonically non-increasing and the tour stays valid.
// This is a compact depth-bounded LK, not the full Lin-Kernighan-Helsgaun.

// Resources used:
// https://en.wikipedia.org/wiki/Lin%E2%80%93Kernighan_heuristic
// "An Effective Heuristic Algorithm for the TSP", Lin & Kernighan (1973).

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/NeighbourList.h"
#include "../../headers/Metaheuristics.h"
#include "../../headers/Trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// reverses tour[a..b] (inclusive, a <= b) and keeps pos[] in sync.
static void reverseSegment(unsigned int* tour, unsigned int* pos, unsigned int a, unsigned int b) {
    while (a < b) {
        unsigned int ca = tour[a], cb = tour[b];
        tour[a] = cb; tour[b] = ca;
        pos[cb] = a; pos[ca] = b;
        a++; b--;
    }
}

// rotates the tour so that `city` sits at index 0 (cyclic left-rotate); rebuilds pos[].
static void rotateToFront(unsigned int* tour, unsigned int* pos, unsigned int numVertices,
                          unsigned int* buf, unsigned int city) {
    unsigned int s = pos[city];
    if (s == 0) return;
    for (unsigned int i = 0; i < numVertices; i++) buf[i] = tour[(s + i) % numVertices];
    for (unsigned int i = 0; i < numVertices; i++) { tour[i] = buf[i]; pos[buf[i]] = i; }
}

// Builds a chain anchored at t1 = tour[0] whose first step adds edge (tour[1], firstT3) and then extends greedily by the gain criterion.
// Each step reverses tour[1..q-1] (t3 at position q), so t1 stays fixed at 0 and every prefix is a valid tour.
// Records the best cumulative delta; if commit != 0 the tour is left at that best prefix, otherwise fully restored.
// Returns that delta.
static double lkChain(const Graph* g, unsigned int* tour, unsigned int* pos, const NeighbourList* neigh,
                      unsigned int numVertices, unsigned int firstT3, unsigned int* moves, int commit) {
    unsigned int t1 = tour[0];
    unsigned int applied = 0, bestApplied = 0;
    double curDelta = 0.0, bestDelta = 0.0;

    for (unsigned int depth = 0; depth < LK_MAX_DEPTH; depth++) {
        unsigned int t2 = tour[1];
        double dt1t2 = GetEdgeWeight(g, t1, t2);
        if (dt1t2 == DBL_MAX) break;

        // choose t3: forced first step, else nearest candidate of t2 satisfying the gain criterion
        unsigned int t3 = numVertices;
        if (depth == 0) {
            t3 = firstT3;
        } else {
            unsigned int cnt = NeighbourListCount(neigh, t2);
            for (unsigned int ci = 0; ci < cnt; ci++) {
                unsigned int cand = NeighbourListGet(neigh, t2, ci);
                if (GetEdgeWeight(g, t2, cand) >= dt1t2) break; // sorted; no gain beyond
                if (pos[cand] < 3) continue;                    // need a real segment (avoid no-ops)
                t3 = cand;
                break;
            }
        }
        if (t3 >= numVertices || pos[t3] < 3) break;

        unsigned int q = pos[t3];
        unsigned int t4 = tour[q - 1];                          // predecessor of t3; edge (t4, t3) breaks
        double dNew = GetEdgeWeight(g, t1, t4) + GetEdgeWeight(g, t2, t3);
        if (dNew == DBL_MAX) break;
        double delta = dNew - (dt1t2 + GetEdgeWeight(g, t4, t3));

        reverseSegment(tour, pos, 1, q - 1);
        moves[applied++] = q - 1;
        curDelta += delta;
        if (curDelta < bestDelta - LK_EPS) { bestDelta = curDelta; bestApplied = applied; }
    }

    unsigned int target = commit ? bestApplied : 0;
    for (unsigned int m = applied; m > target; m--)
        reverseSegment(tour, pos, 1, moves[m - 1]);

    return bestDelta;
}

Tour* LinKernighan_ImproveTour(const Graph* g, Tour* initialTour) {
    if (!initialTour || initialTour->numVertices <= 4) return initialTour;

    unsigned int numVertices = GraphGetNumVertices(g);
    unsigned int* tour = initialTour->path; // work on positions 0..numVertices-1

    unsigned int* pos = malloc(numVertices * sizeof(unsigned int));
    unsigned int* buf = malloc(numVertices * sizeof(unsigned int));
    unsigned int* moves = malloc(LK_MAX_DEPTH * sizeof(unsigned int));
    NeighbourList* neigh = NeighbourListBuild(g, LK_NEIGHBOURS);
    if (!pos || !buf || !moves || !neigh) {
        free(pos); free(buf); free(moves); NeighbourListDestroy(&neigh);
        return initialTour;
    }
    for (unsigned int i = 0; i < numVertices; i++) pos[tour[i]] = i;

    printf("  Starting Lin-Kernighan. Initial Cost: %.2f\n", initialTour->cost);
    TraceEmit(tour, numVertices, numVertices, initialTour->cost); // frame: seed tour

    int improved = 1;
    while (improved) {
        improved = 0;

        for (unsigned int t1 = 0; t1 < numVertices; t1++) {
            rotateToFront(tour, pos, numVertices, buf, t1); // now t1 == tour[0]
            unsigned int t2 = tour[1];
            double dt1t2 = GetEdgeWeight(g, t1, t2);
            if (dt1t2 == DBL_MAX) continue;

            // try every positive-gain first move (superset of candidate-2-Opt), keep the best chain
            double bestDelta = 0.0;
            unsigned int bestFirst = numVertices;
            unsigned int cnt = NeighbourListCount(neigh, t2);
            for (unsigned int ci = 0; ci < cnt; ci++) {
                unsigned int t3 = NeighbourListGet(neigh, t2, ci);
                if (GetEdgeWeight(g, t2, t3) >= dt1t2) break; // sorted; no gain beyond here
                if (pos[t3] < 3) continue;
                double d = lkChain(g, tour, pos, neigh, numVertices, t3, moves, 0);
                if (d < bestDelta) { bestDelta = d; bestFirst = t3; }
            }

            if (bestFirst != numVertices && bestDelta < -LK_EPS) {
                lkChain(g, tour, pos, neigh, numVertices, bestFirst, moves, 1);
                initialTour->cost += bestDelta;
                TraceEmit(tour, numVertices, numVertices, initialTour->cost); // frame: after improving chain
                improved = 1;
            }
        }
    }

    tour[numVertices] = tour[0]; // close cycle

    printf("  Final Cost after Lin-Kernighan: %.2f\n", initialTour->cost);

    free(pos); free(buf); free(moves); NeighbourListDestroy(&neigh);
    return initialTour;
}