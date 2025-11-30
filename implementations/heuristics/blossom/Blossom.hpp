// Blossom.hpp - implements blossom algorithm for Christofides.c
//
// All credit to https://github.com/suddhabrato/edmonds-blossom-algorithm, I adapted from it.
//
// November 2025.

#ifndef BLOSSOM_HPP
#define BLOSSOM_HPP

#include <vector>
using namespace std;

// Wrapper function for WeightedBlossom
void blossomMWPM(int N, int E, int* u, int* v, double* w, int* match_out);

#endif