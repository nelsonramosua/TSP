// TSPTest.h - Test Driver header.
//
// Nelson Ramos, 124921
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

#ifndef _TSP_TEST_H_
#define _TSP_TEST_H_

#include "../TravelingSalesmanProblem.h"

typedef struct {
    NamedGraph* (*graphCreator)(void);     // generic function pointer to graph creation function
    const char* name;                      // Name for display
    double optimal;                        // optimal TSP solution
} GraphTestCase;

typedef struct {
    Tour* (*tspFun)(const Graph*, void*);  // generic function pointer to TSP implementation
    const char* name;                      // Name for display
    unsigned int maxVertices;              // Limit for slow algorithms

    void* extra;                           // Generic extra argument (Tour* or unsigned int* or NULL)
} TSPAlgorithm;

static void runTSPAlgorithms(NamedGraph* ng, const char* graphName, double actualCost, Tour* hk_tour);
static void testRunNamedGraph(GraphTestCase testCase);
static void executeDisplay(NamedGraph* ng, unsigned int numVertices, TSPAlgorithm alg);

// Abstraction adapters.
// Maybe should not have abstracted so much the test driver.... :)
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

static Tour* OrOpt_Adapter(const Graph* g, void* extra) {
    return OrOpt_ImproveTour((Graph*)g, (Tour*)extra);
}

static Tour* ThreeOpt_Adapter(const Graph* g, void* extra) {
    return ThreeOpt_ImproveTour((Graph*)g, (Tour*)extra);
}

static Tour* LinKernighan_Adapter(const Graph* g, void* extra) {
    return LinKernighan_ImproveTour((Graph*)g, (Tour*)extra);
}

static Tour* FarthestInsertion_Adapter(const Graph* g, void* unused) {
    (void)unused;
    return FarthestInsertion_FindTour((Graph*)g);
}

static Tour* ClarkeWright_Adapter(const Graph* g, void* unused) {
    (void)unused;
    return ClarkeWright_FindTour((Graph*)g);
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

static Tour* TabuSearch_Adapter(const Graph* g, void* unused) {
    (void)unused;
    return TabuSearch_FindTour((Graph*)g);
}

static Tour* GeneticAlgorithm_Adapter(const Graph* g, void* unused) {
    (void)unused;
    return GeneticAlgorithm_FindTour((Graph*)g);
}

#endif // _TSP_TEST_H_