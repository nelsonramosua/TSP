// Greedy.c (FINAL CORRECTED VERSION)
#include "../../TravelingSalesmanProblem.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h> 

// Removed the unused find_vertex_index function

Tour* Greedy_FindTour(const Graph* g) {
    unsigned int N = GraphGetNumVertices(g);
    if (N == 0) return NULL;
    
    // For a simple 4-vertex graph, start with 0, 1, 2
    if (N < 3) return NearestNeighbour_FindTour(g, 0); 

    Tour* tour = TourCreate(N);
    if (!tour) return NULL;

    int* visited = (int*) calloc(N, sizeof(int));
    if (!visited) { TourDestroy(&tour); return NULL; }
    
    unsigned int current_tour_size = 3; // Start with vertices 0, 1, 2

    // 1. Initialize Subtour: 0 -> 1 -> 2 -> 0
    tour->path[0] = 0; visited[0] = 1; 
    tour->path[1] = 1; visited[1] = 1; 
    tour->path[2] = 2; visited[2] = 1; 
    tour->path[3] = 0; // Closing the subtour

    // Calculate initial cost: 0-1 (10.0) + 1-2 (8.0) + 2-0 (5.0) = 23.0
    tour->cost = GetEdgeWeight(g, 0, 1) + 
                 GetEdgeWeight(g, 1, 2) + 
                 GetEdgeWeight(g, 2, 0);
    
    // 2. Main Insertion Loop
    while (current_tour_size < N) {
        
        double min_increase = DBL_MAX;
        unsigned int best_insert_v = UINT_MAX;
        unsigned int best_i_idx = UINT_MAX;
        
        // --- A. Find the Cheapest Insertion ---
        
        // Iterate over all unvisited vertices (k)
        for (unsigned int v = 0; v < N; v++) {
            if (!visited[v]) {
                
                // Iterate over all edges (i, j) in the current tour (path[0] up to path[current_tour_size-1])
                for (unsigned int i_idx = 0; i_idx < current_tour_size; i_idx++) {
                    unsigned int i = tour->path[i_idx];
                    unsigned int j = tour->path[i_idx + 1]; 
                    
                    // IMPORTANT: Ensure GetEdgeWeight is used consistently
                    double cost_ij = GetEdgeWeight(g, i, j);
                    double cost_iv = GetEdgeWeight(g, i, v);
                    double cost_vj = GetEdgeWeight(g, v, j);
                    
                    double increase = cost_iv + cost_vj - cost_ij;

                    if (increase < min_increase) {
                        min_increase = increase;
                        best_insert_v = v;
                        best_i_idx = i_idx;
                    }
                }
            }
        }
        
        // --- B. Perform the Insertion ---
        
        if (best_insert_v != UINT_MAX) {
            
            unsigned int insert_pos = best_i_idx + 1; // Insert v at path[i_idx + 1]

            // Shift elements right, starting from the closing vertex (path[current_tour_size])
            // and shifting up to the insertion position.
            // Loop goes from path[current_tour_size] down to path[insert_pos]
            for (int k = current_tour_size; k >= (int)insert_pos; k--) {
                // The new path size will be current_tour_size + 1
                tour->path[k + 1] = tour->path[k];
            }
            
            // Insert the vertex
            tour->path[insert_pos] = best_insert_v;
            
            // Update state
            visited[best_insert_v] = 1;
            tour->cost += min_increase;
            current_tour_size++;
            
            // Re-assert the closing edge (it was shifted right by the loop above)
            // tour->path[current_tour_size] is the new last element, set it to the start vertex
            tour->path[current_tour_size] = tour->path[0];
            
        } else {
            fprintf(stderr, "Error: Greedy failed to find next cheapest insertion.\n");
            TourDestroy(&tour);
            free(visited);
            return NULL;
        }
    }

    free(visited);
    return tour;
}