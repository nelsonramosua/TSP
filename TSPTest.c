// TSPTest.c - Tests different versions of TSP algorithms with NamedGraph.
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit.

#include "TravelingSalesmanProblem.h"
#include "headers/GraphFactory.h"
#include "headers/TSPTest.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(void) {
    srand(time(NULL)); 

    TestAndRunNamedGraph(CreateGraphAula, "Graph Aula", -1);
    TestAndRunNamedGraph(CreatePortugal12CitiesGraph, "Portugal Graph", -1);
    TestAndRunNamedGraph(CreateMatrixGraph15, "Matrix Graph 15 Nodes", -1);
    TestAndRunNamedGraph(CreateMatrixGraph20, "Matrix Graph 20 Nodes", -1);
    TestAndRunNamedGraph(CreateEuclideanGraph15, "Euclidean Graph 15 Nodes", -1);
    TestAndRunNamedGraph(CreateEil51Graph, "TSPLIB - Eil51", 426);
    TestAndRunNamedGraph(CreateOliver30Graph, "TSPLIB - Oliver30", 420);
    TestAndRunNamedGraph(CreateSwiss42Graph, "TSPLIB - Swiss42", 1273);
    TestAndRunNamedGraph(CreateBays29Graph, "TSPLIB - Bays29", 2020);
    // TestAndRunNamedGraph(CreateA280Graph, "TSPLIB - A280", 2579); // takes very long. Uncomment to stress test.

    // Add your own! (Add in GraphFactory.c/.h (prototype!) and call here!).

    printf("All graphs tested.\n");
    return 0;
}

// helper: creates, tests, and destroys a NamedGraph
static void TestAndRunNamedGraph(NamedGraph* (*creator_func)(void), const char* graphName, double knownOptimalCost) {
    NamedGraph* ng = creator_func();
    if (!ng) return;

    unsigned int numVertices = GraphGetNumVertices(ng->g);
    double actualBestCost = knownOptimalCost;
    Tour* hk_tour = NULL;

    if (numVertices <= 20 && knownOptimalCost == -1) {
        hk_tour = HeldKarp_FindTour(ng->g);
        TourMapCityNames(hk_tour, ng);
        TourInvariant(hk_tour, numVertices);
        actualBestCost = hk_tour->cost;
    }

    RunTSPAlgorithms(ng, graphName, actualBestCost, hk_tour);
    
    if (hk_tour) TourDestroy(&hk_tour);
    NamedGraphDestroy(&ng);
}

static void RunTSPAlgorithms(NamedGraph* ng, const char* graphName, double actualCost, Tour* hk_tour) {
    if (!ng) return;

    printf("\n\n\t\tTESTING GRAPH: %s\n", graphName);

    Graph* g = ng->g;
    unsigned int numVertices = GraphGetNumVertices(g);

    Tour* nn_tour = NULL;

    // Lower Bound (MST)
    double lower_bound = LowerBound_MST(g);
    printf("\t\t**LOWER BOUND (MST Cost): %.2f**\n", lower_bound);
    if (actualCost != -1) printf("\t\tActual Best Cost: %.2f\n", actualCost);

    // 0. Brute-Force (only for GraphGetNumVertices(ng->g) <= 12)
    ExecuteAndDisplay(ng, numVertices, (TSPAlgorithm){
        .tspFun = ExhaustiveSearch_Adapter, .name = "Brute-Force", .maxVertices = 12, .extra = NULL });

    // 0.5. Brute-Force w/ Pruning (only for GraphGetNumVertices(ng->g) <= 12)
    ExecuteAndDisplay(ng, numVertices, (TSPAlgorithm){
        .tspFun = ExhaustiveSearchPruning_Adapter, .name = "Brute-Force w/ Pruning", .maxVertices = 12, .extra = NULL });

    // 1. Held-Karp (only for GraphGetNumVertices(ng->g) <= 20)
    printf("\n[Held-Karp]\n");
    if (hk_tour) {
        TourMapCityNames(hk_tour, ng);
        TourDisplay(hk_tour);
    } else printf("Disabled because graph has too many vertices (%u > %u) and it would be too slow.\n", numVertices, 20);

    // 2. Nearest Neighbour (Called directly to keep nn_tour)
    printf("\n[Nearest Neighbour Heuristic]\n");
    nn_tour = NearestNeighbour_FindTour(g, 0);
    if (nn_tour) {
        TourMapCityNames(nn_tour, ng);
        TourInvariant(nn_tour, numVertices);
        TourDisplay(nn_tour);
    }

    // 3. 2-Opt Improvement (based on Nearest Neighbour)
    Tour * twoopt_tour = TourDeepCopy(nn_tour);
    ExecuteAndDisplay(ng, numVertices, (TSPAlgorithm){
        .tspFun = TwoOpt_Adapter, .name = "2-Opt Improvement", .maxVertices = 0, .extra = twoopt_tour });

    // 4. Greedy
    ExecuteAndDisplay(ng, numVertices, (TSPAlgorithm){
        .tspFun = Greedy_Adapter, .name = "Greedy Heuristic", .maxVertices = 0, .extra = NULL });

    // 5. Nearest Insertion
    ExecuteAndDisplay(ng, numVertices, (TSPAlgorithm){
        .tspFun = NearestInsertion_Adapter, .name = "Nearest Insertion Heuristic", .maxVertices = 0, .extra = NULL });

    // 6. Christofides
    ExecuteAndDisplay(ng, numVertices, (TSPAlgorithm){
        .tspFun = Christofides_Adapter, .name = "Christofides Algorithm", .maxVertices = 0, .extra = NULL });

    // 7. Simulated Annealing (based on Nearest Neighbour)
    unsigned int* initialTour = malloc((numVertices + 1) * sizeof(unsigned int));
    if (initialTour) {
        memcpy(initialTour, nn_tour->path, (numVertices + 1) * sizeof(unsigned int));
        ExecuteAndDisplay(ng, numVertices, (TSPAlgorithm){
                .tspFun = SimAnnealing_Adapter, .name = "Simulated Annealing", .maxVertices = 0, .extra = initialTour });
        free(initialTour);
    }
    TourDestroy(&nn_tour); // destroy previously left nn_tour.

    // 8. Ant Colony
    ExecuteAndDisplay(ng, numVertices, (TSPAlgorithm){
        .tspFun = AntColony_Adapter, .name = "Ant Colony Optimization", .maxVertices = 0, .extra = NULL });

    // 9. Genetic Algorithm
    ExecuteAndDisplay(ng, numVertices, (TSPAlgorithm){
        .tspFun = GeneticAlgorithm_Adapter, .name = "Genetic Algorithm Optimization", .maxVertices = 55, .extra = NULL });
    
    printf("TESTING COMPLETE: %s\n", graphName);
}

// helper: runs and displays a Tour using the context provided by the ADT
static void ExecuteAndDisplay(NamedGraph* ng, unsigned int numVertices, TSPAlgorithm alg) {
    printf("\n[%s]\n", alg.name);

    if (alg.maxVertices > 0 && numVertices > alg.maxVertices) {
        printf("Disabled because graph has too many vertices (%u > %u) and it would be too slow.\n", numVertices, alg.maxVertices);
        return;
    }

    Tour* tour = alg.tspFun(ng->g, alg.extra);
    if (!tour) return;

    TourMapCityNames(tour, ng);
    TourInvariant(tour, numVertices);
    TourDisplay(tour);
    TourDestroy(&tour);
}