// ThreeOpt.c - Implements the 3-Opt local search improvement for TSP.
//
// O(N^3) per pass (three nested position loops), applied until no move improves.
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Generalises 2-Opt: it removes three edges instead of two and reconnects the tour in one of the 7 non-trivial ways.
// Three of those are just 2-Opt moves and the rest genuinely need three cuts, so 3-Opt strictly dominates 2-Opt's neighbourhood (at higher cost).
//
// A move only touches the three cut edges (segment reversals keep each segment's internal cost the same on a symmetric graph), so the gain is computed from six endpoints in O(1).
// We only apply strictly-improving moves, so the cost is monotonically non-increasing and the tour stays a valid permutation.

// Resources used:
// https://en.wikipedia.org/wiki/3-opt
// https://tsp-basics.blogspot.com/2017/03/3-opt.html

#include "../../TravelingSalesmanProblem.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// appends path[from..to] (inclusive) to dst forward if step reveals order; helper below rebuilds the tour after choosing reconnection 'kase' for cut positions i < j < k.
static void applyReconnection(unsigned int* path, unsigned int* temp, unsigned int numVertices,
                              unsigned int i, unsigned int j, unsigned int k, int kase);

Tour* ThreeOpt_ImproveTour(const Graph* g, Tour* initialTour) {
    if (!initialTour || initialTour->numVertices <= 4) return initialTour; // needs numVertices >= 4

    unsigned int numVertices = GraphGetNumVertices(g);
    unsigned int* path = initialTour->path; // cyclic: path[numVertices] == path[0]

    unsigned int* temp = malloc((numVertices + 1) * sizeof(unsigned int));
    if (!temp) return initialTour;

    printf("  Starting 3-Opt. Initial Cost: %.2f\n", initialTour->cost);

    int improved = 1;
    while (improved) {
        improved = 0;

        for (unsigned int i = 1; i < numVertices - 1; i++) {
            for (unsigned int j = i + 1; j < numVertices; j++) {
                for (unsigned int k = j + 1; k <= numVertices; k++) {
                    // six endpoints of the three removed edges (A,B), (C,D), (E,F)
                    unsigned int A = path[i - 1], B = path[i];
                    unsigned int C = path[j - 1], D = path[j];
                    unsigned int E = path[k - 1], F = path[k % numVertices];

                    double dAB = GetEdgeWeight(g, A, B);
                    double dCD = GetEdgeWeight(g, C, D);
                    double dEF = GetEdgeWeight(g, E, F);
                    if (dAB == DBL_MAX || dCD == DBL_MAX || dEF == DBL_MAX) continue;
                    double removed = dAB + dCD + dEF;

                    // the 7 non-identity reconnections; delta = new boundary edges - removed
                    double cand[9];
                    cand[2] = GetEdgeWeight(g, A, C) + GetEdgeWeight(g, B, D) + dEF;                          // rev seg1
                    cand[3] = dAB + GetEdgeWeight(g, C, E) + GetEdgeWeight(g, D, F);                          // rev seg2
                    cand[4] = GetEdgeWeight(g, A, C) + GetEdgeWeight(g, B, E) + GetEdgeWeight(g, D, F);       // rev seg1 & seg2
                    cand[5] = GetEdgeWeight(g, A, D) + GetEdgeWeight(g, E, B) + GetEdgeWeight(g, C, F);       // swap segments
                    cand[6] = GetEdgeWeight(g, A, E) + GetEdgeWeight(g, D, B) + GetEdgeWeight(g, C, F);       // rev seg2, then seg1
                    cand[7] = GetEdgeWeight(g, A, D) + GetEdgeWeight(g, E, C) + GetEdgeWeight(g, B, F);       // seg2, then rev seg1
                    cand[8] = GetEdgeWeight(g, A, E) + GetEdgeWeight(g, D, C) + GetEdgeWeight(g, B, F);       // rev seg2, rev seg1

                    int bestCase = 0;
                    double bestDelta = -DBL_EPSILON; // only strict improvements
                    for (int c = 2; c <= 8; c++) {
                        double delta = cand[c] - removed;
                        if (delta < bestDelta) { bestDelta = delta; bestCase = c; }
                    }

                    if (bestCase != 0) {
                        applyReconnection(path, temp, numVertices, i, j, k, bestCase);
                        initialTour->cost += bestDelta;
                        improved = 1;
                        goto restartLoops; // re-scan from scratch after an improvement
                    }
                }
            }
        }

        restartLoops:;
    }

    printf("  Final Cost after 3-Opt: %.2f\n", initialTour->cost);

    free(temp);
    return initialTour;
}

// rebuilds path as: prefix [0, i) + middle (seg1 = [i, j), seg2 = [j, k), possibly reversed and/or swapped per 'kase') + suffix [k, numVertices). temp is scratch of size numVertices+1.
static void applyReconnection(unsigned int* path, unsigned int* temp, unsigned int numVertices,
                              unsigned int i, unsigned int j, unsigned int k, int kase) {
    unsigned int idx = 0;

    // prefix [0, i)
    for (unsigned int x = 0; x < i; x++) temp[idx++] = path[x];

    // seg1 = path[i .. j-1], seg2 = path[j .. k-1]; place them per the chosen reconnection
    int seg1Rev, seg2Rev, seg2First;
    switch (kase) {
        case 2: seg1Rev = 1; seg2Rev = 0; seg2First = 0; break; // rev seg1, seg2
        case 3: seg1Rev = 0; seg2Rev = 1; seg2First = 0; break; // seg1, rev seg2
        case 4: seg1Rev = 1; seg2Rev = 1; seg2First = 0; break; // rev seg1, rev seg2
        case 5: seg1Rev = 0; seg2Rev = 0; seg2First = 1; break; // seg2, seg1
        case 6: seg1Rev = 0; seg2Rev = 1; seg2First = 1; break; // rev seg2, seg1
        case 7: seg1Rev = 1; seg2Rev = 0; seg2First = 1; break; // seg2, rev seg1
        default: seg1Rev = 1; seg2Rev = 1; seg2First = 1; break; // case 8: rev seg2, rev seg1
    }

    for (int piece = 0; piece < 2; piece++) {
        int doSeg2 = (piece == 0) ? seg2First : !seg2First;
        if (doSeg2) {
            if (!seg2Rev) for (unsigned int x = j; x < k; x++) temp[idx++] = path[x];
            else          for (unsigned int x = k; x > j; x--) temp[idx++] = path[x - 1];
        } else {
            if (!seg1Rev) for (unsigned int x = i; x < j; x++) temp[idx++] = path[x];
            else          for (unsigned int x = j; x > i; x--) temp[idx++] = path[x - 1];
        }
    }

    // suffix [k, numVertices)
    for (unsigned int x = k; x < numVertices; x++) temp[idx++] = path[x];

    for (unsigned int x = 0; x < numVertices; x++) path[x] = temp[x];
    path[numVertices] = path[0]; // close cycle
}