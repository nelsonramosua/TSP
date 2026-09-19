// Trace.h - optional frame sink for visualizing an algorithm's progress.
//
// Nelson Ramos, 124921.
//
// September, 2026.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Algorithms call TraceEmit() at meaningful points (a city added, a move applied, a new best, ...).
// In normal runs no sink is installed, so TraceEmit() is a cheap no-op and results are unaffected.
// The animation driver (tools/animate.c) installs a sink that writes each frame to a file, which a Python script renders into a GIF.
// `count` is the number of valid vertices in `path`: count < n is a partial tour (construction), count == n is a complete cyclic tour.

#ifndef _TRACE_H_
#define _TRACE_H_

typedef void (*TraceFn)(const unsigned int* path, unsigned int count, unsigned int n, double cost, void* ctx);

void TraceSet(TraceFn fn, void* ctx); // install (or clear, with NULL) the frame sink
int  TraceActive(void);               // 1 if a sink is installed
void TraceEmit(const unsigned int* path, unsigned int count, unsigned int n, double cost);

#endif // _TRACE_H_