#include "BlossomWrapper.h"
#include <vector>
#include <algorithm>
#include <limits>
using namespace std;

struct WeightedBlossom {
    int N;
    vector<vector<double>> cost;
    vector<int> match;

    WeightedBlossom(int n) : N(n), cost(n, vector<double>(n, 1e18)), match(n, -1) {}

    void addEdge(int a, int b, double w) {
        if (a != b) cost[a][b] = min(cost[a][b], w);
        if (a != b) cost[b][a] = cost[a][b];
    }

    void solve() {
        // Simple greedy for MWPM (replace with full Edmonds if needed)
        for (int u = 0; u < N; u++) {
            if (match[u] != -1) continue;
            double best = 1e18;
            int best_v = -1;
            for (int v = 0; v < N; v++) {
                if (match[v] == -1 && cost[u][v] < best) {
                    best = cost[u][v];
                    best_v = v;
                }
            }
            if (best_v != -1) match[u] = best_v, match[best_v] = u;
        }
    }
};

extern "C" void blossomMWPM(int N, int E, int* u, int* v, double* w, int* match_out) {
    WeightedBlossom bm(N);
    for (int i = 0; i < E; i++) {
        bm.addEdge(u[i], v[i], w[i]);
    }
    bm.solve();
    for (int i = 0; i < N; i++)
        match_out[i] = bm.match[i];
}