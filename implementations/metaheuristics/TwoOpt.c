// TwoOpt.c
#include "../../TravelingSalesmanProblem.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// Reverses the segment of the tour path between indices i and j (inclusive, i < j)
static void TwoOpt_Swap(Tour* tour, unsigned int i, unsigned int j) {
    while (i < j) {
        unsigned int temp = tour->path[i];
        tour->path[i] = tour->path[j];
        tour->path[j] = temp;
        i++;
        j--;
    }
}

Tour* TwoOpt_ImproveTour(const Graph* g, Tour* initialTour) {
    if (!initialTour || initialTour->numVertices <= 2) return initialTour;
    
    // N is the number of vertices, initialTour->numVertices is N+1
    unsigned int N = GraphGetNumVertices(g); 
    int improved = 1;

    printf("  Starting 2-Opt. Initial Cost: %.2f\n", initialTour->cost);

    // Loop until no improvement can be made in a full pass
    while (improved) {
        improved = 0;

        // Iterate over all possible non-adjacent edge pairs (i-1, i) and (j, j+1)
        for (unsigned int i = 1; i < N; i++) { // Edge (path[i-1], path[i])
            for (unsigned int j = i + 1; j < N; j++) { // Edge (path[j], path[j+1])
                
                // Skip adjacent edges and the closing edge of the cycle
                if (j == i || j == i + 1 || j == N) continue;

                unsigned int v1 = initialTour->path[i-1];
                unsigned int v2 = initialTour->path[i];
                unsigned int v3 = initialTour->path[j];
                unsigned int v4 = initialTour->path[j+1]; // Note: j+1 can be N (start vertex)

                // 1. Calculate Old Costs
                double cost_v1_v2 = GetEdgeWeight(g, v1, v2);
                double cost_v3_v4 = GetEdgeWeight(g, v3, v4);
                double cost_old = cost_v1_v2 + cost_v3_v4;

                // 2. Calculate New Costs
                double cost_v1_v3 = GetEdgeWeight(g, v1, v3);
                double cost_v2_v4 = GetEdgeWeight(g, v2, v4);
                double cost_new = cost_v1_v3 + cost_v2_v4;

                // 3. Check for improvement
                double delta_cost = cost_new - cost_old;

                if (delta_cost < -DBL_EPSILON) { // Use epsilon for floating point comparison
                    // Improvement found! Perform swap and update cost.
                    TwoOpt_Swap(initialTour, i, j);
                    initialTour->cost += delta_cost;
                    improved = 1;
                    
                    // Restart outer loop after improvement to ensure optimality
                    goto restart_loops;
                }
            }
        }
        
        restart_loops:; 
    }
    
    printf("  Final Cost after 2-Opt: %.2f\n", initialTour->cost);
    return initialTour;
}