// LowerBound_MST.c
#include "../../TravelingSalesmanProblem.h"
#include <stdio.h>

// Forward declaration of the MST calculation function from Prim_MST.c
extern double Prim_CalculateMSTCost(const Graph* g); 

// The public interface for the Lower Bound calculation
double LowerBound_MST(const Graph* g) {
    if (!GraphIsWeighted(g) || GraphIsDigraph(g)) {
        fprintf(stderr, "Error: Lower bound calculation requires a weighted, non-digraph.\n");
        return 0.0;
    }
    
    // The cost of the MST is the standard lower bound for TSP
    return Prim_CalculateMSTCost(g);
}