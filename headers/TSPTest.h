// TSPTest.h - Test Driver header.
//
// Nelson Ramos, 124921
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit. 

#ifndef _TSP_TEST_H_
#define _TSP_TEST_H_

#include "../TravelingSalesmanProblem.h"

typedef struct {
    Tour* (*tspFun)(const Graph*, void*);  // generic function pointer to TSP implementation
    const char* name;                      // Name for display
    unsigned int maxVertices;              // Limit for slow algorithms
    void* extra;                           // Generic extra argument (Tour* or unsigned int* or NULL)
} TSPAlgorithm;

static void runTSPAlgorithms(NamedGraph* ng, const char* graphName, double actualCost, Tour* hk_tour);
static void testRunNamedGraph(NamedGraph* (*creatorFun)(void), const char* graphName, double knownOptimalCost);
static void executeDisplay(NamedGraph* ng, unsigned int numVertices, TSPAlgorithm alg);

// Abstraction adapters.
static Tour* ExhaustiveSearch_Adapter(const Graph* g, void* unused) {
    (void)unused;
    return ExhaustiveSearch_FindTour((Graph*)g);
}

static Tour* ExhaustiveSearchPruning_Adapter(const Graph* g, void* unused) {
    (void)unused;
    return ExhaustiveSearchPruning_FindTour((Graph*)g);
}

static Tour* TwoOpt_Adapter(const Graph* g, void* extra) {
    return TwoOpt_ImproveTour((Graph*)g, (Tour*)extra);
}

static Tour* SimAnnealing_Adapter(const Graph* g, void* extra) {
    return SimulatedAnnealing_FindTour((Graph*)g, (unsigned int*)extra);
}

static Tour* Greedy_Adapter(const Graph* g, void* unused) {
    (void)unused;
    return Greedy_FindTour((Graph*)g);
}

static Tour* NearestInsertion_Adapter(const Graph* g, void* unused) {
    (void)unused;
    return NearestInsertion_FindTour((Graph*)g);
}

static Tour* Christofides_Adapter(const Graph* g, void* unused) {
    (void)unused;
    return Christofides_FindTour((Graph*)g);
}

static Tour* AntColony_Adapter(const Graph* g, void* unused) {
    (void)unused;
    return AntColony_FindTour((Graph*)g);
}

static Tour* GeneticAlgorithm_Adapter(const Graph* g, void* unused) {
    (void)unused;
    return GeneticAlgorithm_FindTour((Graph*)g);
}

#endif // _TSP_TEST_H_