// Blossom.hpp

#ifndef BLOSSOM_HPP
#define BLOSSOM_HPP

#include <vector>
using namespace std;

// Wrapper function for WeightedBlossom
// N = number of vertices (should be even, or pad with dummy vertices)
// E = number of edges
// u, v = arrays of edges (size E)
// w = array of edge weights (size E)
// match_out = array of size N, returns the matched vertex for each vertex
void blossomMWPM(int N, int E, int* u, int* v, double* w, int* match_out);

#endif