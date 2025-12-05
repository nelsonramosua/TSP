// LowerBound_HeldKarp.c - Held-Karp Lagrangian relaxation lower bound for TSP.
//
// O(N^2).
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Resources used:
// https://www.sciencedirect.com/science/article/pii/S0377221796002147
// "Transcription"!! of the C++ algorithm here: https://github.com/corail-research/learning-hk-bound/blob/main/solver/src/hk2.cc
// In fact, I cannot say I fully understand the algorithm... :')

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/LowerBounds.h"

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>

// Tune configuration in headers/LowerBounds.h.

static int computeAdjustedOneTree(const Graph* g, const double* pi, unsigned int root, double* outCostOneTree, unsigned int* degrees);

unsigned int root = FIXED_START_ROOT; // fixed root for 1-tree construction.

// calcs Held-Karp lower bound
double LowerBound_HeldKarp(const Graph* g) {
    if (!g) return -1.0;
    if (!GraphIsWeighted(g) || GraphIsDigraph(g)) {
        fprintf(stderr, "Error: Held-Karp requires a weighted, undirected graph.\n");
        return -1.0;
    }

    double bestDual = -DBL_MAX;           

    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices <= 1) return 0.0;

    // init mults pi = 0
    double* pi = calloc(numVertices, sizeof(double));
    unsigned int* degrees = malloc(numVertices * sizeof(unsigned int));
    if (!pi || !degrees) { free(pi); free(degrees); return -1.0; }

    for (unsigned int iter = 0; iter < MAX_ITERS; iter++) {
        double adjustedCost;
        int ok = computeAdjustedOneTree(g, pi, root, &adjustedCost, degrees);

        if (!ok || adjustedCost >= DBL_MAX / 2.0) {
            // discon. w/ adj weights - small rand perturbation
            for (unsigned int i = 0; i < numVertices; i++) pi[i] += ((double)rand() / RAND_MAX - 0.5) * 1e-3;
            continue;
        }

        // L(pi) = cost(1-tree) - 2 * sum(pi)
        double sumPi = 0.0;
        for (unsigned int i = 0; i < numVertices; i++) sumPi += pi[i];

        double Lpi = adjustedCost - 2.0 * sumPi;

        if (Lpi > bestDual) bestDual = Lpi;

        // subgradient s_i = degree_i - 2
        double* subGradient = malloc(numVertices * sizeof(double));
        if (!subGradient) break;

        double sqNorm = 0.0;

        for (unsigned int i = 0; i < numVertices; i++) {
            subGradient[i] = (double)((int)degrees[i] - 2);
            sqNorm += subGradient[i] * subGradient[i];
        }

        // all degrees = 2 
        if (sqNorm == 0.0) { free(subGradient); break; }

        // diminishing step size
        double stepSize = STEP0 / sqrt((double)(iter + 1));
        double denom = 1.0 + sqNorm;

        for (unsigned int i = 0; i < numVertices; i++) pi[i] += (stepSize * subGradient[i]) / denom;

        free(subGradient);
    }

    free(pi);
    free(degrees);

    if (!isfinite(bestDual)) return -1.0;

    return bestDual;
}

// calc 1-tree on adjusted weights w'(i,j) = original + pi[i] + pi[j].
static int computeAdjustedOneTree(const Graph* g, const double* pi, unsigned int root, double* outCostOneTree, unsigned int* degrees) {
    unsigned int numVertices = GraphGetNumVertices(g);

    if (numVertices == 0) { *outCostOneTree = 0.0; return 1; }

    // reset degrees
    for (unsigned int i = 0; i < numVertices; i++) degrees[i] = 0;

    if (numVertices == 1) { *outCostOneTree = 0.0; degrees[0] = 0; return 1; }

    double* key = malloc(numVertices * sizeof(double));
    int* inMst = malloc(numVertices * sizeof(int));
    int* parent = malloc(numVertices * sizeof(int));

    if (!key || !inMst || !parent) { free(key); free(inMst); free(parent); return 0; }

    for (unsigned int i = 0; i < numVertices; i++) { key[i] = DBL_MAX; inMst[i] = 0; parent[i] = -1; }

    // start vertex != root
    unsigned int start = (root == 0) ? 1 : 0;
    key[start] = 0.0;

    unsigned int verticesToAdd = numVertices - 1; 
    double mstCostAdjusted = 0.0;

    // Prim MST excluding root
    for (unsigned int step = 0; step < verticesToAdd; step++) {
        double best = DBL_MAX;
        int u = -1;

        for (unsigned int v = 0; v < numVertices; v++) {
            if (v == root) continue;
            if (!inMst[v] && key[v] < best) { best = key[v]; u = (int)v; }
        }

        if (u == -1) { free(key); free(inMst); free(parent); *outCostOneTree = DBL_MAX; return 0; }

        inMst[u] = 1;

        if (parent[u] != -1) mstCostAdjusted += key[u];

        for (unsigned int v = 0; v < numVertices; v++) {
            if (v == root) continue;
            if (inMst[v]) continue;

            double w = GetEdgeWeight(g, (unsigned int)u, v);
            if (w == DBL_MAX) continue;

            double wAdj = w + pi[u] + pi[v];
            if (wAdj < key[v]) { key[v] = wAdj; parent[v] = u; }
        }
    }

    // build degrees from MST
    unsigned int* localDegrees = calloc(numVertices, sizeof(unsigned int));
    if (!localDegrees) { free(key); free(inMst); free(parent); return 0; }

    for (unsigned int v = 0; v < numVertices; v++) {
        if (v == root) continue;
        if (parent[v] != -1) { localDegrees[v] += 1; localDegrees[parent[v]] += 1; }
    }

    // find two smallest edges incident to root
    double best1 = DBL_MAX, best2 = DBL_MAX;
    int best1v = -1, best2v = -1;

    for (unsigned int v = 0; v < numVertices; v++) {
        if (v == root) continue;

        double w = GetEdgeWeight(g, root, v);
        if (w == DBL_MAX) continue;

        double wAdj = w + pi[root] + pi[v];

        if (wAdj < best1) { 
            best2 = best1; best2v = best1v; 
            best1 = wAdj; best1v = (int)v; 
        }
        else if (wAdj < best2) { best2 = wAdj; best2v = (int)v; }
    }

    if (best1 == DBL_MAX || best2 == DBL_MAX) {
        free(key); free(inMst); free(parent); free(localDegrees);
        *outCostOneTree = DBL_MAX;
        return 0;
    }

    double costOneTreeAdjusted = mstCostAdjusted + best1 + best2;

    // final degrees
    for (unsigned int i = 0; i < numVertices; i++) degrees[i] = localDegrees[i];

    degrees[root] += 2;
    degrees[best1v] += 1;
    degrees[best2v] += 1;

    free(key); free(inMst); free(parent); free(localDegrees);

    *outCostOneTree = costOneTreeAdjusted;
    return 1;
}