// Blossom.cpp - implements blossom algorithm for Christofides.c
//
// O(N^3)
//
// All credit to https://github.com/suddhabrato/edmonds-blossom-algorithm, I adapted from it.
//
// November 2025.

#include <bits/stdc++.h>
#include "Blossom.hpp"
using namespace std;

const int MAXN = 500;
const double INF = 1e18;

struct Edge {
    int u, v;
    double w;
};

class WeightedBlossom {
    int N; // number of vertices
    double cost[MAXN][MAXN]; // adjacency matrix of weights
    int match[MAXN], base[MAXN], p[MAXN];
    bool in_blossom[MAXN], in_queue[MAXN];
    queue<int> q;

public:
    WeightedBlossom(int n) : N(n) {
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                cost[i][j] = INF;
        fill(match, match + N, -1);
    }

    void addEdge(int u, int v, double w) {
        cost[u][v] = min(cost[u][v], w);
        cost[v][u] = cost[u][v];
    }

    int LCA(int a, int b, vector<int>& mark) {
        static int t = 0;
        t++;
        while (true) {
            if (a != -1) {
                if (mark[a] == t) return a;
                mark[a] = t;
                if (match[a] == -1) a = -1;
                else a = base[p[match[a]]];
            }
            swap(a,b);
        }
    }

    void markPath(int v, int b, int child, vector<int>& blossom) {
        while (base[v] != b) {
            blossom[base[v]] = blossom[base[match[v]]] = true;
            p[v] = child;
            child = match[v];
            v = p[match[v]];
        }
    }

    int findAugmentingPath(int root) {
        vector<int> par(N, -1);
        vector<int> vis(N, 0);
        for (int i = 0; i < N; i++) base[i] = i;

        q = queue<int>();
        q.push(root);
        vis[root] = 1;

        vector<int> mark(N,0);

        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v = 0; v < N; v++) {
                if (cost[u][v] == INF || base[u] == base[v] || match[u] == v) continue;
                if (v == root || (match[v] != -1 && par[match[v]] != -1)) {
                    int b = LCA(u,v,mark);
                    vector<int> blossom(N,0);
                    markPath(u,b,v,blossom);
                    markPath(v,b,u,blossom);
                    for(int i=0;i<N;i++)
                        if(blossom[base[i]]) {
                            base[i]=b;
                            if(!vis[i]) { vis[i]=1; q.push(i); }
                        }
                } else if (par[v] == -1) {
                    par[v] = u;
                    if (match[v] == -1) return v;
                    vis[match[v]] = 1;
                    q.push(match[v]);
                }
            }
        }
        return -1;
    }

    void augment(int t) {
        while (t != -1) {
            int pv = p[t], ppv = match[pv];
            match[t] = pv; match[pv] = t;
            t = ppv;
        }
    }

    double solve() {
        for (int u = 0; u < N; u++) {
            if (match[u] == -1) {
                int t = findAugmentingPath(u);
                if(t != -1) augment(t);
            }
        }

        double total = 0;
        for(int i=0;i<N;i++)
            if(match[i]!=-1 && i<match[i]) total += cost[i][match[i]];
        return total;
    }

    void printMatching() {
        for(int i=0;i<N;i++)
            if(match[i]!=-1 && i<match[i])
                cout<<i<<" "<<match[i]<<" cost="<<cost[i][match[i]]<<"\n";
    }
};