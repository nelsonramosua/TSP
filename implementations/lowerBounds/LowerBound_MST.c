// LowerBound_MST.c - Determines LowerBound Cost for TSP by MST.
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/LowerBounds.h"

#include <stdio.h>

// Forward declaration of the MST calculation function from Prim_MST.c. (O(N^2))
extern double Prim_CalculateMSTCost(const Graph* g); 

// public interface for the Lower Bound calculation
double LowerBound_MST(const Graph* g) {
    if (!GraphIsWeighted(g) || GraphIsDigraph(g)) {
        fprintf(stderr, "Error: Lower bound calculation requires a weighted, non-digraph.\n");
        return 0.0;
    }
    
    // cost of MST is the standard lower bound for TSP
    return Prim_CalculateMSTCost(g);
}