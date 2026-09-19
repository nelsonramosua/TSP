// ISPO.c - "ISPO" discrete Particle Swarm Optimization for TSP (Wang, Mu & Zhu, 2013).
//
// O(ISPO_PARTICLES * ISPO_ITERATIONS * N^2).
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Based on: Xiaohua Wang, Aiqin Mu, Shisong Zhu, "ISPO: A New Way to Solve Traveling Salesman Problem", Intelligent Control and Automation, 2013 (SciRP).
//
// Despite the name, this is NOT the continuous "Intelligent Single Particle Optimizer" -- it is a DISCRETE PSO whose velocity is a "mobile sequence" of "mobile operators", plus a simulated-annealing neighbourhood step each iteration.
// The pieces (as far as the paper defines them):
//
//   * Mobile operator sv(node, k): move `node` k positions along the tour (k>0 back, k<0 forward).
//   * Mobile sequence: an ordered list of mobile operators = a particle's velocity.
//   * position update:  X <- X applied-with V            (apply the whole mobile sequence to the tour)
//   * velocity update:  V <- (w * V) ++ (c1*r1 * A) ++ (c2*r2 * B),
//        where A = difference(X -> pbest), B = difference(X -> gbest), "++" concatenates sequences, and "coeff * sequence" (Def. 6) RETAINS each operator with probability `coeff` (0..1).
//
// The paper leaves the "difference" operator informal; the natural construction is used here: the mobile operators that turn X into the target, i.e. for each target position t, move the node that belongs there into place (recording sv(node, t - currentIndex)).
// That is what buildDifference does.
//
// Verification note: the returned tour is a genuine permutation kept consistent throughout (every mobile operator is a permutation move), and its cost is recomputed from the distance matrix.

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/DistanceMatrix.h"
#include "../../headers/Metaheuristics.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// ---- mobile operators / mobile sequences -------------------------------------------------------

typedef struct { unsigned int node; int k; } MobileOp;              // move `node` by k positions
typedef struct { MobileOp* ops; unsigned int len, cap; } MobileSeq; // ordered list of operators (a velocity)

static int mseqInit(MobileSeq* s, unsigned int cap) {
    s->ops = malloc(cap * sizeof(MobileOp));
    s->len = 0; s->cap = cap;
    return s->ops != NULL;
}
static void mseqFree(MobileSeq* s) { free(s->ops); s->ops = NULL; s->len = s->cap = 0; }
static void mseqPush(MobileSeq* s, unsigned int node, int k) { if (s->len < s->cap) s->ops[s->len++] = (MobileOp){ node, k }; }

// moves `node` k positions in path[0..n-1] (remove & reinsert), clamped to the ends.
static void moveNode(unsigned int* path, unsigned int n, unsigned int node, int k) {
    if (k == 0) return;
    unsigned int i = 0;
    while (i < n && path[i] != node) i++;
    if (i == n) return; // not found (shouldn't happen)

    long t = (long)i + k;
    if (t < 0) t = 0;
    if (t > (long)n - 1) t = (long)n - 1;
    unsigned int target = (unsigned int)t;
    if (target == i) return;

    if (target > i) {                        // shift the gap left, node moves back
        for (unsigned int p = i; p < target; p++) path[p] = path[p + 1];
    } else {                                 // shift right, node moves forward
        for (unsigned int p = i; p > target; p--) path[p] = path[p - 1];
    }
    path[target] = node;
}

// applies every operator of `s` to path in order (the position-update "X applied-with V").
static void applySeq(unsigned int* path, unsigned int n, const MobileSeq* s) {
    for (unsigned int i = 0; i < s->len; i++) moveNode(path, n, s->ops[i].node, s->ops[i].k);
}

// out <- the mobile sequence that turns `from` into `to`. work is scratch of n uints.
static void buildDifference(const unsigned int* from, const unsigned int* to, unsigned int n,
                            MobileSeq* out, unsigned int* work) {
    out->len = 0;
    for (unsigned int i = 0; i < n; i++) work[i] = from[i];
    for (unsigned int t = 0; t < n; t++) {
        unsigned int node = to[t];
        unsigned int i = t;
        while (i < n && work[i] != node) i++; // node currently sits at index i (>= t, earlier ones are fixed)
        if (i == n) continue;
        if (i != t) {
            mseqPush(out, node, (int)t - (int)i);
            moveNode(work, n, node, (int)t - (int)i); // now `node` is at t; positions < t are settled
        }
    }
}

// appends each operator of `src` to `dst`, retaining it with probability `prob` (Def. 6 scaling).
static void appendScaled(MobileSeq* dst, const MobileSeq* src, double prob) {
    for (unsigned int i = 0; i < src->len; i++)
        if ((double)rand() / RAND_MAX < prob) mseqPush(dst, src->ops[i].node, src->ops[i].k);
}

// ---- tour helpers ------------------------------------------------------------------------------

static double tourCost(const double* D, unsigned int n, const unsigned int* path) {
    double c = 0.0;
    for (unsigned int i = 0; i < n; i++) {
        double w = DIST_AT(D, n, path[i], path[(i + 1) % n]);
        if (w == DBL_MAX) return DBL_MAX;
        c += w;
    }
    return c;
}

static void randomPermutation(unsigned int* path, unsigned int n) {
    for (unsigned int i = 0; i < n; i++) path[i] = i;
    for (unsigned int i = n; i > 1; i--) { // Fisher-Yates
        unsigned int j = (unsigned int)(rand() % (int)i);
        unsigned int tmp = path[i - 1]; path[i - 1] = path[j]; path[j] = tmp;
    }
}

// The paper's hybrid uses a simulated-annealing neighbourhood to intensify the swarm's best; here, that role is a full first-improvement 2-Opt descent (a stronger, deterministic local search), which is what makes the method competitive -- plain discrete PSO drifts badly on larger tours.
// Drives `path` to a 2-Opt local optimum in place, keeping *cost in sync.
// Needs n >= 4.
static void twoOptDescent(const double* D, unsigned int n, unsigned int* path, double* cost) {
    if (n < 4) return;
    int improved = 1;
    while (improved) {
        improved = 0;
        for (unsigned int i = 0; i + 1 < n; i++) {
            for (unsigned int j = i + 1; j < n; j++) {
                if (i == 0 && j == n - 1) continue; // reversing the whole tour is a no-op
                unsigned int a = path[i], b = path[i + 1], c = path[j], d = path[(j + 1) % n];
                double before = DIST_AT(D, n, a, b) + DIST_AT(D, n, c, d);
                double after  = DIST_AT(D, n, a, c) + DIST_AT(D, n, b, d);
                if (before == DBL_MAX || after == DBL_MAX) continue;
                if (after - before < -DBL_EPSILON) {
                    for (unsigned int lo = i + 1, hi = j; lo < hi; lo++, hi--) { unsigned int t = path[lo]; path[lo] = path[hi]; path[hi] = t; }
                    *cost += after - before;
                    improved = 1;
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------

Tour* ISPO_FindTour(const Graph* g) {
    unsigned int n = GraphGetNumVertices(g);
    if (n < 2) return NULL;

    unsigned int P = ISPO_PARTICLES;
    unsigned int vcap = 3 * n + 4; // velocity length is bounded (w<1 shrinks the old velocity each step)

    double* D = DistanceMatrixBuild(g);
    unsigned int* X = malloc((size_t)P * n * sizeof(unsigned int));       // particle positions
    unsigned int* pbest = malloc((size_t)P * n * sizeof(unsigned int));   // personal bests
    double* pbestCost = malloc(P * sizeof(double));
    unsigned int* gbest = malloc(n * sizeof(unsigned int));
    unsigned int* work = malloc(n * sizeof(unsigned int));                // scratch for differences
    MobileSeq* V = malloc(P * sizeof(MobileSeq));                         // per-particle velocities
    MobileSeq A = {0}, B = {0}, newV = {0};                               // reusable scratch sequences
    int seqOk = V != NULL;
    for (unsigned int p = 0; seqOk && p < P; p++) seqOk = mseqInit(&V[p], vcap);
    seqOk = seqOk && mseqInit(&A, n + 1) && mseqInit(&B, n + 1) && mseqInit(&newV, vcap);

    if (!D || !X || !pbest || !pbestCost || !gbest || !work || !seqOk) {
        if (V) { for (unsigned int p = 0; p < P; p++) mseqFree(&V[p]); }
        mseqFree(&A); mseqFree(&B); mseqFree(&newV);
        free(D); free(X); free(pbest); free(pbestCost); free(gbest); free(work); free(V);
        return NULL;
    }

    // initialize swarm: empty velocities, pbest = self, gbest = best.
    // A few particles are seeded with Nearest-Neighbour tours (from varied starts) rather than random permutations, so gbest starts at least as good as NN and the swarm has good basins to pull toward -- pure random init leaves discrete PSO drifting far from the optimum on larger instances.
    unsigned int seeded = 1; // just one NN seed -- more collapses swarm diversity and hurts small instances
    double gbestCost = DBL_MAX;
    for (unsigned int p = 0; p < P; p++) {
        unsigned int* xp = &X[(size_t)p * n];
        if (p < seeded) {
            Tour* nn = NearestNeighbour_FindTour(g, (unsigned int)(rand() % (int)n));
            if (nn) { for (unsigned int i = 0; i < n; i++) xp[i] = nn->path[i]; TourDestroy(&nn); }
            else randomPermutation(xp, n);
        } else {
            randomPermutation(xp, n);
        }
        V[p].len = 0;
        double c = tourCost(D, n, xp);
        for (unsigned int i = 0; i < n; i++) pbest[(size_t)p * n + i] = xp[i];
        pbestCost[p] = c;
        if (c < gbestCost) { gbestCost = c; for (unsigned int i = 0; i < n; i++) gbest[i] = xp[i]; }
    }

    twoOptDescent(D, n, gbest, &gbestCost); // start gbest at a local optimum

    unsigned int stagnation = 0;
    for (unsigned int iter = 0; iter < ISPO_ITERATIONS; iter++) {
        double w = ISPO_W_MAX - (ISPO_W_MAX - ISPO_W_MIN) * ((double)iter / ISPO_ITERATIONS);
        double gPrev = gbestCost;

        for (unsigned int p = 0; p < P; p++) {
            unsigned int* xp = &X[(size_t)p * n];

            // new velocity = (w * old V) ++ (c1*r1 * (X->pbest)) ++ (c2*r2 * (X->gbest))
            buildDifference(xp, &pbest[(size_t)p * n], n, &A, work);
            buildDifference(xp, gbest, n, &B, work);
            newV.len = 0;
            appendScaled(&newV, &V[p], w);
            appendScaled(&newV, &A, ISPO_C1 * ((double)rand() / RAND_MAX));
            appendScaled(&newV, &B, ISPO_C2 * ((double)rand() / RAND_MAX));
            V[p].len = 0;
            appendScaled(&V[p], &newV, 1.0); // copy newV into V[p] (bounded by vcap)

            // position update, then intensify to a local optimum (memetic step -- the paper's hybrid neighbourhood, applied per particle so the swarm searches over 2-Opt optima rather than raw random tours that never rival gbest).
            applySeq(xp, n, &V[p]);
            double c = tourCost(D, n, xp);
            twoOptDescent(D, n, xp, &c);

            if (c < pbestCost[p]) {
                pbestCost[p] = c;
                for (unsigned int i = 0; i < n; i++) pbest[(size_t)p * n + i] = xp[i];
            }
            if (c < gbestCost) {
                gbestCost = c;
                for (unsigned int i = 0; i < n; i++) gbest[i] = xp[i];
            }
        }

        // re-initialize velocities if gbest has stalled for m iterations (the paper's restart rule)
        if (gbestCost < gPrev - DBL_EPSILON) stagnation = 0;
        else if (++stagnation >= ISPO_STAGNATION) { for (unsigned int p = 0; p < P; p++) V[p].len = 0; stagnation = 0; }
    }

    Tour* tour = NULL;
    if (gbestCost != DBL_MAX) {
        tour = TourCreate(n);
        if (tour) {
            for (unsigned int i = 0; i < n; i++) tour->path[i] = gbest[i];
            tour->path[n] = tour->path[0];
            tour->cost = gbestCost;
        }
    } else {
        fprintf(stderr, "Warning: ISPO found no valid tour (graph disconnected?).\n");
    }

    for (unsigned int p = 0; p < P; p++) mseqFree(&V[p]);
    mseqFree(&A); mseqFree(&B); mseqFree(&newV);
    free(D); free(X); free(pbest); free(pbestCost); free(gbest); free(work); free(V);
    return tour;
}