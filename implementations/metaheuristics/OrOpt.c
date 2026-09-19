// OrOpt.c - Implements the Or-opt local search improvement for TSP (neighbour-list accelerated).
//
// O(N * k) work per pass with a k-nearest candidate list (k = LS_NEIGHBOURS), versus O(N^2) for the all-gaps scan.
// This is what makes it usable on large instances (e.g. TSPLIB A280).
// For N <= LS_NEIGHBOURS + 1 the candidate list spans the whole graph, so the neighbourhood is identical to full Or-opt.
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Or-opt complements 2-opt: instead of reversing a segment between two edges, it *relocates* a short chain of 1, 2 or 3 consecutive cities to a better place in the tour (optionally reversing the chain).
// 2-opt cannot express these moves, so running Or-opt after 2-opt usually squeezes out a bit more.
// Like 2-opt here, it only ever applies strictly-improving moves, so the cost is monotonically non-increasing and the tour stays valid.
// A relocated segment only pays off if one of its ends lands next to a near neighbour, so we only try inserting it beside the candidate-list neighbours of its two endpoints (C = predecessor of the insertion gap).
// Scanning every gap is unnecessary and is what made the old version O(N^2) per pass.

// Resources used:
// https://en.wikipedia.org/wiki/Local_search_(optimization)
// https://tsp-basics.blogspot.com/2017/03/or-opt.html

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/NeighbourList.h"
#include "../../headers/Metaheuristics.h"
#include "../../headers/DistanceMatrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#define OR_OPT_MAX_SEGMENT 3 // relocate chains of up to this many cities

// candidate relocation move, described relative to the current path
typedef struct {
    unsigned int segStart;   // start index of the segment in the current path
    unsigned int segLen;     // 1 .. OR_OPT_MAX_SEGMENT
    unsigned int gapCity;    // insert the segment right after this city of the remaining tour
    int reversed;            // 1 if the segment is inserted reversed
    double delta;            // cost change (negative = improvement)
} OrOptMove;

// builds the cyclic order of the tour with the segment [segStart, segStart+segLen) removed, starting just after the segment.
// rem must hold (numVertices - segLen) entries.
static void buildRemaining(const unsigned int* path, unsigned int numVertices,
                           unsigned int segStart, unsigned int segLen, unsigned int* rem) {
    unsigned int m = 0;
    unsigned int after = segStart + segLen; // first index past the segment (<= numVertices)
    for (unsigned int i = 0; i < numVertices - segLen; i++)
        rem[m++] = path[(after + i) % numVertices];
}

// considers inserting the segment after city C (nextC is the city that currently follows C in the remaining tour), both forward and reversed, and keeps whichever beats *best*.
static void considerGap(const double* dist, unsigned int n, unsigned int C, unsigned int nextC,
                        unsigned int segFirst, unsigned int segLast, double removalGain,
                        unsigned int p, unsigned int segLen, OrOptMove* best) {
    double wCD = DIST_AT(dist, n, C, nextC);
    if (wCD == DBL_MAX) return;

    double fwd = DIST_AT(dist, n, C, segFirst) + DIST_AT(dist, n, segLast, nextC) - wCD;
    double rev = DIST_AT(dist, n, C, segLast) + DIST_AT(dist, n, segFirst, nextC) - wCD;

    double fwdDelta = fwd - removalGain;
    double revDelta = rev - removalGain;

    if (fwdDelta < best->delta) *best = (OrOptMove){ p, segLen, C, 0, fwdDelta };
    if (revDelta < best->delta) *best = (OrOptMove){ p, segLen, C, 1, revDelta };
}

Tour* OrOpt_ImproveTour(const Graph* g, Tour* initialTour) {
    if (!initialTour || initialTour->numVertices <= 3) return initialTour;

    unsigned int numVertices = GraphGetNumVertices(g);
    unsigned int* path = initialTour->path; // cyclic: path[numVertices] == path[0]

    unsigned int k = numVertices - 1 < LS_NEIGHBOURS ? numVertices - 1 : LS_NEIGHBOURS;
    NeighbourList* nl = NeighbourListBuild(g, k);
    unsigned int* rem = malloc(numVertices * sizeof(unsigned int));
    unsigned int* rebuilt = malloc((numVertices + 1) * sizeof(unsigned int));
    unsigned int* pos = malloc(numVertices * sizeof(unsigned int)); // pos[city] = index in path
    double* D = DistanceMatrixBuild(g);
    if (!nl || !rem || !rebuilt || !pos || !D) {
        NeighbourListDestroy(&nl); free(rem); free(rebuilt); free(pos); free(D); return initialTour;
    }
    for (unsigned int i = 0; i < numVertices; i++) pos[path[i]] = i;

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

                double wA = DIST_AT(D, numVertices, A, segFirst);
                double wB = DIST_AT(D, numVertices, segLast, B);
                double wAB = DIST_AT(D, numVertices, A, B);
                if (wA == DBL_MAX || wB == DBL_MAX || wAB == DBL_MAX) continue;
                double removalGain = wA + wB - wAB; // cost freed by pulling the segment out

                // promising insertion points: beside a near neighbour of either segment endpoint.
                unsigned int ends[2] = { segFirst, segLast };
                for (int e = 0; e < 2; e++) {
                    unsigned int count = NeighbourListCount(nl, ends[e]);
                    for (unsigned int t = 0; t < count; t++) {
                        unsigned int C = NeighbourListGet(nl, ends[e], t);
                        unsigned int cp = pos[C];
                        if (cp >= p && cp < p + segLen) continue;    // C is inside the segment
                        // city following C in the *remaining* tour (only A's successor is inside the segment)
                        unsigned int nextC = (C == A) ? B : path[(cp + 1) % numVertices];
                        considerGap(D, numVertices, C, nextC, segFirst, segLast, removalGain, p, segLen, &best);
                    }
                }
            }
        }

        if (best.segLen > 0) {
            // apply the best improving move: rebuild path as rem[0..gap], segment, rem[gap+1..]
            unsigned int m = numVertices - best.segLen;
            buildRemaining(path, numVertices, best.segStart, best.segLen, rem);

            unsigned int gap = 0; // locate the chosen insertion city in the remaining order
            for (unsigned int q = 0; q < m; q++) if (rem[q] == best.gapCity) { gap = q; break; }

            unsigned int idx = 0;
            for (unsigned int q = 0; q <= gap; q++) rebuilt[idx++] = rem[q];
            if (!best.reversed)
                for (unsigned int s = 0; s < best.segLen; s++) rebuilt[idx++] = path[best.segStart + s];
            else
                for (unsigned int s = best.segLen; s > 0; s--) rebuilt[idx++] = path[best.segStart + s - 1];
            for (unsigned int q = gap + 1; q < m; q++) rebuilt[idx++] = rem[q];

            for (unsigned int i = 0; i < numVertices; i++) { path[i] = rebuilt[i]; pos[path[i]] = i; }
            path[numVertices] = path[0];
            initialTour->cost += best.delta;
            improved = 1;
        }
    }

    printf("  Final Cost after Or-Opt: %.2f\n", initialTour->cost);

    NeighbourListDestroy(&nl);
    free(rem); free(rebuilt); free(pos); free(D);
    return initialTour;
}