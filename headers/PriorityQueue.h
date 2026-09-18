// PriorityQueue.h - Indexed binary min-heap (priority queue) over integer items.
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// A min-priority queue over a fixed universe of items {0, 1, ..., capacity - 1}, each carrying a double key.
// It is "indexed": a position map lets it find any item in O(1), so it supports DecreaseKey (and re-insertion) in O(log n) -- which is what makes it useful for graph algorithms that repeatedly lower the tentative cost of a vertex.
//
// Ties (equal keys) are broken by item id (smaller id first), so the order in which equal-priority items come out is deterministic.
//
// This is the auxiliary ADT the README/Greedy/Prim notes keep pointing at.
// It is used by Greedy (see Greedy.c); it is deliberately NOT used by Prim/MST -- see the note in Prim_MST.c for why a heap would be a pessimization there on complete graphs.

#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_

typedef struct _PriorityQueue {
    unsigned int capacity;  // size of the item universe (ids 0 .. capacity - 1)
    unsigned int size;      // number of items currently in the queue
    unsigned int* heap;     // heap[0 .. size - 1] = item ids, kept as a min-heap
    double* key;            // key[item] = its priority (only meaningful while present)
    int* pos;               // pos[item] = its index in heap[], or -1 if not present
} PriorityQueue;

PriorityQueue* PQCreate(unsigned int capacity);
void PQDestroy(PriorityQueue** pq);

int PQIsEmpty(const PriorityQueue* pq);
int PQContains(const PriorityQueue* pq, unsigned int item);

// Inserts item with the given key. Item must not already be present.
void PQInsert(PriorityQueue* pq, unsigned int item, double key);

// Removes and returns the item with the smallest key (ties: smallest item id).
// Returns capacity if the queue is empty (an invalid item id acting as a sentinel).
unsigned int PQExtractMin(PriorityQueue* pq);

// Lowers the key of an item already present. A newKey that is not smaller is ignored.
void PQDecreaseKey(PriorityQueue* pq, unsigned int item, double newKey);

#endif // _PRIORITY_QUEUE_H_