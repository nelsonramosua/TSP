// BranchAndBound.c - Solves TSP exactly via Branch & Bound with a lower-bound.
//
// Worst case O(N!), but a lower-bound test prunes most of the tree.
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Unlike the plain pruned brute force (which only cuts a branch when the partial cost already exceeds the best tour), this uses an admissible *lower bound* on the cost to finish the tour:
// Every vertex still needing an outgoing edge (the current path end and each unvisited vertex) contributes at least its cheapest incident edge, so
//     bound = costSoFar + minOut[last] + sum over unvisited of minOut[v]
// is a valid lower bound.
// If that already reaches the best complete tour found, the branch is pruned.
// We seed the incumbent with a Nearest Neighbour tour and expand children nearest-first, so a good bound appears early and prunes hard.
// The result is the exact optimum.

#include "../../TravelingSalesmanProblem.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

typedef struct {
    const Graph* g;
    unsigned int numVertices;
    double* minOut;         // minOut[v] = cheapest edge incident to v
    int* visited;
    unsigned int* path;     // current partial path (path[0..depth-1])
    unsigned int* bestPath; // best full path found (numVertices entries)
    double best;            // cost of the best complete tour so far (incumbent)
} BnB;

// cheapest edge incident to v (over all other reachable vertices), or DBL_MAX if isolated.
static double minOutgoing(const Graph* g, unsigned int v, unsigned int numVertices) {
    double m = DBL_MAX;
    for (unsigned int u = 0; u < numVertices; u++) {
        if (u == v) continue;
        double w = GetEdgeWeight(g, v, u);
        if (w < m) m = w;
    }
    return m;
}

// depth = number of vertices placed in path[]; last = path[depth-1]; sumUnv = sum of minOut over the still-unvisited vertices.
// Explores completions of the current partial path.
static void branch(BnB* s, unsigned int depth, unsigned int last, double costSoFar, double sumUnv) {
    unsigned int n = s->numVertices;

    if (depth == n) {
        double closing = GetEdgeWeight(s->g, last, s->path[0]);
        if (closing == DBL_MAX) return;
        double total = costSoFar + closing;
        if (total < s->best) {
            s->best = total;
            for (unsigned int i = 0; i < n; i++) s->bestPath[i] = s->path[i];
        }
        return;
    }

    // lower bound on any completion from here; prune if it can't beat the incumbent
    double bound = costSoFar + s->minOut[last] + sumUnv;
    if (bound >= s->best) return;

    // gather unvisited children and expand them nearest-to-`last` first (better pruning)
    unsigned int* cand = malloc((n - depth) * sizeof(unsigned int));
    if (!cand) return;
    unsigned int k = 0;
    for (unsigned int v = 0; v < n; v++) if (!s->visited[v]) cand[k++] = v;
    for (unsigned int a = 0; a < k; a++) { // selection sort by distance from last (k is small)
        unsigned int m = a;
        for (unsigned int b = a + 1; b < k; b++)
            if (GetEdgeWeight(s->g, last, cand[b]) < GetEdgeWeight(s->g, last, cand[m])) m = b;
        unsigned int t = cand[a]; cand[a] = cand[m]; cand[m] = t;
    }

    for (unsigned int i = 0; i < k; i++) {
        unsigned int next = cand[i];
        double w = GetEdgeWeight(s->g, last, next);
        if (w == DBL_MAX) continue;
        if (costSoFar + w + sumUnv - s->minOut[next] >= s->best) continue; // quick per-child bound

        s->visited[next] = 1;
        s->path[depth] = next;
        branch(s, depth + 1, next, costSoFar + w, sumUnv - s->minOut[next]);
        s->visited[next] = 0;
    }

    free(cand);
}

Tour* BranchAndBound_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices < 2) return NULL;

    BnB s;
    s.g = g;
    s.numVertices = numVertices;
    s.minOut = malloc(numVertices * sizeof(double));
    s.visited = calloc(numVertices, sizeof(int));
    s.path = malloc(numVertices * sizeof(unsigned int));
    s.bestPath = malloc(numVertices * sizeof(unsigned int));
    if (!s.minOut || !s.visited || !s.path || !s.bestPath) {
        free(s.minOut); free(s.visited); free(s.path); free(s.bestPath);
        return NULL;
    }

    double sumMinOut = 0.0;
    for (unsigned int v = 0; v < numVertices; v++) {
        s.minOut[v] = minOutgoing(g, v, numVertices);
        sumMinOut += s.minOut[v];
    }

    // seed the incumbent with a Nearest Neighbour tour (a good early upper bound)
    s.best = DBL_MAX;
    Tour* nn = NearestNeighbour_FindTour(g, 0);
    if (nn) {
        s.best = nn->cost;
        for (unsigned int i = 0; i < numVertices; i++) s.bestPath[i] = nn->path[i];
        TourDestroy(&nn);
    }

    // fix vertex 0 as the start; sumUnv starts as the sum of minOut over vertices 1..N-1
    s.visited[0] = 1;
    s.path[0] = 0;
    branch(&s, 1, 0, 0.0, sumMinOut - s.minOut[0]);

    Tour* tour = NULL;
    if (s.best != DBL_MAX) {
        tour = TourCreate(numVertices);
        if (tour) {
            for (unsigned int i = 0; i < numVertices; i++) tour->path[i] = s.bestPath[i];
            tour->path[numVertices] = tour->path[0];
            tour->cost = s.best;
        }
    } else {
        fprintf(stderr, "Warning: Branch & Bound found no tour (graph disconnected?).\n");
    }

    free(s.minOut); free(s.visited); free(s.path); free(s.bestPath);
    return tour;
}