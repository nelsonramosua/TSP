// TabuSearch.c - Implements the Tabu Search metaheuristic for TSP.
//
// O(N^2) per iteration (scan the 2-Opt neighbourhood); iterations = TABU_MULTIPLIER * N.
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Tabu Search is a local search that, unlike 2-Opt, accepts the *best* move each step even when it worsens the tour -- this lets it climb out of local optima.
// To stop it from immediately undoing a move (cycling), the edges a move just added are made "tabu" (forbidden to remove) for a few iterations.
// A tabu move is still allowed if it beats the best tour found so far (the aspiration criterion).
// We keep the best tour seen across all iterations and return it.
//
// Neighbourhood: 2-Opt moves.
// Tabu attribute: the two edges added by a move (stored in a symmetric N x N "tabu until iteration" matrix).
// Rounds off the metaheuristic family (SA / ACO / GA) with a deterministic-memory approach.

// Tune parameters in headers/Metaheuristics.h.

// Resources used:
// https://en.wikipedia.org/wiki/Tabu_search
// F. Glover, "Tabu Search - Part I", ORSA Journal on Computing (1989).

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/Metaheuristics.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>

// reverses tour[a..b] (inclusive).
static void reverseSegment(unsigned int* tour, unsigned int a, unsigned int b) {
    while (a < b) {
        unsigned int t = tour[a]; tour[a] = tour[b]; tour[b] = t;
        a++; b--;
    }
}

Tour* TabuSearch_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices < 4) return NearestNeighbour_FindTour(g, 0); // too small to be interesting

    // start from a Nearest Neighbour tour
    Tour* nn = NearestNeighbour_FindTour(g, 0);
    if (!nn) return NULL;

    unsigned int* cur = malloc(numVertices * sizeof(unsigned int));
    unsigned int* best = malloc(numVertices * sizeof(unsigned int));
    // symmetric "tabu until" matrix over edges (city pairs); 0 = never tabu
    unsigned int* tabu = calloc((size_t)numVertices * numVertices, sizeof(unsigned int));
    if (!cur || !best || !tabu) {
        free(cur); free(best); free(tabu); TourDestroy(&nn);
        return NULL;
    }

    for (unsigned int i = 0; i < numVertices; i++) cur[i] = best[i] = nn->path[i];
    double curCost = nn->cost, bestCost = nn->cost;
    TourDestroy(&nn);

    unsigned int iterations = TABU_MULTIPLIER * numVertices;

    for (unsigned int iter = 1; iter <= iterations; iter++) {
        // find the best admissible 2-Opt move (allowing worsening moves)
        double bestDelta = DBL_MAX;
        unsigned int bi = 0, bj = 0;
        int found = 0;

        for (unsigned int i = 0; i < numVertices - 1; i++) {
            unsigned int a = cur[i], b = cur[i + 1];
            double dab = GetEdgeWeight(g, a, b);
            for (unsigned int j = i + 2; j < numVertices; j++) {
                unsigned int c = cur[j], d = cur[(j + 1) % numVertices];
                if (i == 0 && j == numVertices - 1) continue; // would reverse whole tour (no-op)

                double dcd = GetEdgeWeight(g, c, d);
                double dac = GetEdgeWeight(g, a, c);
                double dbd = GetEdgeWeight(g, b, d);
                if (dab == DBL_MAX || dcd == DBL_MAX || dac == DBL_MAX || dbd == DBL_MAX) continue;

                double delta = (dac + dbd) - (dab + dcd); // 2-Opt: add (a,c),(b,d); remove (a,b),(c,d)

                // tabu if the move would remove a currently-protected edge...
                int isTabu = tabu[(size_t)a * numVertices + b] > iter ||
                             tabu[(size_t)c * numVertices + d] > iter;
                // ...unless it beats the best tour ever seen (aspiration)
                if (isTabu && curCost + delta >= bestCost - 1e-9) continue;

                if (delta < bestDelta) { bestDelta = delta; bi = i; bj = j; found = 1; }
            }
        }

        if (!found) break; // everything tabu and nothing aspirated (rare)

        // apply the chosen move
        unsigned int a = cur[bi], b = cur[bi + 1];
        unsigned int c = cur[bj], d = cur[(bj + 1) % numVertices];
        reverseSegment(cur, bi + 1, bj);
        curCost += bestDelta;

        // protect the two edges just added, so we don't immediately undo them
        tabu[(size_t)a * numVertices + c] = tabu[(size_t)c * numVertices + a] = iter + TABU_TENURE;
        tabu[(size_t)b * numVertices + d] = tabu[(size_t)d * numVertices + b] = iter + TABU_TENURE;

        if (curCost < bestCost - 1e-9) {
            bestCost = curCost;
            memcpy(best, cur, numVertices * sizeof(unsigned int));
        }
    }

    Tour* result = TourCreate(numVertices);
    if (!result) { free(cur); free(best); free(tabu); return NULL; }
    for (unsigned int i = 0; i < numVertices; i++) result->path[i] = best[i];
    result->path[numVertices] = result->path[0];
    result->cost = bestCost;

    free(cur); free(best); free(tabu);
    return result;
}