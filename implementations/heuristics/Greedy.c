// Greedy.c - Implements Greedy approximation algorithm.
//
// Nelson Ramos, 124921.
//
// November, 2025.
// 
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit.

// Resources used:
// https://www.geeksforgeeks.org/dsa/travelling-salesman-problem-greedy-approach/

#include "../../TravelingSalesmanProblem.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h> 

Tour* Greedy_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices == 0) return NULL;

    // for graphs with < 3 vertices, use NN approx.
    if (numVertices < 3) return NearestNeighbour_FindTour(g, 0);
    
    Tour* tour = TourCreate(numVertices);
    if (!tour) return NULL;

    int* visited = (int*) calloc(numVertices, sizeof(int));
    if (!visited) { TourDestroy(&tour); return NULL; }
    
    // currentTourSize keeps number of unique verts currently in tour
    unsigned int currentTourSize = 3; // start with vertices 0, 1, 2

    // init Subtour ( 0 - 1 - 2 - 0 )
    tour->path[0] = 0; visited[0] = 1; 
    tour->path[1] = 1; visited[1] = 1; 
    tour->path[2] = 2; visited[2] = 1; 
    tour->path[3] = 0; // close subtour

    // calc init cost
    tour->cost = GetEdgeWeight(g, 0, 1) + 
                 GetEdgeWeight(g, 1, 2) + 
                 GetEdgeWeight(g, 2, 0);
    
    // iterate until all verts are included
    while (currentTourSize < numVertices) {
        
        double minIncrease = DBL_MAX;
        unsigned int bestInsertV = UINT_MAX;
        unsigned int bestIIdx = UINT_MAX; // index i of edge (i, j) to replace
                
        // iterate over unvisited vertices k
        for (unsigned int v = 0; v < numVertices; v++) {
            if (!visited[v]) {
                // iterate over edges (i, j) in cur subtour
                for (unsigned int iIdx = 0; iIdx < currentTourSize; iIdx++) {
                    unsigned int i = tour->path[iIdx];
                    unsigned int j = tour->path[iIdx + 1]; 
                    
                    // calc cost components
                    double costIj = GetEdgeWeight(g, i, j);
                    double costIv = GetEdgeWeight(g, i, v);
                    double costVj = GetEdgeWeight(g, v, j);
                    
                    double increase = costIv + costVj - costIj;

                    if (increase < minIncrease) {
                        minIncrease = increase;
                        bestInsertV = v;
                        bestIIdx = iIdx;
                    }
                }
            }
        }
                
        if (bestInsertV != UINT_MAX) {
            // new vertex 'v' inserted at pos iIdx + 1, 
            // breaking edge tour->path[iIdx] -> tour->path[iIdx + 1].
            unsigned int insertPos = bestIIdx + 1; 

            // shift elements from closing vertex backwards, up to insertPos
            // loop runs from the cur end of path down to insertPos.
            for (int k = currentTourSize; k >= (int)insertPos; k--) {
                // shift is tour->path[k] to tour->path[k + 1]
                tour->path[k + 1] = tour->path[k];
            }
            
            // insert vertex
            tour->path[insertPos] = bestInsertV;
            
            // update state and cost
            visited[bestInsertV] = 1;
            tour->cost += minIncrease;
            currentTourSize++; 
            
            // last element back to start
            tour->path[currentTourSize] = tour->path[0];
            
        } else {
            // only happens if graph is disconnected (should not happen)
            fprintf(stderr, "Error: Greedy failed to find the next cheapest insertion (graph disconnected?).\n");
            TourDestroy(&tour);
            free(visited);
            return NULL;
        }
    }

    free(visited);
    return tour;
}