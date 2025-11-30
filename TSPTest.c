#include "TravelingSalesmanProblem.h"
#include "GraphFactory.h"
#include <stdlib.h>
#include <stdio.h>

// Helper function: run all algorithms on a single graph
static void RunTSPAlgorithms(Graph* g, const char* graphName) {
    if (!g) return;

    printf("\n============================================\n");
    printf("      TESTING GRAPH: %s\n", graphName);
    printf("============================================\n");

    // Lower Bound (MST)
    double lower_bound = LowerBound_MST(g);
    printf("  **LOWER BOUND (MST Cost): %.2f**\n", lower_bound);
    printf("--------------------------------------------\n");

    /* disabled. turn on if the graphs you're testing have < 12 vertices.
    // 1. Brute-Force
    Tour* bf_tour = ExhaustiveSearch_FindTour(g);
    printf("\n[Brute-Force]\n");
    TourDisplay(bf_tour);
    */

    // 2. Nearest Neighbour Heuristic
    Tour* nn_tour = NearestNeighbour_FindTour(g, 0);
    printf("\n[Nearest Neighbour Heuristic]\n");
    TourDisplay(nn_tour);

    // 3. 2-Opt Improvement
    Tour* improved_tour = NULL;
    if (nn_tour) {
        improved_tour = TwoOpt_ImproveTour(g, nn_tour);
        printf("\n[2-Opt Improvement]\n");
        TourDisplay(improved_tour);
    }
    TourDestroy(&nn_tour);

    // 4. Greedy Heuristic
    Tour* greedy_tour = Greedy_FindTour(g);
    printf("\n[Greedy Heuristic]\n");
    TourDisplay(greedy_tour);

    // 5. Christofides Algorithm
    Tour* christofides_tour = Christofides_FindTour(g);
    printf("\n[Christofides Algorithm]\n");
    TourDisplay(christofides_tour);
    TourDestroy(&christofides_tour);

    // 6. Simulated Annealing
    unsigned int N = GraphGetNumVertices(g);
    unsigned int* initialTour = malloc(N * sizeof(unsigned int));
    if (initialTour && greedy_tour) {
        for (unsigned int i = 0; i < N; i++)
            initialTour[i] = greedy_tour->path[i];
        Tour* sa_tour = SimulatedAnnealing_FindTour(g, initialTour);
        printf("\n[Simulated Annealing]\n");
        TourDisplay(sa_tour);
        TourDestroy(&sa_tour);
    }
    free(initialTour);
    TourDestroy(&greedy_tour);

    // 7. Ant Colony Optimization
    Tour* aco_tour = AntColony_FindTour(g);
    printf("\n[Ant Colony Optimization]\n");
    TourDisplay(aco_tour);
    TourDestroy(&aco_tour);

    printf("\n============================================\n");
    printf("      TESTING COMPLETE: %s\n", graphName);
    printf("============================================\n\n");
}

int main(void) {
    printf("Running TSP Algorithm Comparison Suite...\n");

    // --- STATIC GRAPHS ---
    Graph* g1 = GraphFactory_CreateMatrixGraph15();
    RunTSPAlgorithms(g1, "Matrix Graph 15 Nodes");
    GraphDestroy(&g1);

    Graph* g2 = GraphFactory_CreateMatrixGraph20();
    RunTSPAlgorithms(g2, "Matrix Graph 20 Nodes");
    GraphDestroy(&g2);

    Graph* g3 = GraphFactory_CreateEuclideanGraph15();
    RunTSPAlgorithms(g3, "Euclidean Graph 15 Nodes");
    GraphDestroy(&g3);

    // --- RANDOM EUCLIDEAN GRAPHS ---
    for (int i = 0; i < 3; i++) {
        Graph* gr = GraphFactory_CreateRandomEuclideanGraph(10 + i*5, 100.0, 100.0);
        char name[64];
        snprintf(name, sizeof(name), "Random Euclidean Graph %u Nodes", GraphGetNumVertices(gr));
        RunTSPAlgorithms(gr, name);
        GraphDestroy(&gr);
    }

    printf("All graphs tested.\n");
    return 0;
}