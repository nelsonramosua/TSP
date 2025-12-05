// TSPTest.c - Tests different versions of TSP algorithms with NamedGraph.
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

#include "TravelingSalesmanProblem.h"
#include "headers/GraphFactory.h"
#include "headers/TSPTest.h"

int main(void) {
    srand(time(NULL)); 

    testRunNamedGraph(CreateGraphAula, "Graph Aula", -1);
    testRunNamedGraph(CreatePortugal12CitiesGraph, "Graph 12 Portuguese Cities", -1);
    testRunNamedGraph(CreateEurope12CitiesGraph, "Graph 12 European Cities", -1);
    testRunNamedGraph(CreateMatrixGraph15, "Matrix Graph 15 Nodes", -1);
    testRunNamedGraph(CreateMatrixGraph20, "Matrix Graph 20 Nodes", -1);
    testRunNamedGraph(CreateEuclideanGraph15, "Euclidean Graph 15 Nodes", -1);
    testRunNamedGraph(CreateEil51Graph, "TSPLIB - Eil51", 426);
    testRunNamedGraph(CreateOliver30Graph, "TSPLIB - Oliver30", 420);
    testRunNamedGraph(CreateSwiss42Graph, "TSPLIB - Swiss42", 1273);
    testRunNamedGraph(CreateBays29Graph, "TSPLIB - Bays29", 2020);
    // testRunNamedGraph(CreateA280Graph, "TSPLIB - A280", 2579); // takes very long. Uncomment to stress test.

    // Add your own! (Add in GraphFactory.c/.h (prototype!) and call here!). See GraphFactory.c for more info.

    printf("All graphs tested.\n");
    return 0;
}

// helper: creates, tests, and destroys a NamedGraph
static void testRunNamedGraph(NamedGraph* (*creatorFun)(void), const char* graphName, double knownOptimalCost) {
    NamedGraph* namedGraph = creatorFun();

    if (!namedGraph) return;
    if (!namedGraph->g) { NamedGraphDestroy(&namedGraph); return; }

    unsigned int numVertices = GraphGetNumVertices(namedGraph->g);
    double actualBestCost = knownOptimalCost;
    Tour* heldKarpTour = NULL;

    if (numVertices <= 20 && knownOptimalCost == -1) {
        heldKarpTour = HeldKarp_FindTour(namedGraph->g);
        if (heldKarpTour) {
            TourMapCityNames(heldKarpTour, namedGraph);
            TourInvariant(heldKarpTour, numVertices);
            actualBestCost = heldKarpTour->cost;
        }
    }

    runTSPAlgorithms(namedGraph, graphName, actualBestCost, heldKarpTour);
    
    if (heldKarpTour) TourDestroy(&heldKarpTour);
    NamedGraphDestroy(&namedGraph);
}

static void runTSPAlgorithms(NamedGraph* namedGraph, const char* graphName, double actualCost, Tour* heldKarpTour) {
    if (!namedGraph) return;

    printf("\n\n\t\tTESTING GRAPH: %s\n", graphName);

    Graph* g = namedGraph->g;
    unsigned int numVertices = GraphGetNumVertices(g);

    Tour* nearestNeighbourTour = NULL;

    // Lower Bound (MST)
    double lowerBoundMST = LowerBound_MST(g);
    double lowerBoundHKLR = LowerBound_HeldKarp(g);
    printf("\t**LOWER BOUNDS: %.2f (MST) | %.2f (HKLR)**\n", lowerBoundMST, lowerBoundHKLR);
    if (actualCost != -1) printf("\t     Actual Best Cost: %.2f (%s)\n", actualCost, numVertices > 20 ? "by TSPLIB" : "by HK DP");

    // 0. Brute-Force (only for GraphGetNumVertices(ng->g) <= 10)
    executeDisplay(namedGraph, numVertices, (TSPAlgorithm){
        .tspFun = ExhaustiveSearch_Adapter, .name = "Brute-Force", .maxVertices = 10, .extra = NULL });

    // 0.5. Brute-Force w/ Pruning (only for GraphGetNumVertices(ng->g) <= 12)
    executeDisplay(namedGraph, numVertices, (TSPAlgorithm){
        .tspFun = ExhaustiveSearchPruning_Adapter, .name = "Brute-Force w/ Pruning", .maxVertices = 12, .extra = NULL });

    // 1. Held-Karp (only for GraphGetNumVertices(ng->g) <= 20)
    printf("\n[Held-Karp]\n");
    if (heldKarpTour) {
        TourMapCityNames(heldKarpTour, namedGraph);
        TourDisplay(heldKarpTour);
    } else printf("Disabled because graph has too many vertices (%u > %u) and it would be too slow.\n", numVertices, 20);

    // 2. Nearest Neighbour (called directly to keep nearestNeighbourTour)
    printf("\n[Nearest Neighbour Heuristic]\n");
    nearestNeighbourTour = NearestNeighbour_FindTour(g, 0);
    if (nearestNeighbourTour) {
        TourMapCityNames(nearestNeighbourTour, namedGraph);
        TourInvariant(nearestNeighbourTour, numVertices);
        TourDisplay(nearestNeighbourTour);
    }

    // 3. 2-Opt Improvement (based on Nearest Neighbour)
    Tour * twoOptTour = TourDeepCopy(nearestNeighbourTour);
    executeDisplay(namedGraph, numVertices, (TSPAlgorithm){
        .tspFun = TwoOpt_Adapter, .name = "2-Opt Improvement", .maxVertices = 0, .extra = twoOptTour });

    // 4. Greedy
    executeDisplay(namedGraph, numVertices, (TSPAlgorithm){
        .tspFun = Greedy_Adapter, .name = "Greedy Heuristic", .maxVertices = 0, .extra = NULL });

    // 5. Nearest Insertion
    executeDisplay(namedGraph, numVertices, (TSPAlgorithm){
        .tspFun = NearestInsertion_Adapter, .name = "Nearest Insertion Heuristic", .maxVertices = 0, .extra = NULL });

    // 6. Christofides
    executeDisplay(namedGraph, numVertices, (TSPAlgorithm){
        .tspFun = Christofides_Adapter, .name = "Christofides Algorithm", .maxVertices = 0, .extra = NULL });

    // 7. Simulated Annealing (based on Nearest Neighbour)
    unsigned int* initialTour = malloc((numVertices + 1) * sizeof(unsigned int));
    if (initialTour) {
        memcpy(initialTour, nearestNeighbourTour->path, (numVertices + 1) * sizeof(unsigned int));
        executeDisplay(namedGraph, numVertices, (TSPAlgorithm){
                .tspFun = SimAnnealing_Adapter, .name = "Simulated Annealing", .maxVertices = 0, .extra = initialTour });
        free(initialTour);
    }
    TourDestroy(&nearestNeighbourTour); // destroy previously left nearestNeighbourTour.

    // 8. Ant Colony
    executeDisplay(namedGraph, numVertices, (TSPAlgorithm){
        .tspFun = AntColony_Adapter, .name = "Ant Colony Optimization", .maxVertices = 0, .extra = NULL });

    // 9. Genetic Algorithm
    executeDisplay(namedGraph, numVertices, (TSPAlgorithm){
        .tspFun = GeneticAlgorithm_Adapter, .name = "Genetic Algorithm Optimization", .maxVertices = 55, .extra = NULL });
    
    printf("TESTING COMPLETE: %s\n", graphName);
}

// helper: runs and displays a Tour using the context provided by the ADT
static void executeDisplay(NamedGraph* namedGraph, unsigned int numVertices, TSPAlgorithm algorithm) {
    printf("\n[%s]\n", algorithm.name);

    if (algorithm.maxVertices > 0 && numVertices > algorithm.maxVertices) {
        printf("Disabled because graph has too many vertices (%u > %u) and it would be too slow.\n", numVertices, algorithm.maxVertices);
        return;
    }

    Tour* tour = algorithm.tspFun(namedGraph->g, algorithm.extra);
    if (!tour) return;

    TourMapCityNames(tour, namedGraph);
    TourInvariant(tour, numVertices);
    TourDisplay(tour);
    TourDestroy(&tour);
}