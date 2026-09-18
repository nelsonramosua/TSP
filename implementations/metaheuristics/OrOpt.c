// OrOpt.c - Implements the Or-opt local search improvement for TSP.
//
// O(N^3) (O(N^2) work per improving move, applied until no move improves).
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Or-opt complements 2-opt: instead of reversing a segment between two edges, it *relocates* a short chain of 1, 2 or 3 consecutive cities to a better place in the tour (optionally reversing the chain).
// 2-opt cannot express these moves, so running Or-opt after 2-opt usually squeezes out a bit more.
// Like 2-opt here, it only ever applies strictly-improving moves, so the cost is monotonically non-increasing and the tour stays valid.

// Resources used:
// https://en.wikipedia.org/wiki/Local_search_(optimization)
// https://tsp-basics.blogspot.com/2017/03/or-opt.html

#include "../../TravelingSalesmanProblem.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#define OR_OPT_MAX_SEGMENT 3 // relocate chains of up to this many cities

// candidate relocation move, described relative to the "remaining" tour (segment removed)
typedef struct {
    unsigned int segStart;   // start index of the segment in the current path
    unsigned int segLen;     // 1 .. OR_OPT_MAX_SEGMENT
    unsigned int gap;        // insert after this index of the remaining order
    int reversed;            // 1 if the segment is inserted reversed
    double delta;            // cost change (negative = improvement)
} OrOptMove;

// builds the cyclic order of the tour with the segment [segStart, segStart+segLen) removed, starting just after the segment; rem must hold (numVertices - segLen) entries.
static void buildRemaining(const unsigned int* path, unsigned int numVertices,
                           unsigned int segStart, unsigned int segLen, unsigned int* rem) {
    unsigned int m = 0;
    unsigned int after = segStart + segLen; // first index past the segment (<= numVertices)
    for (unsigned int i = 0; i < numVertices - segLen; i++)
        rem[m++] = path[(after + i) % numVertices];
}

Tour* OrOpt_ImproveTour(const Graph* g, Tour* initialTour) {
    if (!initialTour || initialTour->numVertices <= 3) return initialTour;

    unsigned int numVertices = GraphGetNumVertices(g);
    unsigned int* path = initialTour->path; // cyclic: path[numVertices] == path[0]

    unsigned int* rem = malloc(numVertices * sizeof(unsigned int));
    unsigned int* rebuilt = malloc((numVertices + 1) * sizeof(unsigned int));
    if (!rem || !rebuilt) { free(rem); free(rebuilt); return initialTour; }

    printf("  Starting Or-Opt. Initial Cost: %.2f\n", initialTour->cost);

    int improved = 1;
    while (improved) {
        improved = 0;
        OrOptMove best = { 0, 0, 0, 0, -1e-9 }; // only strict improvements beat this

        for (unsigned int segLen = 1; segLen <= OR_OPT_MAX_SEGMENT && segLen < numVertices - 1; segLen++) {
            for (unsigned int p = 0; p + segLen <= numVertices; p++) {
                unsigned int segFirst = path[p];
                unsigned int segLast = path[p + segLen - 1];
                unsigned int A = path[(p + numVertices - 1) % numVertices]; // before segment
                unsigned int B = path[(p + segLen) % numVertices];          // after segment

                double wA = GetEdgeWeight(g, A, segFirst);
                double wB = GetEdgeWeight(g, segLast, B);
                double wAB = GetEdgeWeight(g, A, B);
                if (wA == DBL_MAX || wB == DBL_MAX || wAB == DBL_MAX) continue;
                double removalGain = wA + wB - wAB; // cost freed by pulling the segment out

                unsigned int m = numVertices - segLen;
                buildRemaining(path, numVertices, p, segLen, rem);

                // try inserting the segment into every gap (C, D) of the remaining tour
                for (unsigned int q = 0; q < m; q++) {
                    unsigned int C = rem[q];
                    unsigned int D = rem[(q + 1) % m];
                    double wCD = GetEdgeWeight(g, C, D);
                    if (wCD == DBL_MAX) continue;

                    double fwd = GetEdgeWeight(g, C, segFirst) + GetEdgeWeight(g, segLast, D) - wCD;
                    double rev = GetEdgeWeight(g, C, segLast) + GetEdgeWeight(g, segFirst, D) - wCD;

                    double fwdDelta = fwd - removalGain;
                    double revDelta = rev - removalGain;

                    if (fwdDelta < best.delta) best = (OrOptMove){ p, segLen, q, 0, fwdDelta };
                    if (revDelta < best.delta) best = (OrOptMove){ p, segLen, q, 1, revDelta };
                }
            }
        }

        if (best.segLen > 0) {
            // apply the best improving move: rebuild path as rem[0..gap], segment, rem[gap+1..]
            unsigned int m = numVertices - best.segLen;
            buildRemaining(path, numVertices, best.segStart, best.segLen, rem);

            unsigned int idx = 0;
            for (unsigned int q = 0; q <= best.gap; q++) rebuilt[idx++] = rem[q];
            if (!best.reversed)
                for (unsigned int s = 0; s < best.segLen; s++) rebuilt[idx++] = path[best.segStart + s];
            else
                for (unsigned int s = best.segLen; s > 0; s--) rebuilt[idx++] = path[best.segStart + s - 1];
            for (unsigned int q = best.gap + 1; q < m; q++) rebuilt[idx++] = rem[q];

            for (unsigned int i = 0; i < numVertices; i++) path[i] = rebuilt[i];
            path[numVertices] = path[0];
            initialTour->cost += best.delta;
            improved = 1;
        }
    }

    printf("  Final Cost after Or-Opt: %.2f\n", initialTour->cost);

    free(rem); free(rebuilt);
    return initialTour;
}