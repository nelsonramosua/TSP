// Trace.c - optional frame sink for visualizing an algorithm's progress. See Trace.h.
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

#include "../../headers/Trace.h"

#include <stddef.h>

static TraceFn g_fn = NULL;
static void* g_ctx = NULL;

void TraceSet(TraceFn fn, void* ctx) { g_fn = fn; g_ctx = ctx; }

int TraceActive(void) { return g_fn != NULL; }

void TraceEmit(const unsigned int* path, unsigned int count, unsigned int n, double cost) {
    if (g_fn) g_fn(path, count, n, cost, g_ctx);
}