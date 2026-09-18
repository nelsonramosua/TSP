// Greedy.c - Implements the Greedy (cheapest-insertion) approximation algorithm.
//
// O(N^2 * log N) with the priority queue (was O(N^3) with a full rescan each step).
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Updates:
// 1. Nelson Ramos, September 2026:
//      Use the PriorityQueue ADT to avoid rescanning every (unvisited vertex, tour edge) pair each step.
//      Each unvisited vertex keeps its cheapest insertion cost in the heap.
//      After a vertex is inserted we only update the two new edges it created (a decrease-key), and re-validate a popped vertex against the current tour.
//      That turns the per-step O(N^2) rescan into O(N log N), i.e. O(N^2 log N) overall.
//      The tour produced is identical to the previous O(N^3) version.

// Resources used:
// https://www.geeksforgeeks.org/dsa/travelling-salesman-problem-greedy-approach/

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/PriorityQueue.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// Cheapest insertion cost of vertex v between tour vertices a and its successor b, i.e. w(a,v) + w(v,b) - w(a,b).
static double insertionCost(const Graph* g, unsigned int a, unsigned int b, unsigned int v) {
    return GetEdgeWeight(g, a, v) + GetEdgeWeight(g, v, b) - GetEdgeWeight(g, a, b);
}

// Recomputes v's cheapest insertion over the current tour, walking it in order from vertex 0.
// Writes the best predecessor (insert v between *bestPred and succ[*bestPred]).
// Ties are broken towards the earlier tour position, matching the old rescan.
static double bestInsertion(const Graph* g, const unsigned int* succ, unsigned int tourSize,
                            unsigned int v, unsigned int* bestPred) {
    double best = DBL_MAX;
    unsigned int a = 0;
    for (unsigned int step = 0; step < tourSize; step++) {
        unsigned int b = succ[a];
        double inc = insertionCost(g, a, b, v);
        if (inc < best) { best = inc; *bestPred = a; }
        a = b;
    }
    return best;
}

Tour* Greedy_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices == 0) return NULL;

    // for graphs with < 3 vertices, use NN approx.
    if (numVertices < 3) return NearestNeighbour_FindTour(g, 0);

    Tour* tour = TourCreate(numVertices);
    if (!tour) return NULL;

    // succ[a] = successor of a in the current (sub)tour cycle.
    // The heap key of each unvisited vertex is its cheapest insertion cost; the position is (re)derived when the vertex is popped, so it need not be stored.
    unsigned int* succ = malloc(numVertices * sizeof(unsigned int));
    PriorityQueue* pq = PQCreate(numVertices);
    if (!succ || !pq) {
        free(succ); PQDestroy(&pq); TourDestroy(&tour);
        return NULL;
    }

    // init subtour ( 0 - 1 - 2 - 0 )
    succ[0] = 1; succ[1] = 2; succ[2] = 0;
    unsigned int tourSize = 3;
    double totalCost = GetEdgeWeight(g, 0, 1) + GetEdgeWeight(g, 1, 2) + GetEdgeWeight(g, 2, 0);

    // seed the queue with every unvisited vertex's cheapest insertion into the triangle
    for (unsigned int v = 3; v < numVertices; v++) {
        unsigned int pred = 0;
        double best = bestInsertion(g, succ, tourSize, v, &pred);
        PQInsert(pq, v, best);
    }

    // insert the globally cheapest vertex each step, until the tour is complete
    while (tourSize < numVertices) {
        unsigned int v = PQExtractMin(pq);
        if (v == numVertices) { // sentinel: queue empty but tour incomplete (disconnected)
            fprintf(stderr, "Error: Greedy failed to find the next cheapest insertion (graph disconnected?).\n");
            free(succ); PQDestroy(&pq); TourDestroy(&tour);
            return NULL;
        }

        // re-validate: the stored cost may be stale-low if v's chosen edge was since removed
        unsigned int pred = 0;
        double best = bestInsertion(g, succ, tourSize, v, &pred);
        if (best > pq->key[v] + 1e-9) {
            // stale: v's real cheapest is higher now -> put it back with the true cost
            PQInsert(pq, v, best);
            continue;
        }

        // insert v between 'a' and 'b', splitting edge (a, b) into (a, v) and (v, b)
        unsigned int a = pred;
        unsigned int b = succ[a];
        succ[a] = v;
        succ[v] = b;
        totalCost += best;
        tourSize++;

        // only the two new edges can lower another unvisited vertex's insertion cost
        for (unsigned int u = 3; u < numVertices; u++) {
            if (!PQContains(pq, u)) continue;
            double via_a = insertionCost(g, a, v, u); // insert u between a and v
            double via_v = insertionCost(g, v, b, u); // insert u between v and b
            double cand = via_a <= via_v ? via_a : via_v;
            if (cand < pq->key[u]) PQDecreaseKey(pq, u, cand);
        }
    }

    // materialize the cycle from succ[], starting at vertex 0
    unsigned int cur = 0;
    for (unsigned int i = 0; i < numVertices; i++) {
        tour->path[i] = cur;
        cur = succ[cur];
    }
    tour->path[numVertices] = tour->path[0]; // close cycle
    tour->cost = totalCost;

    free(succ); PQDestroy(&pq);
    return tour;
}