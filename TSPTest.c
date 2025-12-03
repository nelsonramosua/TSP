// TSPTest.c - Tests different versions of TSP algorithms.
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit.

#include "TravelingSalesmanProblem.h"
#include "GraphFactory.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// helper: run all algs on a graph                                              // might be NULL
static void RunTSPAlgorithms(Graph* g, const char* graphName, double actualCost, Tour* hk_tour) {
    if (!g) return;

    printf("\n\n\t\tTESTING GRAPH: %s\n", graphName);

    // Lower Bound (MST)
    double lower_bound = LowerBound_MST(g);
    printf("\t\t**LOWER BOUND (MST Cost): %.2f**\n", lower_bound);

    if(actualCost != -1) printf("\t\tActual Best Cost: %.2f\n", actualCost);

    // 0. Brute-Force (only for numVertices <= 12)
    printf("\n[Brute-Force]\n");
    if(GraphGetNumVertices(g) <= 12)  { 
        Tour* bf_tour = ExhaustiveSearch_FindTour(g);
        TourInvariant(bf_tour, GraphGetNumVertices(g));
        TourDisplay(bf_tour);
        TourDestroy(&bf_tour);
    }
    else printf("Disabled because graph is too big and it would take too long.\n");

    // 0.5. Brute-Force Pruning (only for numVertices <= 12)
    printf("\n[Brute-Force w/ Pruning]\n");
    if(GraphGetNumVertices(g) <= 12)  { 
        Tour* bf_tour_pu = ExhaustiveSearchPruning_FindTour(g);
        TourInvariant(bf_tour_pu, GraphGetNumVertices(g));
        TourDisplay(bf_tour_pu);
        TourDestroy(&bf_tour_pu);
    }
    else printf("Disabled because graph is too big and it would take too long.\n");

    // 1. Held-Karp DP (only for numVertices <= 15)
    printf("\n[Held-Karp]\n");
    if (hk_tour != NULL) { TourDisplay(hk_tour); goto afterHK; }
    if(GraphGetNumVertices(g) <= 20)  { 
        Tour* hk_tour = HeldKarp_FindTour(g);
        TourInvariant(hk_tour, GraphGetNumVertices(g));
        TourDisplay(hk_tour);
        TourDestroy(&hk_tour);
    }
    else printf("Disabled because graph is too big and it would take too long.\n");

afterHK:

    // 2. Nearest Neighbour Heuristic
    Tour* nn_tour = NearestNeighbour_FindTour(g, 0);
    TourInvariant(nn_tour, GraphGetNumVertices(g));
    printf("\n[Nearest Neighbour Heuristic]\n");
    TourDisplay(nn_tour);

    // 3. 2-Opt Improvement
    Tour* improved_tour = NULL;
    if (nn_tour) {
        improved_tour = TwoOpt_ImproveTour(g, nn_tour);
        TourInvariant(improved_tour, GraphGetNumVertices(g));
        printf("\n[2-Opt Improvement]\n");
        TourDisplay(improved_tour);
    }
    TourDestroy(&nn_tour);

    // 4. Greedy Heuristic
    Tour* greedy_tour = Greedy_FindTour(g);
    TourInvariant(greedy_tour, GraphGetNumVertices(g));
    printf("\n[Greedy Heuristic]\n");
    TourDisplay(greedy_tour);

    // 5. Nearest Insertion Heuristic
    Tour* nearestInsertion_tour = NearestInsertion_FindTour(g);
    TourInvariant(nearestInsertion_tour, GraphGetNumVertices(g));
    printf("\n[Nearest Insertion Heuristic]\n");
    TourDisplay(nearestInsertion_tour);
    TourDestroy(&nearestInsertion_tour);

    // 6. Christofides Algorithm (could not get it working correctly...)
    Tour* christofides_tour = Christofides_FindTour(g);
    TourInvariant(christofides_tour, GraphGetNumVertices(g));
    printf("\n[Christofides Algorithm]\n");
    TourDisplay(christofides_tour);
    TourDestroy(&christofides_tour);

    // 7. Simulated Annealing
    unsigned int numVertices = GraphGetNumVertices(g);
    unsigned int* initialTour = malloc(numVertices * sizeof(unsigned int));
    if (initialTour && greedy_tour) {
        for (unsigned int i = 0; i < numVertices; i++) initialTour[i] = greedy_tour->path[i];
        Tour* sa_tour = SimulatedAnnealing_FindTour(g, initialTour);
        TourInvariant(sa_tour, GraphGetNumVertices(g));
        printf("\n[Simulated Annealing]\n");
        TourDisplay(sa_tour);
        TourDestroy(&sa_tour);
    }
    free(initialTour);
    TourDestroy(&greedy_tour);

    // 8. Ant Colony Optimization
    Tour* aco_tour = AntColony_FindTour(g);
    TourInvariant(aco_tour, GraphGetNumVertices(g));
    printf("\n[Ant Colony Optimization]\n");
    TourDisplay(aco_tour);
    TourDestroy(&aco_tour);

    // 9. Genetic Algorithm (only for numVertices <= 55)
    printf("\n[Genetic Algorithm Optimization]\n");
    if(GraphGetNumVertices(g) <= 55) { 
        Tour* ga_tour = GeneticAlgorithm_FindTour(g);
        TourInvariant(ga_tour, GraphGetNumVertices(g));
        TourDisplay(ga_tour);
        TourDestroy(&ga_tour);
    }
    else printf("Disabled because graph is too big and it would take too long.");

    printf("TESTING COMPLETE: %s\n", graphName);
}

// helper: creates, tests, and destroys a specific graph structure.
static void TestAndRunGraph(Graph* (*creator_func)(void), const char* graphName, double knownOptimalCost) {
    Graph* g = creator_func();
    
    unsigned int numVertices = GraphGetNumVertices(g);
    double actualBestCost = knownOptimalCost;
    Tour* hk_tour = NULL;

    if (numVertices <= 20 && knownOptimalCost == -1) { // Held-Karp, if numVertices <= 20, will return optimum cost.
        hk_tour = HeldKarp_FindTour(g);
        TourInvariant(hk_tour, numVertices);
        actualBestCost = hk_tour->cost;
    } 

    RunTSPAlgorithms(g, graphName, actualBestCost, hk_tour);

    if (hk_tour) TourDestroy(&hk_tour);
    GraphDestroy(&g);
}

int main(void) {
    srand(time(NULL)); 

    // 1. Graph Aula (N=4)
    TestAndRunGraph(CreateGraphAula, "Graph Aula", -1);

    // 2. Matrix Graph 15 Nodes (N=15)
    TestAndRunGraph(CreateMatrixGraph15, "Matrix Graph 15 Nodes", -1);
    
    // 3. Matrix Graph 20 Nodes (N=20)
    TestAndRunGraph(CreateMatrixGraph20, "Matrix Graph 20 Nodes", -1);

    // 4. Euclidean Graph 15 Nodes (N=15)
    TestAndRunGraph(CreateEuclideanGraph15, "Euclidean Graph 15 Nodes", -1);

    // 5. TSPLIB - Eil51 (N=51, Optimal Cost 426)
    TestAndRunGraph(CreateEil51Graph, "TSPLIB - Eil51", 426);

    // 6. TSPLIB - Oliver30 (N=30, Optimal Cost 420)
    TestAndRunGraph(CreateOliver30Graph, "TSPLIB - Oliver30", 420);

    // 7. TSPLIB - Swiss42 (N=42, Optimal Cost 1273)
    TestAndRunGraph(CreateSwiss42Graph, "TSPLIB - Swiss42", 1273);

    // 8. TSPLIB - Bays29 (N=29, Optimal Cost 2020)
    TestAndRunGraph(CreateBays29Graph, "TSPLIB - Bays29", 2020);

    // 9. TSPLIB - A280 (N=280, Optimal Cost 2579)
    // Takes a long time... You might want to comment this line.
    TestAndRunGraph(CreateA280Graph, "TSPLIB - A280", 2579);

    // --- RANDOM EUCLIDEAN GRAPHS ---
    int numRandEucGraphs = 1;
    for (int i = 0; i < numRandEucGraphs; i++) {
        Graph* randGraph = CreateRandomEuclideanGraph(10 + i*5, 100.0, 100.0);
        char name[64];
        snprintf(name, sizeof(name), "Random Euclidean Graph %u Nodes", GraphGetNumVertices(randGraph));
        RunTSPAlgorithms(randGraph, name, -1, NULL);
        GraphDestroy(&randGraph);
    }

    printf("All graphs tested.\n");
    return 0;
}