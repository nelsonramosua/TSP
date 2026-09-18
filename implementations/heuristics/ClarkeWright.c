// ClarkeWright.c - Implements the Clarke-Wright Savings constructive heuristic for TSP.
//
// O(N^2 * log N) (dominated by sorting the O(N^2) savings).
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// A different *class* of construction from the insertion family.
// Pick a depot (vertex 0); every other city starts as its own out-and-back stub depot->c->depot.
// The "saving" of linking i and j directly is s(i,j) = d(0,i) + d(0,j) - d(i,j).
// We process links by decreasing saving, joining two chains whenever both endpoints are still free (degree < 2) and are not already in the same chain (union-find guards against premature sub-cycles).
// On a complete graph this leaves a single Hamiltonian path over the customers, which we close through the depot.

// Resources used:
// https://en.wikipedia.org/wiki/Vehicle_routing_problem  (Clarke & Wright, 1964)
// https://web.mit.edu/urban_or_book/www/book/chapter6/6.4.12.html

#include "../../TravelingSalesmanProblem.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

typedef struct {
    unsigned int i, j;
    double saving;
} Saving;

// union-find (path-compressed) over customer vertices
static unsigned int ufFind(unsigned int* parent, unsigned int x) {
    while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
    return x;
}

// sort savings by decreasing value (qsort comparator)
static int compareSavings(const void* a, const void* b) {
    double sa = ((const Saving*)a)->saving;
    double sb = ((const Saving*)b)->saving;
    if (sa < sb) return 1;   // larger saving first
    if (sa > sb) return -1;
    return 0;
}

Tour* ClarkeWright_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices < 2) return NULL;
    // savings needs a depot plus at least two customers to have anything to merge
    if (numVertices < 4) return NearestNeighbour_FindTour(g, 0);

    const unsigned int depot = 0;
    unsigned int numCustomers = numVertices - 1;

    // per-vertex chain state (indexed by vertex id; depot slot unused)
    unsigned int* parent = malloc(numVertices * sizeof(unsigned int));
    unsigned int* degree = calloc(numVertices, sizeof(unsigned int));
    unsigned int (*link)[2] = malloc(numVertices * sizeof(*link)); // link[v][0..1] = chain neighbours
    unsigned long maxSavings = (unsigned long)numCustomers * (numCustomers - 1) / 2;
    Saving* savings = malloc(maxSavings * sizeof(Saving));
    if (!parent || !degree || !link || !savings) {
        free(parent); free(degree); free(link); free(savings);
        return NULL;
    }

    for (unsigned int v = 0; v < numVertices; v++) { parent[v] = v; link[v][0] = link[v][1] = UINT_MAX; }

    // build the savings list over customer pairs
    unsigned long count = 0;
    for (unsigned int i = 1; i < numVertices; i++)
        for (unsigned int j = i + 1; j < numVertices; j++) {
            double di = GetEdgeWeight(g, depot, i);
            double dj = GetEdgeWeight(g, depot, j);
            double dij = GetEdgeWeight(g, i, j);
            if (di == DBL_MAX || dj == DBL_MAX || dij == DBL_MAX) continue; // unlinkable pair
            savings[count++] = (Saving){ i, j, di + dj - dij };
        }

    qsort(savings, count, sizeof(Saving), compareSavings);

    // merge chains greedily by decreasing saving
    for (unsigned long s = 0; s < count; s++) {
        unsigned int i = savings[s].i, j = savings[s].j;
        if (degree[i] >= 2 || degree[j] >= 2) continue;      // an endpoint is already interior
        if (ufFind(parent, i) == ufFind(parent, j)) continue; // would close a sub-cycle
        link[i][degree[i]++] = j;
        link[j][degree[j]++] = i;
        parent[ufFind(parent, i)] = ufFind(parent, j);
    }

    // stitch the resulting chains into one tour: depot, then each chain end-to-end, back to depot
    Tour* tour = TourCreate(numVertices);
    if (!tour) { free(parent); free(degree); free(link); free(savings); return NULL; }

    int* visited = calloc(numVertices, sizeof(int));
    if (!visited) { TourDestroy(&tour); free(parent); free(degree); free(link); free(savings); return NULL; }

    unsigned int idx = 0;
    tour->path[idx++] = depot;

    // walk every chain starting from one of its endpoints (a customer with degree < 2)
    for (unsigned int c = 1; c < numVertices; c++) {
        if (visited[c] || degree[c] >= 2) continue; // only start from a chain endpoint

        unsigned int prev = UINT_MAX, cur = c;
        while (cur != UINT_MAX && !visited[cur]) {
            tour->path[idx++] = cur;
            visited[cur] = 1;
            unsigned int next = UINT_MAX;
            for (int e = 0; e < 2; e++)
                if (link[cur][e] != UINT_MAX && link[cur][e] != prev && !visited[link[cur][e]]) {
                    next = link[cur][e];
                    break;
                }
            prev = cur;
            cur = next;
        }
    }

    tour->path[numVertices] = tour->path[0]; // close cycle

    // compute the tour cost
    double totalCost = 0.0;
    for (unsigned int e = 0; e < numVertices; e++) {
        double w = GetEdgeWeight(g, tour->path[e], tour->path[e + 1]);
        if (w == DBL_MAX) { totalCost = DBL_MAX; break; }
        totalCost += w;
    }
    tour->cost = totalCost;

    free(visited); free(parent); free(degree); free(link); free(savings);
    return tour;
}