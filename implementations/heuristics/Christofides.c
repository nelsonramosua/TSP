// Christofides.c - Implements Christofides' algorithm (guaranteed 1.5-approximation for TSP).
//
// O(N^3).
//
// Nelson Ramos, 124921.
//
// November, 2025.
// 
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Resources used:
// https://algorithms.discrete.ma.tum.de/graph-algorithms/hierholzer/index_en.html
// https://alon.kr/posts/christofides
// https://en.wikipedia.org/wiki/Blossom_algorithm
// https://github.com/suddhabrato/edmonds-blossom-algorithm

// I could not for the life of me get this to work well... must have some bug in here.

#include "../../TravelingSalesmanProblem.h"
#include "blossom/BlossomWrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>

// Forward declarations

// finds all vertices with odd degree in graph.
unsigned int* findOddVertices(const Graph* g, unsigned int* count);

// computes the Minimum Weight Perfect Matching (MWPM) on the odd-degree subgraph.
// uses imported C++ Blossom library indicated above.
Graph* computeExactMwpm(const Graph* g, unsigned int* odd, unsigned int oddCount);

// finds an Eulerian tour by Hierholzer's alg, optimized for a given edge count matrix.
unsigned int* findEulerianTourFromEdgeCounts(const int *edgeCntIn, unsigned int N, unsigned int *tourSize);

// converts the Eulerian tour into a Hamiltonian cycle by skipping repeated vertices (shortcut).
double shortcutEuler(const Graph* g, unsigned int* euler, unsigned int size, unsigned int* hamilton, unsigned int N);

// external MST function. go to implementations/mst/Prim_MST.c
extern Graph* MST_PrimGraph(const Graph* g);

Tour* Christofides_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices == 0) return NULL;

    Tour* tour = TourCreate(numVertices);
    if (!tour) return NULL;

    Graph* mst = MST_PrimGraph(g); // find MST

    // find vertices w/ odd deg in MST
    unsigned int oddCount = 0;
    unsigned int* odd = findOddVertices(mst, &oddCount);

    // find mwpm on subgraph with odd vertices
    Graph* matching = NULL;
    if (oddCount > 0) matching = computeExactMwpm(g, odd, oddCount);

    int* multi = calloc(numVertices * numVertices, sizeof(int));
    if (!multi) { free(odd); GraphDestroy(&mst); GraphDestroy(&matching); return tour; }

    // add MST edges
    for (unsigned int u = 0; u < numVertices; u++) {
        unsigned int* adj = GraphGetAdjacentsTo(mst, u);
        for (unsigned int k = 1; k <= adj[0]; k++) {
            unsigned int v = adj[k];
            if (u < v) multi[u * numVertices + v] += 1;
        }
        free(adj);
    }

    // add matching edges
    if (matching) 
        for (unsigned int u = 0; u < numVertices; u++) {
            unsigned int* adj = GraphGetAdjacentsTo(matching, u);
            for (unsigned int k = 1; k <= adj[0]; k++) {
                unsigned int v = adj[k];
                if (u < v) multi[u * numVertices + v] += 1;
            }
            free(adj);
        }
    // the result (multi)graph is Eulerian (all degs are even).

    unsigned int eulerSize = 0;
    unsigned int* euler = findEulerianTourFromEdgeCounts(multi, numVertices, &eulerSize); // find an Eulerian Tour on the combined multigraph

    free(multi);

    if (!euler || eulerSize == 0) {
        fprintf(stderr, "Euler tour construction failed.\n");
        free(odd); GraphDestroy(&mst); GraphDestroy(&matching); return tour;
    }

    // eulerian to hamiltonian (with shortcut):
    // skip repeat verts in Euler path and take direct edge (shortcut) from last visited unique vert to cur unique vert in the orig. graph.
    tour->cost = shortcutEuler(g, euler, eulerSize, tour->path, numVertices);
    tour->path[numVertices] = tour->path[0]; // close loop

    free(odd); free(euler); GraphDestroy(&mst); GraphDestroy(&matching);

    return tour;
}

unsigned int* findOddVertices(const Graph* g, unsigned int* count) {
    unsigned int numVertices = GraphGetNumVertices(g);
    unsigned int* odd = malloc(numVertices * sizeof(unsigned int));
    if (!odd) { *count = 0; return NULL; }

    *count = 0;
    for (unsigned int v = 0; v < numVertices; v++) {
        unsigned int* adj = GraphGetAdjacentsTo(g, v);
        unsigned int deg = adj[0];
        free(adj); // is doing it this way stupid? maybe we should alter Graph.h. maybe too tired.
        
        if (deg % 2 == 1) odd[(*count)++] = v;
    }

    if (*count == 0) { free(odd); return NULL; }
    // resize array to fit (exactly) count elems.
    unsigned int* finalOdd = realloc(odd, (*count) * sizeof(unsigned int));
    return finalOdd ? finalOdd : odd; 
}

Graph* computeExactMwpm(const Graph* g, unsigned int* odd, unsigned int oddCount) {
    if (oddCount == 0) return GraphCreate(GraphGetNumVertices(g), 0, 1);
    if (oddCount & 1) return NULL; // vert. num must be an even for perfect matching

    // prep edge list for Blossom library
    int maxEdges = oddCount * (oddCount - 1) / 2;
    int* u = malloc(maxEdges * sizeof(int));
    int* v = malloc(maxEdges * sizeof(int));
    double* w = malloc(maxEdges * sizeof(double));
    int edgeIndex = 0;

    // build complete graph on odd verts
    for (unsigned int i = 0; i < oddCount; i++) 
        for (unsigned int j = i + 1; j < oddCount; j++) {
            // u and v inds refer to odd arr inds (0 - oddCount-1)
            u[edgeIndex] = i; v[edgeIndex] = j;
            // weight comes from original graph, using the real vert inds (odd[i], odd[j])
            w[edgeIndex] = GetEdgeWeight(g, odd[i], odd[j]);
            edgeIndex++;
        }

    // call the external wrapper to Blossom (which is in cpp)
    int* matchArr = malloc(oddCount * sizeof(int));
    blossomMWPM((int)oddCount, edgeIndex, u, v, w, matchArr);

    // remake Graph* matching with original vert inds
    Graph* matching = GraphCreate(GraphGetNumVertices(g), 0, 1);
    for (unsigned int i = 0; i < oddCount; i++) {
        int j = matchArr[i];
        // we only add edge once (i < j) & must be valid match (j != -1)
        if (j != -1 && i < (unsigned int)j) {
            unsigned int uIdx = odd[i];
            unsigned int vIdx = odd[j];
            double weight = GetEdgeWeight(g, uIdx, vIdx);
            GraphAddWeightedEdge(matching, uIdx, vIdx, weight);
        }
    }

    free(u); free(v); free(w); free(matchArr);

    return matching;
}

unsigned int* findEulerianTourFromEdgeCounts(const int *edgeCntIn, unsigned int numVertices, unsigned int *tourSize) {
    // copy counts (Hierholzer's consumes edges)
    int *edgeCnt = malloc(numVertices * numVertices * sizeof(int));
    if (!edgeCnt) { *tourSize = 0; return NULL; }
    memcpy(edgeCnt, edgeCntIn, numVertices * numVertices * sizeof(int));

    // number of edges
    unsigned int totalEdges = 0;
    for (unsigned int i = 0; i < numVertices * numVertices; ++i) totalEdges += (unsigned int)edgeCnt[i];
    
    // max size of Tour is totalEdges + 1 (close loop)
    unsigned int maxSize = totalEdges + 1; 
    unsigned int *stack = malloc(maxSize * sizeof(unsigned int)); // we could use a separate adt for this stack but it's not worth it.
    unsigned int *tour = malloc(maxSize * sizeof(unsigned int));
    if (!stack || !tour) { free(edgeCnt); free(stack); free(tour); *tourSize = 0; return NULL; }

    unsigned int stackPtr = 0, tourPtr = 0;
    stack[stackPtr++] = 0; // start tour at vertex 0 (could be implemented pseudorandomly?, but prob not worth it)

    while (stackPtr > 0) {
        unsigned int u = stack[stackPtr - 1];
        
        // find neighbor v with remaining edge (u, v)
        unsigned int vFound = numVertices; // N = not found
        for (unsigned int v = 0; v < numVertices; ++v) {
            if (v == u) continue;
            
            // calc unique storage index (a < b)
            unsigned int a = u < v ? u : v;
            unsigned int b = u < v ? v : u;
            
            if (edgeCnt[a * numVertices + b] > 0) { vFound = v; break; }
        }

        if (vFound != numVertices) {
            unsigned int v = vFound;
            unsigned int a = u < v ? u : v;
            unsigned int b = u < v ? v : u;
            
            // consume parallel edge
            edgeCnt[a * numVertices + b]--;
            // push new vertex into stack
            stack[stackPtr++] = v;
        } else { tour[tourPtr++] = u; stackPtr--; } // no available edges from u, add it to the tour and backtrack (pop)
    }

    free(stack); free(edgeCnt);

    // trim array
    unsigned int *res = realloc(tour, tourPtr * sizeof(unsigned int));
    *tourSize = tourPtr;
    return res ? res : tour;
}

double shortcutEuler(const Graph* g, unsigned int* euler, unsigned int size, unsigned int* hamilton, unsigned int numVertices) {
    // array for track unique verts in the Hamiltonian cycle
    int* visited = calloc(numVertices, sizeof(int));
    unsigned int pos = 0;
    double cost = 0.0;
    
    // start with first vertex of Euler
    unsigned int last = euler[0];
    hamilton[pos++] = last;
    visited[last] = 1;

    // traverse rest of Euler
    for (unsigned int i = 1; i < size; i++) {
        unsigned int cur = euler[i];
        
        // if cur vert not visited yet, include
        if (!visited[cur]) {
            cost += GetEdgeWeight(g, last, cur);
            hamilton[pos++] = cur;
            visited[cur] = 1;
            last = cur;
        }
    }
    
    // close cycle
    cost += GetEdgeWeight(g, last, hamilton[0]);
    hamilton[pos] = hamilton[0]; 
    
    free(visited);
    return cost;
}