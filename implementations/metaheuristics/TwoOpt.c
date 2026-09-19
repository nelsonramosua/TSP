// TwoOpt.c - Implements the 2-Opt local search algorithm (neighbour-list accelerated).
//
// O(N * k) work per pass with a k-nearest candidate list (k = LS_NEIGHBOURS), versus O(N^2) for the textbook all-pairs scan.
// This is what makes it usable on large instances (e.g. TSPLIB A280).
// For N <= LS_NEIGHBOURS + 1 the candidate list spans the whole graph, so the neighbourhood is identical to full 2-Opt.
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// A 2-Opt move removes two tour edges (c1,c2) and (c3,c4) and reconnects them as c1,c3) and (c2,c4), which reverses the path segment between them.
// An improving move must shorten at least one of its two removed edges, so it is enough, for each tour edge (c1,c2), to look only at the near neighbours of c1 (candidate new edge (c1,c3)) and of c2 (candidate new edge (c2,c4)).
// The sorted candidate list lets us stop as soon as a candidate is farther than the edge we are trying to beat.
// Scanning every tour edge from both endpoints catches every improving move.

// Resources used:
// https://en.wikipedia.org/wiki/2-opt
// J. L. Bentley, "Fast Algorithms for Geometric Traveling Salesman Problems" (candidate lists).

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/NeighbourList.h"
#include "../../headers/Metaheuristics.h"
#include "../../headers/DistanceMatrix.h"
#include "../../headers/Trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// reverses path[lo..hi] (inclusive) and keeps the position index in sync.
static void reverseSegment(unsigned int* path, unsigned int* pos, unsigned int lo, unsigned int hi);

// tries to improve existing TSP tour using neighbour-list 2-Opt local search.
Tour* TwoOpt_ImproveTour(const Graph* g, Tour* initialTour) {
    if (!initialTour || initialTour->numVertices <= 2) return initialTour;

    unsigned int numVertices = GraphGetNumVertices(g);
    unsigned int* path = initialTour->path; // cyclic: path[numVertices] == path[0]

    // candidate lists (k clamped inside NeighbourListBuild to numVertices - 1) and a position index pos[city] = index of city in path[].
    unsigned int k = numVertices - 1 < LS_NEIGHBOURS ? numVertices - 1 : LS_NEIGHBOURS;
    NeighbourList* nl = NeighbourListBuild(g, k);
    unsigned int* pos = malloc(numVertices * sizeof(unsigned int));
    double* D = DistanceMatrixBuild(g);
    if (!nl || !pos || !D) { NeighbourListDestroy(&nl); free(pos); free(D); return initialTour; }
    for (unsigned int i = 0; i < numVertices; i++) pos[path[i]] = i;

    printf("  Starting 2-Opt. Initial Cost: %.2f\n", initialTour->cost);
    TraceEmit(path, numVertices, numVertices, initialTour->cost); // frame: seed tour (no-op unless tracing)

    int improved = 1;
    while (improved) {
        improved = 0;

        for (unsigned int i = 0; i < numVertices; i++) {
            unsigned int c1 = path[i];
            unsigned int c2 = path[(i + 1) % numVertices]; // successor of c1 in the tour
            double dC1C2 = DIST_AT(D, numVertices, c1, c2);
            if (dC1C2 == DBL_MAX) continue;

            // two candidate sources for the new short edge: near neighbours of c1 (new edge (c1,c3)) and near neighbours of c2 (new edge (c2,c4), i.e. c3 = predecessor of c4).
            for (int source = 0; source < 2; source++) {
                unsigned int anchor = (source == 0) ? c1 : c2;
                unsigned int count = NeighbourListCount(nl, anchor);

                for (unsigned int t = 0; t < count; t++) {
                    unsigned int cand = NeighbourListGet(nl, anchor, t);
                    double dAnchorCand = DIST_AT(D, numVertices, anchor, cand);
                    if (dAnchorCand >= dC1C2) break; // sorted nearest-first: no gain possible beyond here

                    // resolve the move so its new short edge is (anchor, cand):
                    // source 0 -> c3 = cand;                 source 1 -> c4 = cand, c3 = predecessor(c4)
                    unsigned int c3 = (source == 0) ? cand : path[(pos[cand] + numVertices - 1) % numVertices];
                    unsigned int j = pos[c3];
                    unsigned int c4 = path[(j + 1) % numVertices]; // successor of c3

                    if (c3 == c1 || c3 == c2 || c4 == c1) continue; // need two distinct, non-adjacent edges

                    double dC3C4 = DIST_AT(D, numVertices, c3, c4);
                    double dC1C3 = DIST_AT(D, numVertices, c1, c3);
                    double dC2C4 = DIST_AT(D, numVertices, c2, c4);
                    if (dC3C4 == DBL_MAX || dC1C3 == DBL_MAX || dC2C4 == DBL_MAX) continue;

                    double gain = (dC1C2 + dC3C4) - (dC1C3 + dC2C4);
                    if (gain > DBL_EPSILON) {
                        // reverse the segment between the two edges (never wraps: lo >= 1)
                        unsigned int lo = (i < j ? i : j) + 1;
                        unsigned int hi = (i < j ? j : i);
                        reverseSegment(path, pos, lo, hi);
                        initialTour->cost -= gain;
                        TraceEmit(path, numVertices, numVertices, initialTour->cost); // frame: after move
                        improved = 1;
                        goto nextEdge; // c1/c2 changed; move on to the next tour edge
                    }
                }
            }
            nextEdge:;
        }
    }

    printf("  Final Cost after 2-Opt: %.2f\n", initialTour->cost);

    NeighbourListDestroy(&nl);
    free(pos);
    free(D);
    return initialTour;
}

// reverses path[lo..hi] (inclusive) and keeps the position index in sync. lo >= 1 here, so path[0] never moves and the cyclic sentinel path[numVertices] == path[0] stays valid.
static void reverseSegment(unsigned int* path, unsigned int* pos, unsigned int lo, unsigned int hi) {
    while (lo < hi) {
        unsigned int a = path[lo], b = path[hi];
        path[lo] = b; path[hi] = a;
        pos[b] = lo; pos[a] = hi;
        lo++; hi--;
    }
}