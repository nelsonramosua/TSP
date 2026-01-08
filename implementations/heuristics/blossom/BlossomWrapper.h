// BlossomWrapper.h - Minimum Weight Perfect Matching for Christofides algorithm.
//
// O(n³) --- implementation of Edmonds' weighted Blossom algorithm using the primal-dual method.
//
// Based on the algorithm described in "Combinatorial Optimization" by Korte & Vygen.
// https://www.mathematik.uni-muenchen.de/~kpanagio/KombOpt/book.pdf
//
// January 2026.

// near transcription... so no props to me.

#ifndef BLOSSOM_WRAPPER_H
#define BLOSSOM_WRAPPER_H

void blossomMWPM(int N, int E, int* u, int* v, double* w, int* match_out);

#endif