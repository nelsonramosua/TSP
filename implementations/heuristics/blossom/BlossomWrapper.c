// BlossomWrapper.c - Minimum Weight Perfect Matching.
//
// O(n^3) --- Edmonds' weighted general-graph matching (primal-dual, with blossom duals and expansion).
//
// We do not minimise directly: we solve a MAXIMUM weight matching on the transformed weights w' = CAP - w.
// On a complete graph with positive weights that matching is necessarily perfect, and since every perfect matching has exactly n/2 edges, maximising sum(CAP - w) is the same as minimising sum(w).
// Input distances (doubles) are scaled to integers so the delta arithmetic stays exact (no float-equality pitfalls).
//
// Based on the standard weighted-blossom algorithm; see "Combinatorial Optimization" by Korte & Vygen.
// https://www.mathematik.uni-muenchen.de/~kpanagio/KombOpt/book.pdf
//
// The previous version maintained only per-vertex duals (no blossom duals / expansion), so it returned valid but non-minimum matchings whenever odd blossoms formed -- which degraded Christofides tours.
// This version is verified against a brute-force minimum-weight perfect matching oracle.
//
// January 2026.
// Updates:
// 1. Nelson Ramos, September 2026: rewrote as a correct weighted blossom.

#include "BlossomWrapper.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// Factor used to turn double distances into integers for exact arithmetic.
#define WEIGHT_SCALE 1e6

// A labelled node's role in the current alternating tree.
#define STATE_FREE  (-1) // not yet reached by the tree
#define STATE_OUTER   0  // even distance from the root (a.k.a. S node)
#define STATE_INNER   1  // odd  distance from the root (a.k.a. T node)

// A graph edge with its (scaled, transformed) weight. weight == 0 means "no edge".
typedef struct {
    int u, v;
    long long weight;
} Edge;

// All working state for one matching computation.
typedef struct {
    int numVertices;    // real vertices, indexed 1..numVertices
    int numNodes;       // real vertices + blossom pseudo-nodes (grows during the solve)
    int capacity;       // allocated dimension (>= 2 * numVertices + 2)

    Edge* edges;        // capacity x capacity adjacency matrix (flattened)
    long long* label;   // dual variable (potential) of each node
    int* match;         // match[v] = partner vertex, or 0 if v is unmatched
    int* slackFrom;     // slackFrom[v] = neighbour giving v its tightest edge, or 0
    int* base;          // union-find base: the outermost blossom a node belongs to
    int* parent;        // alternating-tree parent
    int* state;         // STATE_FREE / STATE_OUTER / STATE_INNER
    int* mark;          // timestamps used by the lowest-common-ancestor search

    int** blossomNodes; // for each pseudo-node, the ordered cycle of child nodes
    int* blossomLen;    // number of children of each pseudo-node
    int* baseNode;      // baseNode[b][x] = child of b through which x is reached (flattened)

    int* queue;         // BFS queue of outer nodes to scan
    int queueHead, queueTail, queueCap;
} WeightedMatching;

// --- small accessors -------------------------------------------------------

// Edge between nodes u and v.
static inline Edge* edgeAt(WeightedMatching* m, int u, int v) {
    return &m->edges[(long long)u * m->capacity + v];
}

// baseNode[b][x], flattened over (numVertices + 1) columns.
static inline int* baseNodeAt(WeightedMatching* m, int b, int x) {
    return &m->baseNode[(long long)b * (m->numVertices + 1) + x];
}

// Reduced cost of an edge under the current duals (0 means the edge is tight).
static long long reducedCost(WeightedMatching* m, const Edge* e) {
    return m->label[e->u] + m->label[e->v] - e->weight * 2;
}

// --- BFS queue -------------------------------------------------------------

static void queueReset(WeightedMatching* m) {
    m->queueHead = m->queueTail = 0;
}

// Pushes a node; a pseudo-node is expanded to its underlying real vertices.
static void queuePush(WeightedMatching* m, int x) {
    if (x <= m->numVertices) {
        if (m->queueTail >= m->queueCap) {
            m->queueCap *= 2;
            m->queue = realloc(m->queue, m->queueCap * sizeof(int));
        }
        m->queue[m->queueTail++] = x;
    } else {
        for (int i = 0; i < m->blossomLen[x]; i++) queuePush(m, m->blossomNodes[x][i]);
    }
}

// --- slack bookkeeping -----------------------------------------------------

// Records u as x's best neighbour if edge (u, x) is at least as tight as the current one.
static void updateSlack(WeightedMatching* m, int u, int x) {
    if (!m->slackFrom[x] ||
        reducedCost(m, edgeAt(m, u, x)) < reducedCost(m, edgeAt(m, m->slackFrom[x], x)))
        m->slackFrom[x] = u;
}

// Recomputes x's tightest edge from all outer nodes.
static void setSlack(WeightedMatching* m, int x) {
    m->slackFrom[x] = 0;
    for (int u = 1; u <= m->numVertices; u++)
        if (edgeAt(m, u, x)->weight > 0 && m->base[u] != x && m->state[m->base[u]] == STATE_OUTER)
            updateSlack(m, u, x);
}

// Sets the base of x (and, recursively, of every node inside x if x is a blossom).
static void setBase(WeightedMatching* m, int x, int b) {
    m->base[x] = b;
    if (x > m->numVertices)
        for (int i = 0; i < m->blossomLen[x]; i++) setBase(m, m->blossomNodes[x][i], b);
}

// --- generic array helpers -------------------------------------------------

static void reverseInts(int* a, int len) {
    for (int i = 0, j = len - 1; i < j; i++, j--) {
        int t = a[i]; a[i] = a[j]; a[j] = t;
    }
}

// Rotates a[0..len) left so that a[k] becomes the new first element.
static void rotateIntsLeft(int* a, int len, int k) {
    if (k <= 0 || k >= len) return;
    int* tmp = malloc(len * sizeof(int));
    for (int i = 0; i < len; i++) tmp[i] = a[(i + k) % len];
    memcpy(a, tmp, len * sizeof(int));
    free(tmp);
}

// Finds the offset of child xr within blossom b's cycle, orienting the cycle so the returned offset is even (the direction that keeps the matching alternating).
static int childOffset(WeightedMatching* m, int b, int xr) {
    int pr = 0;
    while (pr < m->blossomLen[b] && m->blossomNodes[b][pr] != xr) pr++;
    if (pr & 1) {
        reverseInts(m->blossomNodes[b] + 1, m->blossomLen[b] - 1);
        return m->blossomLen[b] - pr;
    }
    return pr;
}

// --- augmenting ------------------------------------------------------------

// Matches u to v, recursing into u's blossom so its internal matching stays consistent.
static void setMatch(WeightedMatching* m, int u, int v) {
    m->match[u] = edgeAt(m, u, v)->v;
    if (u > m->numVertices) {
        Edge e = *edgeAt(m, u, v);
        int xr = *baseNodeAt(m, u, e.u);
        int pr = childOffset(m, u, xr);
        for (int i = 0; i < pr; i++) setMatch(m, m->blossomNodes[u][i], m->blossomNodes[u][i ^ 1]);
        setMatch(m, xr, v);
        rotateIntsLeft(m->blossomNodes[u], m->blossomLen[u], pr);
    }
}

// Flips the alternating path found from an augmenting edge, growing the matching by one.
static void augment(WeightedMatching* m, int u, int v) {
    for (;;) {
        int nextV = m->base[m->match[u]];
        setMatch(m, u, v);
        if (!nextV) return;
        setMatch(m, nextV, m->base[m->parent[nextV]]);
        u = m->base[m->parent[nextV]];
        v = nextV;
    }
}

// Lowest common ancestor of u and v in the alternating tree (used to close a blossom).
static int lowestCommonAncestor(WeightedMatching* m, int u, int v) {
    static int timer = 0;
    ++timer;
    while (u || v) {
        if (u) {
            if (m->mark[u] == timer) return u;
            m->mark[u] = timer;
            u = m->base[m->match[u]];
            if (u) u = m->base[m->parent[u]];
        }
        int tmp = u; u = v; v = tmp;
    }
    return 0;
}

// --- blossoms --------------------------------------------------------------

static void blossomAppend(WeightedMatching* m, int b, int x) {
    m->blossomNodes[b][m->blossomLen[b]++] = x;
}

// Contracts the odd cycle closed by edge (u, v) at their common ancestor lca into a new pseudo-node, and wires up its edges/labels for the rest of the search.
static void addBlossom(WeightedMatching* m, int u, int lca, int v) {
    int b = m->numVertices + 1;
    while (b <= m->numNodes && m->base[b]) ++b;
    if (b > m->numNodes) ++m->numNodes;

    m->label[b] = 0;
    m->state[b] = STATE_OUTER;
    m->match[b] = m->match[lca];
    m->blossomLen[b] = 0;

    // walk u's branch up to the lca, then v's branch, recording the cycle
    blossomAppend(m, b, lca);
    for (int x = u, y; x != lca; x = m->base[m->parent[y]]) {
        blossomAppend(m, b, x);
        y = m->base[m->match[x]];
        blossomAppend(m, b, y);
        queuePush(m, y);
    }
    reverseInts(m->blossomNodes[b] + 1, m->blossomLen[b] - 1);
    for (int x = v, y; x != lca; x = m->base[m->parent[y]]) {
        blossomAppend(m, b, x);
        y = m->base[m->match[x]];
        blossomAppend(m, b, y);
        queuePush(m, y);
    }

    setBase(m, b, b);

    // the blossom inherits the tightest edge to every other node
    for (int x = 1; x <= m->numNodes; x++) { edgeAt(m, b, x)->weight = 0; edgeAt(m, x, b)->weight = 0; }
    for (int x = 1; x <= m->numVertices; x++) *baseNodeAt(m, b, x) = 0;
    for (int i = 0; i < m->blossomLen[b]; i++) {
        int child = m->blossomNodes[b][i];
        for (int x = 1; x <= m->numNodes; x++)
            if (edgeAt(m, b, x)->weight == 0 ||
                reducedCost(m, edgeAt(m, child, x)) < reducedCost(m, edgeAt(m, b, x))) {
                *edgeAt(m, b, x) = *edgeAt(m, child, x);
                *edgeAt(m, x, b) = *edgeAt(m, x, child);
            }
        for (int x = 1; x <= m->numVertices; x++)
            if (*baseNodeAt(m, child, x)) *baseNodeAt(m, b, x) = child;
    }
    setSlack(m, b);
}

// Expands a blossom whose dual reached zero, restoring its children to the tree.
static void expandBlossom(WeightedMatching* m, int b) {
    for (int i = 0; i < m->blossomLen[b]; i++) setBase(m, m->blossomNodes[b][i], m->blossomNodes[b][i]);

    int xr = *baseNodeAt(m, b, edgeAt(m, b, m->parent[b])->u);
    int pr = childOffset(m, b, xr);

    // the children on the matched side rejoin the alternating tree in pairs
    for (int i = 0; i < pr; i += 2) {
        int outer = m->blossomNodes[b][i];
        int inner = m->blossomNodes[b][i + 1];
        m->parent[outer] = edgeAt(m, inner, outer)->u;
        m->state[outer] = STATE_INNER;
        m->state[inner] = STATE_OUTER;
        m->slackFrom[outer] = 0;
        setSlack(m, inner);
        queuePush(m, inner);
    }
    m->state[xr] = STATE_INNER;
    m->parent[xr] = m->parent[b];
    for (int i = pr + 1; i < m->blossomLen[b]; i++) {
        int child = m->blossomNodes[b][i];
        m->state[child] = STATE_FREE;
        setSlack(m, child);
    }
    m->base[b] = 0; // free the pseudo-node slot for reuse
}

// --- core search -----------------------------------------------------------

// Handles a tight edge e reached during the BFS: grow the tree, close a blossom, or augment.
// Returns 1 if an augmenting path was found and applied.
static int onTightEdge(WeightedMatching* m, const Edge* e) {
    int u = m->base[e->u];
    int v = m->base[e->v];

    if (m->state[v] == STATE_FREE) {
        // grow the tree: v becomes inner, its matched partner becomes outer
        m->parent[v] = e->u;
        m->state[v] = STATE_INNER;
        int partner = m->base[m->match[v]];
        m->slackFrom[v] = m->slackFrom[partner] = 0;
        m->state[partner] = STATE_OUTER;
        queuePush(m, partner);
    } else if (m->state[v] == STATE_OUTER) {
        int lca = lowestCommonAncestor(m, u, v);
        if (!lca) { augment(m, u, v); augment(m, v, u); return 1; }
        addBlossom(m, u, lca, v);
    }
    return 0;
}

// Grows an alternating tree from every free node, adjusting duals as needed, until an augmenting path is found.
// Returns 1 on success, 0 when none exists.
static int findAugmentingPath(WeightedMatching* m) {
    for (int i = 0; i <= m->numNodes; i++) { m->state[i] = STATE_FREE; m->slackFrom[i] = 0; }

    queueReset(m);
    for (int x = 1; x <= m->numNodes; x++)
        if (m->base[x] == x && !m->match[x]) { m->parent[x] = 0; m->state[x] = STATE_OUTER; queuePush(m, x); }
    if (m->queueHead == m->queueTail) return 0;

    for (;;) {
        // scan outer nodes for tight edges
        while (m->queueHead < m->queueTail) {
            int u = m->queue[m->queueHead++];
            if (m->state[m->base[u]] != STATE_OUTER) continue;
            for (int v = 1; v <= m->numVertices; v++)
                if (edgeAt(m, u, v)->weight > 0 && m->base[u] != m->base[v]) {
                    if (reducedCost(m, edgeAt(m, u, v)) == 0) {
                        if (onTightEdge(m, edgeAt(m, u, v))) return 1;
                    } else {
                        updateSlack(m, u, m->base[v]);
                    }
                }
        }

        // no tight edge available: shift the duals by the smallest slack (delta)
        long long delta = 0;
        int haveDelta = 0;
        for (int b = m->numVertices + 1; b <= m->numNodes; b++)
            if (m->base[b] == b && m->state[b] == STATE_INNER) {
                long long c = m->label[b] / 2;
                if (!haveDelta || c < delta) { delta = c; haveDelta = 1; }
            }
        for (int x = 1; x <= m->numNodes; x++)
            if (m->base[x] == x && m->slackFrom[x]) {
                long long c;
                if (m->state[x] == STATE_FREE)       c = reducedCost(m, edgeAt(m, m->slackFrom[x], x));
                else if (m->state[x] == STATE_OUTER) c = reducedCost(m, edgeAt(m, m->slackFrom[x], x)) / 2;
                else continue;
                if (!haveDelta || c < delta) { delta = c; haveDelta = 1; }
            }
        if (!haveDelta) return 0; // safety: no perfect matching reachable

        // apply delta: outer nodes drop, inner nodes rise (blossoms move by 2*delta)
        for (int u = 1; u <= m->numVertices; u++) {
            if (m->state[m->base[u]] == STATE_OUTER) {
                if (m->label[u] <= delta) return 0;
                m->label[u] -= delta;
            } else if (m->state[m->base[u]] == STATE_INNER) {
                m->label[u] += delta;
            }
        }
        for (int b = m->numVertices + 1; b <= m->numNodes; b++)
            if (m->base[b] == b) {
                if (m->state[b] == STATE_OUTER)      m->label[b] += delta * 2;
                else if (m->state[b] == STATE_INNER) m->label[b] -= delta * 2;
            }

        // pick up edges that just became tight, then expand any zero-dual blossoms
        queueReset(m);
        for (int x = 1; x <= m->numNodes; x++)
            if (m->base[x] == x && m->slackFrom[x] && m->base[m->slackFrom[x]] != x &&
                reducedCost(m, edgeAt(m, m->slackFrom[x], x)) == 0)
                if (onTightEdge(m, edgeAt(m, m->slackFrom[x], x))) return 1;
        for (int b = m->numVertices + 1; b <= m->numNodes; b++)
            if (m->base[b] == b && m->state[b] == STATE_INNER && m->label[b] == 0) expandBlossom(m, b);
    }
}

// --- lifecycle -------------------------------------------------------------

static WeightedMatching* createMatching(int numVertices) {
    WeightedMatching* m = malloc(sizeof(WeightedMatching));
    if (!m) return NULL;

    m->numVertices = numVertices;
    m->numNodes = numVertices;
    m->capacity = 2 * numVertices + 2;

    int cap = m->capacity;
    m->edges       = calloc((long long)cap * cap, sizeof(Edge));
    m->label       = calloc(cap, sizeof(long long));
    m->match       = calloc(cap, sizeof(int));
    m->slackFrom   = calloc(cap, sizeof(int));
    m->base        = calloc(cap, sizeof(int));
    m->parent      = calloc(cap, sizeof(int));
    m->state       = calloc(cap, sizeof(int));
    m->mark        = calloc(cap, sizeof(int));
    m->blossomLen  = calloc(cap, sizeof(int));
    m->baseNode    = calloc((long long)cap * (numVertices + 1), sizeof(int));
    m->blossomNodes = calloc(cap, sizeof(int*));
    if (m->blossomNodes)
        for (int i = 0; i < cap; i++) m->blossomNodes[i] = calloc(cap, sizeof(int));

    m->queueCap = 8 * numVertices + 64;
    m->queueHead = m->queueTail = 0;
    m->queue = malloc(m->queueCap * sizeof(int));

    return m;
}

static void destroyMatching(WeightedMatching* m) {
    if (!m) return;
    if (m->blossomNodes)
        for (int i = 0; i < m->capacity; i++) free(m->blossomNodes[i]);
    free(m->blossomNodes);
    free(m->edges); free(m->label); free(m->match); free(m->slackFrom);
    free(m->base); free(m->parent); free(m->state); free(m->mark);
    free(m->blossomLen); free(m->baseNode); free(m->queue);
    free(m);
}

// --- public entry point ----------------------------------------------------

// Computes a minimum weight perfect matching over the odd-degree vertices.
// Inputs describe an undirected weighted graph on vertices 0..N-1 (N even):
// E edges, edge i joining u[i] and v[i] with weight w[i]. On return match_out[i] is the partner of vertex i, or -1 if it was left unmatched.
void blossomMWPM(int N, int E, int* u, int* v, double* w, int* match_out) {
    WeightedMatching* m = createMatching(N);
    if (!m) {
        for (int i = 0; i < N; i++) match_out[i] = -1;
        return;
    }

    // transform minimisation into maximisation: w' = CAP - w, kept strictly positive
    double maxWeight = 0.0;
    for (int i = 0; i < E; i++) if (w[i] > maxWeight) maxWeight = w[i];
    long long cap = (long long)llround(maxWeight * WEIGHT_SCALE) + 1;

    for (int i = 0; i < E; i++) {
        int a = u[i] + 1, b = v[i] + 1; // this algorithm indexes vertices from 1
        long long tw = cap - (long long)llround(w[i] * WEIGHT_SCALE);
        if (tw <= 0) tw = 1;
        if (edgeAt(m, a, b)->weight < tw) {
            *edgeAt(m, a, b) = (Edge){ a, b, tw };
            *edgeAt(m, b, a) = (Edge){ b, a, tw };
        }
    }

    // every real vertex is its own base; seed the duals with the maximum edge weight
    for (int i = 0; i <= m->numVertices; i++) { m->base[i] = i; m->blossomLen[i] = 0; m->match[i] = 0; }
    long long startLabel = 0;
    for (int a = 1; a <= m->numVertices; a++)
        for (int b = 1; b <= m->numVertices; b++) {
            *baseNodeAt(m, a, b) = (a == b ? a : 0);
            if (edgeAt(m, a, b)->weight > startLabel) startLabel = edgeAt(m, a, b)->weight;
        }
    for (int a = 1; a <= m->numVertices; a++) m->label[a] = startLabel;

    while (findAugmentingPath(m)) ;

    for (int i = 0; i < N; i++) match_out[i] = (m->match[i + 1] ? m->match[i + 1] - 1 : -1);

    destroyMatching(m);
}
