#include "../../TravelingSalesmanProblem.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include "blossom/BlossomWrapper.h"

// Forward declarations
unsigned int* FindOddVertices(const Graph* g, unsigned int* count);
Graph* ComputeExactMWPM(const Graph* g, unsigned int* odd, unsigned int oddCount);
unsigned int* FindEulerianTour(Graph* g, unsigned int* tourSize);
unsigned int* FindEulerianTourFromEdgeCounts(const int *edge_cnt_in, unsigned int N, unsigned int *tourSize);
double ShortcutEuler(const Graph* g, unsigned int* euler, unsigned int size, unsigned int* hamilton, unsigned int N);
extern Graph* MST_PrimGraph(const Graph* g);

// -------------------- CHRISTOFIDES MAIN --------------------
Tour* Christofides_FindTour(const Graph* g) {
    unsigned int N = GraphGetNumVertices(g);
    if (N == 0) return NULL;

    Tour* tour = TourCreate(N);
    if (!tour) return NULL;

    // 1) MST
    Graph* mst = MST_PrimGraph(g);

    // 2) Odd vertices
    unsigned int oddCount = 0;
    unsigned int* odd = FindOddVertices(mst, &oddCount);

    // 3) Exact MWPM
    Graph* matching = NULL;
    if (oddCount > 0)
        matching = ComputeExactMWPM(g, odd, oddCount);

    // ---------------------------------------------------------
    // 4) BUILD CLEAN MULTIGRAPH MATRIX (TRIANGULAR STORAGE: only a < b)
    // ---------------------------------------------------------
    // multi[a*N + b] counts #parallel physical edges for a < b
    int* multi = calloc(N * N, sizeof(int));
    if (!multi) { /* OOM handling */ free(odd); GraphDestroy(&mst); GraphDestroy(&matching); return tour; }

    // --- add MST edges (note: Graph stores undirected edges as two adjacencies,
    //     so only add once when u < v) ---
    for (unsigned int u = 0; u < N; u++) {
        unsigned int* adj = GraphGetAdjacentsTo(mst, u);
        for (unsigned int k = 1; k <= adj[0]; k++) {
            unsigned int v = adj[k];
            if (u < v) {
                multi[u * N + v] += 1;
            }
        }
        free(adj);
    }

    // --- add matching edges (matching is also undirected) ---
    if (matching) {
        for (unsigned int u = 0; u < N; u++) {
            unsigned int* adj = GraphGetAdjacentsTo(matching, u);
            for (unsigned int k = 1; k <= adj[0]; k++) {
                unsigned int v = adj[k];
                if (u < v) {
                    multi[u * N + v] += 1;
                }
            }
            free(adj);
        }
    }

    // ---------------------------------------------------------
    // 5) Euler tour (Hierholzer on the multigraph matrix)
    // ---------------------------------------------------------

    unsigned int eulerSize = 0;
    unsigned int* euler = FindEulerianTourFromEdgeCounts(multi, N, &eulerSize);

    free(multi);

    if (!euler || eulerSize == 0) {
        fprintf(stderr, "Euler construction failed.\n");
        free(odd);
        GraphDestroy(&mst);
        GraphDestroy(&matching);
        return tour;
    }

    // ---------------------------------------------------------
    // 6) Shortcut to Hamiltonian
    // ---------------------------------------------------------
    tour->cost = ShortcutEuler(g, euler, eulerSize, tour->path, N);
    tour->path[N] = tour->path[0];

    free(odd);
    free(euler);
    GraphDestroy(&mst);
    GraphDestroy(&matching);

    return tour;
}

// -------------------- ODD-DEGREE VERTICES --------------------
unsigned int* FindOddVertices(const Graph* g, unsigned int* count) {
    unsigned int N = GraphGetNumVertices(g);
    unsigned int* odd = malloc(N * sizeof(unsigned int));
    if (!odd) { *count = 0; return NULL; }

    *count = 0;
    for (unsigned int v = 0; v < N; v++) {
        unsigned int* adj = GraphGetAdjacentsTo((Graph*)g, v);
        unsigned int deg = adj[0];
        free(adj);
        if (deg % 2 == 1) odd[(*count)++] = v;
    }

    if (*count == 0) { free(odd); return NULL; }
    unsigned int* final_odd = realloc(odd, (*count) * sizeof(unsigned int));
    return final_odd ? final_odd : odd;
}

// -------------------- EXACT MWPM (DP) --------------------
Graph* ComputeExactMWPM(const Graph* g, unsigned int* odd, unsigned int oddCount) {
    if (oddCount == 0) 
        return GraphCreate(GraphGetNumVertices(g), 0, 1);
    if (oddCount & 1) 
        return NULL; // must be even

    // 1) Prepare edge list
    int maxEdges = oddCount * (oddCount - 1) / 2;
    int* u = malloc(maxEdges * sizeof(int));
    int* v = malloc(maxEdges * sizeof(int));
    double* w = malloc(maxEdges * sizeof(double));
    int edgeIndex = 0;

    for (unsigned int i = 0; i < oddCount; i++) {
        for (unsigned int j = i + 1; j < oddCount; j++) {
            u[edgeIndex] = i;
            v[edgeIndex] = j;
            w[edgeIndex] = GetEdgeWeight(g, odd[i], odd[j]);
            edgeIndex++;
        }
    }

    // 2) Call C-compatible Blossom wrapper
    int* match_arr = malloc(oddCount * sizeof(int));
    blossomMWPM((int)oddCount, edgeIndex, u, v, w, match_arr);

    // 3) Build Graph* matching
    Graph* matching = GraphCreate(GraphGetNumVertices(g), 0, 1);
    for (unsigned int i = 0; i < oddCount; i++) {
        int j = match_arr[i];
        if (j != -1 && i < (unsigned int)j) {
            unsigned int uidx = odd[i];
            unsigned int vidx = odd[j];
            double weight = GetEdgeWeight(g, uidx, vidx);
            GraphAddWeightedEdge(matching, uidx, vidx, weight);
        }
    }

    free(u);
    free(v);
    free(w);
    free(match_arr);

    return matching;
}

// -------------------- HIERHOLZER EULERIAN TOUR --------------------
unsigned int* FindEulerianTour(Graph* g, unsigned int* tourSize) {
    unsigned int N = GraphGetNumVertices(g);
    unsigned int maxSize = 4 * N * N;
    unsigned int* tour = malloc(maxSize * sizeof(unsigned int));
    if (!tour) { *tourSize = 0; return NULL; }

    // Use one N*N array for symmetric edge tracking (i < j stores the count)
    int* edge_counter = calloc(N * N, sizeof(int)); 
    if (!edge_counter) { free(tour); *tourSize = 0; return NULL; }
    
    // --- 1. Initialize Edge Counter from the Graph ---
    for (unsigned int u = 0; u < N; u++) {
        unsigned int* neighbors = GraphGetAdjacentsTo(g, u);
        
        // neighbors[0] is the degree count
        for (unsigned int i = 1; i <= neighbors[0]; i++) {
            unsigned int v = neighbors[i];
            
            // CRITICAL: Only count the edge in the direction min -> max
            if (u < v) {
                edge_counter[u * N + v]++;
            }
        }
        free(neighbors);
    }
    
    // --- 2. Hierholzer Traversal ---
    unsigned int sp = 0, tp = 0;
    unsigned int* stack = malloc(maxSize * sizeof(unsigned int));
    if (!stack) { free(tour); free(edge_counter); *tourSize = 0; return NULL; }
    
    stack[sp++] = 0; // Start at vertex 0

    while (sp > 0) {
        unsigned int u = stack[sp-1];
        int next_v = -1;
        
        // Iterate over ALL possible neighbors (0 to N-1) to find an available edge.
        for (unsigned int v = 0; v < N; v++) {
            if (u == v) continue;
            
            // Determine the unique storage index (i < j)
            unsigned int i_idx = (u < v) ? u : v;
            unsigned int j_idx = (u < v) ? v : u;

            // Check the counter for the physical edge
            if (edge_counter[i_idx * N + j_idx] > 0) {
                next_v = v;
                break;
            }
        }

        if (next_v != -1) {
            unsigned int v = (unsigned int)next_v;
            
            // Consume the physical edge (u, v)
            unsigned int i_idx = (u < v) ? u : v;
            unsigned int j_idx = (u < v) ? v : u;
            edge_counter[i_idx * N + j_idx]--; 
            
            stack[sp++] = v;
        } else {
            // No more available edges from u, add to tour and backtrack
            tour[tp++] = u; 
            sp--; 
        }
    }

    // --- 3. Cleanup ---
    free(stack); 
    free(edge_counter);

    *tourSize = tp;
    return realloc(tour, tp * sizeof(unsigned int));
}

// Hierholzer on explicit symmetric edge-count matrix.
// edge_cnt is NxN, only entries with i < j are used (we store in that triangular form)
unsigned int* FindEulerianTourFromEdgeCounts(const int *edge_cnt_in, unsigned int N, unsigned int *tourSize) {
    // Make a mutable copy of the counts
    int *edge_cnt = malloc(N * N * sizeof(int));
    if (!edge_cnt) { *tourSize = 0; return NULL; }
    memcpy(edge_cnt, edge_cnt_in, N * N * sizeof(int));

    // Precompute degrees to know when a vertex still has incident edges
    unsigned int *deg = calloc(N, sizeof(unsigned int));
    if (!deg) { free(edge_cnt); *tourSize = 0; return NULL; }
    for (unsigned int i = 0; i < N; ++i) {
        unsigned int d = 0;
        for (unsigned int j = 0; j < N; ++j) {
            if (i == j) continue;
            unsigned int a = i < j ? i : j;
            unsigned int b = i < j ? j : i;
            d += (unsigned int)edge_cnt[a * N + b];
        }
        deg[i] = d;
    }

    // upper bound on number of visited vertices in Euler tour
    unsigned int maxSize = 2 * N * N + 4;
    unsigned int *stack = malloc(maxSize * sizeof(unsigned int));
    unsigned int *tour = malloc(maxSize * sizeof(unsigned int));
    if (!stack || !tour) { free(edge_cnt); free(deg); free(stack); free(tour); *tourSize = 0; return NULL; }

    unsigned int sp = 0, tp = 0;
    stack[sp++] = 0;

    while (sp > 0) {
        unsigned int u = stack[sp - 1];
        // find any neighbor v with a remaining edge
        unsigned int v_found = N; // sentinel: none
        for (unsigned int v = 0; v < N; ++v) {
            if (v == u) continue;
            unsigned int a = u < v ? u : v;
            unsigned int b = u < v ? v : u;
            if (edge_cnt[a * N + b] > 0) { v_found = v; break; }
        }

        if (v_found != N) {
            unsigned int v = v_found;
            unsigned int a = u < v ? u : v;
            unsigned int b = u < v ? v : u;
            // consume one parallel edge
            edge_cnt[a * N + b]--;
            // update degrees
            deg[u]--; deg[v]--;
            stack[sp++] = v;
        } else {
            // backtrack
            tour[tp++] = u;
            sp--;
        }
    }

    free(stack);
    free(edge_cnt);
    free(deg);

    // reverse tour to standard forward order (optionally)
    // (already backtrack order; many codebases keep it as-is and then shortcut works)
    // return trimmed array
    unsigned int *res = realloc(tour, tp * sizeof(unsigned int));
    *tourSize = tp;
    return res ? res : tour;
}

// -------------------- SHORTCUT TO HAMILTONIAN --------------------
double ShortcutEuler(const Graph* g, unsigned int* euler, unsigned int size, unsigned int* hamilton, unsigned int N) {
    int* visited = calloc(N, sizeof(int));
    unsigned int pos = 0;
    double cost = 0.0;
    unsigned int last = euler[0];
    hamilton[pos++] = last;
    visited[last] = 1;

    for (unsigned int i = 1; i < size; i++) {
        unsigned int cur = euler[i];
        if (!visited[cur]) {
            cost += GetEdgeWeight(g, last, cur);
            hamilton[pos++] = cur;
            visited[cur] = 1;
            last = cur;
        }
    }
    cost += GetEdgeWeight(g, last, hamilton[0]);
    hamilton[pos] = hamilton[0];
    free(visited);
    return cost;
}