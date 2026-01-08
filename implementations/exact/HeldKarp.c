// HeldKarp.c - Solves TSP using the Held-Karp Dynamic Programming algorithm.
//
// (O(N^2 * 2^N)).
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// I cannot at all take credit for this. It was "transcribed" from the C++ implementation here:
// https://rosettacode.org/wiki/Held%E2%80%93Karp_algorithm
// To be perfectly candid... I did not really, fully understand the algorithm.

#include "../../TravelingSalesmanProblem.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h> 
#include <limits.h>
#include <string.h>
#include <assert.h>

// Macro helper to determine the minimum of two doubles
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// ADT to hold DP table and parent pointers (for path reconstruction).
typedef struct {
    double** dp;            // dp[subsetMask][j] reps. min cost of path started at v_0, visiting verts in 'subsetMask' & ending at v_j
    unsigned int** parent;  // parent[subsetMask][j] reps. v_k (before v_j) in optimal path stored in dp[subsetMask][j]
                            // essentially, maintains "predecessor" table.
    double** dists;         // Precomputed distance matrix O(N^2) to avoid expensive graph lookups in inner loop
    unsigned int numStates;
    unsigned int numVertices;
} HeldKarpTable;

// Inits & allocs the DP table & parent table.
static HeldKarpTable* initializeTable(const Graph* g);
// Frees memory allocd for HeldKarpTable ADT.
static void destroyTable(HeldKarpTable* table);

// helper to count set bits in mask. I think gcc has one builtin function for this.
// static inline unsigned int popCount(unsigned int n);
// helper to count trailing 0 #. I think gcc has one builtin function for this.
// static inline int ctz(unsigned int v);

// after testing, I decided to use the builtin versions, __builtin_popcount(n) and __builtin_ctz(n), because it made the algorithm that much faster.

Tour* HeldKarp_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices < 2) return NULL; 

    if (numVertices >= (unsigned int)(sizeof(unsigned int) * 8)) return NULL;  // masks are 32-bit. It would cause memory corruption or buffer overruns
                                                          // ok, because we only test this up to 21 vertices...

    // init DP table (and precompute distances)
    HeldKarpTable* hkt = initializeTable(g);
    if (!hkt) return NULL;

    // mask for set of all verts.
    unsigned int allVertsMask = (1U << numVertices) - 1;

    // mask for starting vert {0}
    unsigned int maskZero = 1U << 0; 

    // base case (start at vert 0)
    hkt->dp[maskZero][0] = 0.0; 

    // DP calc.
    for (unsigned int m = 2; m <= numVertices; m++) {
        // iterate over subsetMask
        for (unsigned int subsetMask = 1; subsetMask < (1U << numVertices); subsetMask++) {
            // discard subsets without start vert (0) or without exact cur size 'm'
            if (!(subsetMask & maskZero)) continue; 
            // if (popCount(subsetMask) != m) continue; 
            if (__builtin_popcount(subsetMask) != m) continue;

            // calc min cost to reach v_j (j in subset & j != 0)
            for (unsigned int j = 1; j < numVertices; j++) {
                if (!(subsetMask & (1U << j))) continue; // v_j not in cur subset

                // recurrence: dp[S][j] = min_{k in S, k != j} { dp[S\{j}][k] + weight(k, j) }
                unsigned int subsetMinusJ = subsetMask & ~(1U << j); // prev state mask: S without j

                // iterate over predecessors k in prev state (bit iteration)
                unsigned int bits = subsetMinusJ;
                while (bits) {
                    // index of least-significant set bit
                    // unsigned int k = ctz(bits); 
                    unsigned int k = __builtin_ctz(bits);

                    bits &= (bits - 1);  // clear LSB (set)

                    if (k == j) continue;

                    double previousCost = hkt->dp[subsetMinusJ][k];
                    if (previousCost == DBL_MAX) continue;  // skip if prev path unreachable

                    // OPTIMIZATION: Use precomputed distance
                    double edgeWeight = hkt->dists[k][j];
                    if (edgeWeight == DBL_MAX) continue; // skip if edge k->j invalid

                    double newCost = previousCost + edgeWeight;

                    // update DP table if cheaper path to (S, j) found
                    if (newCost < hkt->dp[subsetMask][j]) {
                        hkt->dp[subsetMask][j] = newCost;
                        hkt->parent[subsetMask][j] = k;  // save k as j's predecessor
                    }
                }
            }
        }
    }

    double finalMinCost = DBL_MAX;
    unsigned int finalEndVert = UINT_MAX;

    // check final edge from v_j back to v_0
    for (unsigned int j = 1; j < numVertices; j++) {
        double pathCost = hkt->dp[allVertsMask][j];
        if (pathCost == DBL_MAX) continue;

        double edgeToZero = hkt->dists[j][0];
        if (edgeToZero == DBL_MAX) continue;

        double totalCost = pathCost + edgeToZero;

        if (totalCost < finalMinCost) {
            finalMinCost = totalCost;
            finalEndVert = j; // vert right before v_0 in optimal cycle
        }
    }

    Tour* bestTour = TourCreate(numVertices);
    if (!bestTour) { destroyTable(hkt); return NULL; }
    bestTour->cost = finalMinCost;

    if (finalMinCost == DBL_MAX) { TourDestroy(&bestTour); destroyTable(hkt); return NULL; } // path not found (discon graph, should not happen)

    // reconstruct path backwards w/ parent pointers
    unsigned int* pathIndices = bestTour->path; // path[0] to path[numVertices]
    
    unsigned int currentVert = finalEndVert;
    unsigned int currentMask = allVertsMask;
    
    // fill path arr backwards, from 2nd-to-last vert (path[numVertices-1]) down to path[1].
    for (unsigned int i = numVertices - 1; i > 0; i--) {
        if (currentVert == UINT_MAX) { TourDestroy(&bestTour); destroyTable(hkt); return NULL; } 

        pathIndices[i] = currentVert;
        
        unsigned int prevVert = hkt->parent[currentMask][currentVert];
        currentMask &= ~(1U << currentVert); // remove currentVert from mask
        currentVert = prevVert;
    }
    pathIndices[0] = 0; // start
    pathIndices[numVertices] = 0; // end (loop)

    destroyTable(hkt);
    return bestTour;
}


static HeldKarpTable* initializeTable(const Graph* g) {
    if (!g) return NULL;
    unsigned int numVertices = GraphGetNumVertices(g);
    unsigned int numStates = 1U << numVertices;

    HeldKarpTable* table = (HeldKarpTable*) malloc(sizeof(HeldKarpTable));
    if (!table) return NULL;

    table->numVertices = numVertices;
    table->numStates = numStates;
    table->dp = (double**) calloc(numStates, sizeof(double*));
    table->parent = (unsigned int**) calloc(numStates, sizeof(unsigned int*));
    table->dists = (double**) calloc(numVertices, sizeof(double*));
    
    if (!table->dp || !table->parent || !table->dists) {
        if (table->dp) free(table->dp);
        if (table->parent) free(table->parent);
        if (table->dists) free(table->dists);
        free(table);
        return NULL;
    }

    // alloc and fill dist matrix
    for (unsigned int i = 0; i < numVertices; i++) {
        table->dists[i] = (double*) malloc(numVertices * sizeof(double));
        if (!table->dists[i]) { destroyTable(table); return NULL; }
        for (unsigned int j = 0; j < numVertices; j++) {
            if (i == j) table->dists[i][j] = 0.0;
            else table->dists[i][j] = GetEdgeWeight(g, i, j);
        }
    }

    for (unsigned int subsetMask = 0; subsetMask < numStates; subsetMask++) {
        table->dp[subsetMask] = (double*) malloc(numVertices * sizeof(double));
        table->parent[subsetMask] = (unsigned int*) malloc(numVertices * sizeof(unsigned int));

        if (!table->dp[subsetMask] || !table->parent[subsetMask]) { destroyTable(table); return NULL; } 
    
        // init path costs to DBL_MAX (unreached states)
        for (unsigned int j = 0; j < numVertices; j++) {
            table->dp[subsetMask][j] = DBL_MAX;
            table->parent[subsetMask][j] = UINT_MAX;
        }
    }
    
    return table;
}


// Frees memory allocd for HeldKarpTable ADT.
static void destroyTable(HeldKarpTable* table) {
    if (!table) return;

    if (table->dp) {
        for (unsigned int subsetMask = 0; subsetMask < table->numStates; subsetMask++)
            if (table->dp[subsetMask]) free(table->dp[subsetMask]);
        free(table->dp);
    }
    if (table->parent) {
        for (unsigned int subsetMask = 0; subsetMask < table->numStates; subsetMask++)
            if (table->parent[subsetMask]) free(table->parent[subsetMask]);
        free(table->parent);
    }
    if (table->dists) {
        for (unsigned int i = 0; i < table->numVertices; i++)
            if (table->dists[i]) free(table->dists[i]);
        free(table->dists);
    }
    free(table);
}

/*
// helper to count set bits in mask. gcc has a builtin function for this, which was used.
static inline unsigned int popCount(unsigned int n) {
    unsigned int count = 0;
    while (n > 0) {
        n &= (n - 1); // clear set lsb
        count++;
    }
    return count;
}

// helper to count trailing 0 #. gcc has a builtin function for this, which was used.
static inline int ctz(unsigned int v) {
    if (v == 0) return 0;

    int c = 0;
    while ((v & 1) == 0) {
        v >>= 1;
        c++;
    }
    return c;
}
*/