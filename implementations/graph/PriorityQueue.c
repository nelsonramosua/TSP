// PriorityQueue.c - Indexed binary min-heap (priority queue) over integer items.
//
// O(log n) insert / extract-min / decrease-key; O(1) contains.
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Standard binary heap kept in an array, plus a position map (pos[]) so any item can be located in O(1) -- that is what makes DecreaseKey possible in O(log n).
// See the header for the intended use (Greedy) and the deliberate non-use (Prim/MST).

#include "../../headers/PriorityQueue.h"

#include <stdlib.h>

// helpers at EOF.
static int higherPriority(const PriorityQueue* pq, unsigned int a, unsigned int b);
static void swapNodes(PriorityQueue* pq, unsigned int i, unsigned int j);
static void siftUp(PriorityQueue* pq, unsigned int i);
static void siftDown(PriorityQueue* pq, unsigned int i);

PriorityQueue* PQCreate(unsigned int capacity) {
    PriorityQueue* pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    if (!pq) return NULL;

    pq->capacity = capacity;
    pq->size = 0;
    pq->heap = (unsigned int*)malloc(capacity * sizeof(unsigned int));
    pq->key  = (double*)malloc(capacity * sizeof(double));
    pq->pos  = (int*)malloc(capacity * sizeof(int));
    if (!pq->heap || !pq->key || !pq->pos) {
        free(pq->heap); free(pq->key); free(pq->pos); free(pq);
        return NULL;
    }

    for (unsigned int i = 0; i < capacity; i++) pq->pos[i] = -1; // nothing present yet
    return pq;
}

void PQDestroy(PriorityQueue** pq) {
    if (!pq || !(*pq)) return;
    free((*pq)->heap);
    free((*pq)->key);
    free((*pq)->pos);
    free(*pq);
    *pq = NULL;
}

int PQIsEmpty(const PriorityQueue* pq) {
    return pq->size == 0;
}

int PQContains(const PriorityQueue* pq, unsigned int item) {
    return item < pq->capacity && pq->pos[item] != -1;
}

void PQInsert(PriorityQueue* pq, unsigned int item, double key) {
    if (item >= pq->capacity || pq->pos[item] != -1) return; // out of range or already in

    unsigned int i = pq->size++;
    pq->heap[i] = item;
    pq->key[item] = key;
    pq->pos[item] = (int)i;
    siftUp(pq, i);
}

unsigned int PQExtractMin(PriorityQueue* pq) {
    if (pq->size == 0) return pq->capacity; // sentinel: an invalid item id

    unsigned int min = pq->heap[0];
    unsigned int last = pq->heap[--pq->size];

    pq->pos[min] = -1; // min leaves the queue

    if (pq->size > 0) {
        pq->heap[0] = last;
        pq->pos[last] = 0;
        siftDown(pq, 0);
    }
    return min;
}

void PQDecreaseKey(PriorityQueue* pq, unsigned int item, double newKey) {
    if (item >= pq->capacity || pq->pos[item] == -1) return; // not present
    if (newKey >= pq->key[item]) return;                     // only decreases

    pq->key[item] = newKey;
    siftUp(pq, (unsigned int)pq->pos[item]);
}

// Utilities

// True if item a should sit above item b: smaller key, ties broken by smaller id.
static int higherPriority(const PriorityQueue* pq, unsigned int a, unsigned int b) {
    if (pq->key[a] < pq->key[b]) return 1;
    if (pq->key[a] > pq->key[b]) return 0;
    return a < b; // deterministic tie-break
}

static void swapNodes(PriorityQueue* pq, unsigned int i, unsigned int j) {
    unsigned int tmp = pq->heap[i];
    pq->heap[i] = pq->heap[j];
    pq->heap[j] = tmp;
    pq->pos[pq->heap[i]] = (int)i;
    pq->pos[pq->heap[j]] = (int)j;
}

static void siftUp(PriorityQueue* pq, unsigned int i) {
    while (i > 0) {
        unsigned int parent = (i - 1) / 2;
        if (!higherPriority(pq, pq->heap[i], pq->heap[parent])) break;
        swapNodes(pq, i, parent);
        i = parent;
    }
}

static void siftDown(PriorityQueue* pq, unsigned int i) {
    for (;;) {
        unsigned int left = 2 * i + 1;
        unsigned int right = 2 * i + 2;
        unsigned int best = i;

        if (left  < pq->size && higherPriority(pq, pq->heap[left],  pq->heap[best])) best = left;
        if (right < pq->size && higherPriority(pq, pq->heap[right], pq->heap[best])) best = right;

        if (best == i) break;
        swapNodes(pq, i, best);
        i = best;
    }
}