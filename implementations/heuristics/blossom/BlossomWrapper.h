#ifndef BLOSSOM_WRAPPER_H
#define BLOSSOM_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// N = number of vertices
// E = number of edges
// u, v = arrays of edges (size E)
// w = array of edge weights (size E)
// match_out = array of size N, returns matched vertex for each vertex
void blossomMWPM(int N, int E, int* u, int* v, double* w, int* match_out);

#ifdef __cplusplus
}
#endif

#endif