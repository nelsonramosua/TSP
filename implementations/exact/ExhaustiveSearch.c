// ExhaustiveSearch.c  // Brute-force method.
#include "../../TravelingSalesmanProblem.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h> 
#include <limits.h>

// Helper to swap two elements in an array
static void swap(unsigned int* a, unsigned int* b) {
    unsigned int temp = *a;
    *a = *b;
    *b = temp;
}

// Recursive function to find the minimum cost tour
// current_path: The array of vertices being permuted (initially V_1 to V_N-1)
// start_index: The position in current_path we are fixing in this step
// N: Total number of vertices in the graph
// g: The graph structure
// best_tour: Pointer to the tour structure storing the best result found so far
static void solve_tsp_recursive(
    unsigned int* current_path, 
    int start_index, 
    unsigned int N, 
    const Graph* g, 
    Tour* best_tour
) {
    // Base Case: All inner vertices (N-1) have been placed
    if ((unsigned int)start_index == N - 1) { 
        // 1. Construct the Full Tour (0, P_1, ..., P_N-1, 0)
        Tour* current_tour = TourCreate(N);
        if (!current_tour) return; // Allocation failure
        
        current_tour->path[0] = 0; // Fixed start
        for (unsigned int i = 0; i < N - 1; i++) {
            current_tour->path[i + 1] = current_path[i];
        }
        current_tour->path[N] = 0; // Closing the loop

        // 2. Calculate Cost
        double current_cost = 0.0;
        for (unsigned int i = 0; i < N; i++) {
            unsigned int u = current_tour->path[i];
            unsigned int v = current_tour->path[i + 1];
            current_cost += GetEdgeWeight(g, u, v);
        }
        current_tour->cost = current_cost;

        // 3. Update Best Tour Found
        if (current_cost < best_tour->cost) {
            // Copy the new, cheaper tour into the best_tour structure
            best_tour->cost = current_cost;
            for (unsigned int i = 0; i <= N; i++) {
                best_tour->path[i] = current_tour->path[i];
            }
        }
        
        TourDestroy(&current_tour);
        return;
    }

    // Recursive Step: Generate permutations
    for (unsigned int i = start_index; i < N - 1; i++) {
        swap(&current_path[start_index], &current_path[i]);
        
        solve_tsp_recursive(current_path, start_index + 1, N, g, best_tour);
        
        // Backtrack: Restore the original order for the next iteration
        swap(&current_path[start_index], &current_path[i]);
    }
}


Tour* ExhaustiveSearch_FindTour(const Graph* g) {
    unsigned int N = GraphGetNumVertices(g);
    if (N < 2) return NULL;

    // 1. Setup the Best Tour Structure
    Tour* best_tour = TourCreate(N);
    if (!best_tour) return NULL;

    // Initialize best cost to maximum
    best_tour->cost = DBL_MAX;

    // 2. Setup the Array for Permutation (Vertices 1 to N-1)
    // Size is N-1.
    unsigned int* inner_vertices = (unsigned int*) malloc((N - 1) * sizeof(unsigned int));
    if (!inner_vertices) { TourDestroy(&best_tour); return NULL; }

    for (unsigned int i = 0; i < N - 1; i++) {
        // inner_vertices holds [1, 2, ..., N-1]
        inner_vertices[i] = i + 1; 
    }

    // 3. Run the Recursive Solver (Starts fixing the first inner vertex)
    solve_tsp_recursive(inner_vertices, 0, N, g, best_tour);

    free(inner_vertices);

    // If the graph is disconnected (impossible for a path to exist), cost will be DBL_MAX
    if (best_tour->cost == DBL_MAX) {
        fprintf(stderr, "Warning: Brute Force failed to find a path (graph disconnected?).\n");
        TourDestroy(&best_tour);
        return NULL;
    }
    
    return best_tour;
}