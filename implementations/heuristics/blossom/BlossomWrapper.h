// BlossomWrapper.hpp - implements a wrapper for blossom algorithm for Christofides.c
//
// All credit for the algorithm to https://github.com/suddhabrato/edmonds-blossom-algorithm, I adapted from it.
//
// November 2025.

// ports C++ to C.

#ifndef BLOSSOM_WRAPPER_H
#define BLOSSOM_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

void blossomMWPM(int N, int E, int* u, int* v, double* w, int* match_out);

#ifdef __cplusplus
}
#endif

#endif