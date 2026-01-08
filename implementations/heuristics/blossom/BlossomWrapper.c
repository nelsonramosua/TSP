// BlossomWrapper.c - Minimum Weight Perfect Matching.
//
// O(n³) --- implementation of Edmonds' weighted Blossom algorithm using the primal-dual method.
//
// Based on the algorithm described in "Combinatorial Optimization" by Korte & Vygen.
// https://www.mathematik.uni-muenchen.de/~kpanagio/KombOpt/book.pdf
//
// January 2026.

// near transcription... so no props to me.

#include "BlossomWrapper.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>

#define INF 1e18
#define EPS 1e-9

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SWAP(a, b) do { int _t = (a); (a) = (b); (b) = _t; } while(0)

typedef struct {
    int n;
    double* cost;       // n x n matrix (flattened)
    int* match;
    int* slack_id;
    double* lab;        // Dual variables (labels/potentials)
    int* blossom;
    int* vis;
    int* par;
    
    // Queue (simple array-based)... could have created auxADT.
    int* queue;
    int qHead, qTail;
    
    // LCA marking
    int* mark;
    int timer;
} WeightedBlossom;

static inline double getCost(WeightedBlossom* bm, int u, int v) {
    return bm->cost[u * bm->n + v];
}

static inline void setCost(WeightedBlossom* bm, int u, int v, double w) {
    bm->cost[u * bm->n + v] = w;
}

static inline double reducedCost(WeightedBlossom* bm, int u, int v) {
    return getCost(bm, u, v) - bm->lab[u] - bm->lab[v];
}

static void updateSlack(WeightedBlossom* bm, int u, int v) {
    if (bm->slack_id[v] == -1 || reducedCost(bm, u, v) < reducedCost(bm, bm->slack_id[v], v) - EPS) {
        bm->slack_id[v] = u;
    }
}

static int getBase(WeightedBlossom* bm, int v) {
    if (bm->blossom[v] == v) return v;
    bm->blossom[v] = getBase(bm, bm->blossom[v]);
    return bm->blossom[v];
}

static void setSlack(WeightedBlossom* bm, int v) {
    bm->slack_id[v] = -1;
    for (int u = 0; u < bm->n; u++) {
        if (getCost(bm, u, v) < INF / 2 && bm->vis[u] == 1 && getBase(bm, u) != getBase(bm, v)) {
            updateSlack(bm, u, v);
        }
    }
}

static void qPush(WeightedBlossom* bm, int u) {
    if (bm->vis[u] != 1) {
        bm->vis[u] = 1;
        bm->queue[bm->qTail++] = u;
    }
}

static int qPop(WeightedBlossom* bm) {
    return bm->queue[bm->qHead++];
}

static int qEmpty(WeightedBlossom* bm) {
    return bm->qHead >= bm->qTail;
}

static int getLCA(WeightedBlossom* bm, int u, int v) {
    bm->timer++;
    u = getBase(bm, u);
    v = getBase(bm, v);
    
    while (1) {
        if (u != -1) {
            if (bm->mark[u] == bm->timer) return u;
            bm->mark[u] = bm->timer;
            u = (bm->match[u] == -1) ? -1 : getBase(bm, bm->par[bm->match[u]]);
        }
        SWAP(u, v);
    }
}

static void contract(WeightedBlossom* bm, int u, int v, int b) {
    while (getBase(bm, u) != b) {
        bm->par[u] = v;
        v = bm->match[u];
        if (bm->vis[v] == 2) qPush(bm, v);
        bm->blossom[u] = bm->blossom[v] = b;
        u = bm->par[v];
    }
}

static void augment(WeightedBlossom* bm, int u) {
    while (u != -1) {
        int pv = bm->par[u];
        int ppv = bm->match[pv];
        bm->match[u] = pv;
        bm->match[pv] = u;
        u = ppv;
    }
}

static int bfs(WeightedBlossom* bm, int root) {
    int n = bm->n;
    
    // Reset state
    memset(bm->vis, 0, n * sizeof(int));
    for (int i = 0; i < n; i++) {
        bm->slack_id[i] = -1;
        bm->par[i] = -1;
        bm->blossom[i] = i;
    }
    
    bm->qHead = bm->qTail = 0;
    qPush(bm, root);
    
    while (1) {
        // Process S-vertices in queue
        while (!qEmpty(bm)) {
            int u = qPop(bm);
            
            for (int v = 0; v < n; v++) {
                if (getCost(bm, u, v) >= INF / 2) continue;
                if (getBase(bm, u) == getBase(bm, v)) continue;
                
                double rc = reducedCost(bm, u, v);
                if (rc > EPS) {
                    updateSlack(bm, u, v);
                    continue;
                }
                
                // Edge is tight
                if (bm->vis[v] == 0) {
                    if (bm->match[v] == -1) {
                        bm->par[v] = u;
                        return v;  // Found augmenting path
                    }
                    bm->vis[v] = 2;
                    bm->par[v] = u;
                    qPush(bm, bm->match[v]);
                } else if (bm->vis[v] == 1) {
                    // Form blossom
                    int b = getLCA(bm, u, v);
                    contract(bm, u, v, b);
                    contract(bm, v, u, b);
                    for (int w = 0; w < n; w++) {
                        if (bm->vis[w] == 0) setSlack(bm, w);
                    }
                }
            }
        }
        
        // Compute delta
        double delta = INF;
        for (int v = 0; v < n; v++) {
            if (bm->vis[v] == 0 && bm->slack_id[v] != -1) {
                delta = MIN(delta, reducedCost(bm, bm->slack_id[v], v));
            }
        }
        
        if (delta >= INF / 2) return -1;  // No perfect matching exists
        
        // Update dual variables
        for (int v = 0; v < n; v++) {
            if (bm->vis[v] == 1) bm->lab[v] += delta;
            else if (bm->vis[v] == 2) bm->lab[v] -= delta;
        }
        
        // Check for newly tight edges
        for (int v = 0; v < n; v++) {
            if (bm->vis[v] != 0 || bm->slack_id[v] == -1) continue;
            if (reducedCost(bm, bm->slack_id[v], v) < EPS) {
                int u = bm->slack_id[v];
                if (bm->match[v] == -1) {
                    bm->par[v] = u;
                    return v;
                }
                bm->vis[v] = 2;
                bm->par[v] = u;
                qPush(bm, bm->match[v]);
            }
        }
    }
}

static WeightedBlossom* createBlossom(int n) {
    WeightedBlossom* bm = malloc(sizeof(WeightedBlossom));
    if (!bm) return NULL;
    
    bm->n = n;
    bm->cost = malloc(n * n * sizeof(double));
    bm->match = malloc(n * sizeof(int));
    bm->slack_id = malloc(n * sizeof(int));
    bm->lab = malloc(n * sizeof(double));
    bm->blossom = malloc(n * sizeof(int));
    bm->vis = malloc(n * sizeof(int));
    bm->par = malloc(n * sizeof(int));
    bm->queue = malloc(n * sizeof(int));
    bm->mark = malloc(n * sizeof(int));
    
    if (!bm->cost || !bm->match || !bm->slack_id || !bm->lab || 
        !bm->blossom || !bm->vis || !bm->par || !bm->queue || !bm->mark) {
        free(bm->cost); free(bm->match); free(bm->slack_id); free(bm->lab);
        free(bm->blossom); free(bm->vis); free(bm->par); free(bm->queue); free(bm->mark);
        free(bm);
        return NULL;
    }
    
    // Initialize
    for (int i = 0; i < n * n; i++) bm->cost[i] = INF;
    for (int i = 0; i < n; i++) {
        bm->match[i] = -1;
        bm->slack_id[i] = -1;
        bm->lab[i] = 0;
        bm->blossom[i] = i;
        bm->vis[i] = 0;
        bm->par[i] = -1;
        bm->mark[i] = 0;
    }
    bm->qHead = bm->qTail = 0;
    bm->timer = 0;
    
    return bm;
}

static void destroyBlossom(WeightedBlossom* bm) {
    if (!bm) return;
    free(bm->cost);
    free(bm->match);
    free(bm->slack_id);
    free(bm->lab);
    free(bm->blossom);
    free(bm->vis);
    free(bm->par);
    free(bm->queue);
    free(bm->mark);
    free(bm);
}

static void addEdge(WeightedBlossom* bm, int u, int v, double w) {
    if (u != v && w < getCost(bm, u, v)) {
        setCost(bm, u, v, w);
        setCost(bm, v, u, w);
    }
}

static void solve(WeightedBlossom* bm) {
    int n = bm->n;
    if (n == 0 || n % 2 != 0) return;
    
    // Initialize dual variables: lab[v] = min edge weight / 2
    for (int i = 0; i < n; i++) {
        double minEdge = INF;
        for (int j = 0; j < n; j++) {
            if (i != j && getCost(bm, i, j) < minEdge) {
                minEdge = getCost(bm, i, j);
            }
        }
        bm->lab[i] = minEdge / 2.0;
    }
    
    // Find augmenting paths for each unmatched vertex
    for (int u = 0; u < n; u++) {
        if (bm->match[u] == -1) {
            int v = bfs(bm, u);
            if (v != -1) augment(bm, v);
        }
    }
}

void blossomMWPM(int N, int E, int* u, int* v, double* w, int* match_out) {
    WeightedBlossom* bm = createBlossom(N);
    if (!bm) {
        for (int i = 0; i < N; i++) match_out[i] = -1;
        return;
    }
    
    for (int i = 0; i < E; i++) addEdge(bm, u[i], v[i], w[i]);
    
    solve(bm);
    
    for (int i = 0; i < N; i++) match_out[i] = bm->match[i];
    
    destroyBlossom(bm);
}