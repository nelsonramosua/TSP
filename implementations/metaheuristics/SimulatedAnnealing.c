// SimulatedAnnealing.c
#include "../../TravelingSalesmanProblem.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h> // for DBL_MAX
#include <string.h> // For memcpy

// Helper: swap two unsigned int
static void swap(unsigned int* a, unsigned int* b) {
    unsigned int t = *a;
    *a = *b;
    *b = t;
}

// 2-opt swap: reverse segment [i..j] in tour
// Note: i and j are the INDICES of the vertices to swap
static void twoOptSwap(unsigned int* tour, unsigned int i, unsigned int j) {
    while (i < j) swap(&tour[i++], &tour[j--]);
}

// Compute total tour cost
static double tourCost(const Graph* g, unsigned int* tour, unsigned int N) {
    double cost = 0.0;
    // Iterate N times (N edges in the cycle)
    for (unsigned int i = 0; i < N; i++) {
        // Edge is (tour[i], tour[(i + 1) % N])
        double w = GetEdgeWeight(g, tour[i], tour[(i + 1) % N]);
        if (w == DBL_MAX) return DBL_MAX; // invalid edge
        cost += w;
    }
    return cost;
}

Tour* SimulatedAnnealing_FindTour(const Graph* g, unsigned int* initialTour) {
    unsigned int N = GraphGetNumVertices(g);
    if (N < 2) return NULL; // Need at least 2 vertices

    Tour* tour = TourCreate(N);
    if (!tour) return NULL;

    // Use a fixed seed for better testing, or keep time(NULL) for randomness
    // srand((unsigned int)time(NULL)); 

    // --- 1) Initialize tour
    unsigned int* current = malloc(N * sizeof(unsigned int));
    unsigned int* bestTour = malloc(N * sizeof(unsigned int));
    if (!current || !bestTour) { 
        if (current) free(current); 
        if (bestTour) free(bestTour); 
        TourDestroy(&tour); 
        return NULL; 
    }

    // InitialTour is assumed to be a valid permutation of size N
    memcpy(current, initialTour, N * sizeof(unsigned int));
    memcpy(bestTour, initialTour, N * sizeof(unsigned int));

    double currentCost = tourCost(g, current, N);
    double bestCost = currentCost;

    // Handle invalid initial tour cost (shouldn't happen with Christofides)
    if (bestCost == DBL_MAX) {
        // If initial tour is invalid, we can't proceed.
        // For a more robust SA, one would generate a random valid tour here.
        fprintf(stderr, "[SA ERROR] Initial tour is invalid (DBL_MAX cost).\n");
        free(current);
        free(bestTour);
        TourDestroy(&tour);
        return NULL;
    }

    // --- 2) SA parameters
    // Optimized parameters for TSP
    double T = currentCost / 10.0; // Start T related to the initial solution quality
    if (T < 100.0) T = 100.0;     // Ensure minimum starting T
    
    double Tmin = 1e-6; // Go colder
    double alpha = 0.90; // Slower cooling for better exploration
    unsigned int iterations = 25 * N; // Significantly more iterations for metaheuristics

    // --- 3) SA main loop
    while (T > Tmin) {
        for (unsigned int k = 0; k < iterations; k++) {
            
            // --- CRITICAL FIX: Robust 2-opt segment selection ---
            // Choose i and j such that they are not equal, not adjacent, and 
            // 0 <= i < j < N. We need to select two edges (i, i+1) and (j, j+1).
            unsigned int i, j;
            
            // Generate two distinct indices, 0 <= i_idx, j_idx < N
            do {
                i = rand() % N;
                j = rand() % N;
            } while (i == j || i == (j+1)%N || j == (i+1)%N); // Ensure non-adjacent edges

            if (i > j) swap(&i, &j); // Ensure i < j for simpler segment reversal

            // Now i and j define the start of two non-adjacent edges.
            // The segment to be reversed is between index i+1 and index j.
            unsigned int seg_start = (i + 1) % N;
            unsigned int seg_end = j;
            
            // --- 2-opt Delta Calculation ---
            unsigned int a = current[i]; // End of first old edge, start of new edge (A->C)
            unsigned int b = current[(i + 1) % N]; // Start of reversed segment (B)
            unsigned int c = current[j]; // End of reversed segment (C)
            unsigned int d = current[(j + 1) % N]; // Start of second old edge, end of new edge (B->D)

            // Re-check indices for reversal: reverse segment [i+1...j]
            // The edges being removed are (a, b) and (c, d)
            // The edges being added are (a, c) and (b, d)
            
            double w_ab = GetEdgeWeight(g, a, b);
            double w_cd = GetEdgeWeight(g, c, d);
            double w_ac = GetEdgeWeight(g, a, c);
            double w_bd = GetEdgeWeight(g, b, d);

            if (w_ab == DBL_MAX || w_cd == DBL_MAX || w_ac == DBL_MAX || w_bd == DBL_MAX)
                continue;

            double delta = -w_ab - w_cd + w_ac + w_bd;

            // Accept swap with Metropolis criterion
            if (delta < 0 || exp(-delta / T) > (double)rand() / RAND_MAX) {
                // Perform the swap on the segment [i+1, j]
                if (seg_start < seg_end) {
                    twoOptSwap(current, seg_start, seg_end);
                } else {
                    // Handle wrap-around case (segment is [i+1..N-1] and [0..j])
                    // This is more complex, let's simplify selection to avoid wrap-around for now
                    // The generation loop above: while (i == j || i == (j+1)%N || j == (i+1)%N)
                    // and if (i > j) swap(&i, &j) ensures i < j.
                    // If i+1 > j, it means i=N-1 and j=0, which is covered by the `do/while` 
                    // loop being non-adjacent. 
                    
                    // To guarantee a continuous segment, let's constrain i and j:
                    // Force 0 <= i < j-1 < N-1 to ensure a gap.
                    
                    // REVERTING TO ORIGINAL 2-OPT LOGIC:
                    // The simplest 2-opt reversal swaps the edges (i-1, i) and (j, j+1), 
                    // reversing the segment from [i..j].
                    // Let's use simpler indices for the segment endpoints:
                    
                    // New segment selection (safer, less general)
                    unsigned int ii = rand() % N;
                    unsigned int jj = rand() % N;
                    if (ii == jj) continue;
                    if (ii > jj) swap(&ii, &jj);

                    // Ensure indices are not adjacent
                    if (jj == ii + 1) continue; 
                    
                    // The edges removed are (ii-1, ii) and (jj, jj+1)
                    // The segment reversed is [ii, jj]
                    
                    // Use the original code's indices for maximum compatibility with the code structure:
                    twoOptSwap(current, i + 1, j); // The indices in your original 2-opt: segment [i+1] to [j]

                    currentCost += delta;

                    if (currentCost < bestCost) {
                        bestCost = currentCost;
                        memcpy(bestTour, current, N * sizeof(unsigned int));
                    }
                }
            }
        }
        T *= alpha; // Cool down
    }

    // --- 4) Copy best tour into output
    for (unsigned int i = 0; i < N; i++) tour->path[i] = bestTour[i];
    tour->path[N] = bestTour[0]; // Close the cycle
    tour->cost = bestCost;

    free(current);
    free(bestTour);
    return tour;
}