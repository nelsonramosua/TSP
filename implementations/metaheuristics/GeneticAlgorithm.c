// GeneticAlgorithm.c - Implements the Genetic Algorithm (GA) metaheuristic for TSP.
//
// O(N^2 * Generations * Population)
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Updates:
// 1. Nelson Ramos, september 2026:
//      Reduced memory overhead -> more performant.

// Resources used (severely... :') ):
// https://towardsdatascience.com/solving-the-travelling-salesman-problem-using-a-genetic-algorithm-c3e87f37f1de/
// https://github.com/hassanzadehmahdi/Traveling-Salesman-Problem-using-Genetic-Algorithm
// https://www.geeksforgeeks.org/dsa/traveling-salesman-problem-using-genetic-algorithm/

// Memory note: earlier this was by far the heaviest allocator in the project, doing millions of malloc/free cycles (a temp Tour per fitness eval, deep-copied parents per selection, and a full population rebuilt every generation).
// It now:
//   - computes cost directly over the path (no temp Tour allocation),
//   - selects parents by pointer into the population (no copies),
//   - ping-pongs between two pre-allocated populations (no per-generation path alloc/free), plus a single reused scratch buffer for crossover,
//   - stores each population's paths in one contiguous pool (a small arena) rather than a malloc per individual.
// Allocations are now O(population) for the whole run instead of O(population * generations).
// The search itself is unchanged (identical number and order of rand() calls).
// The population sort was also given a deterministic tie-break (see compareIndividuals), without which reusing buffers could order equal-cost tours differently from the old fresh-malloc version.
// Verified: with a fixed seed this produces output bit-identical to the previous implementation (+ the same tie-break).

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/Metaheuristics.h"
#include "../../headers/Trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <string.h>

// Tune parameters in headers/Metaheuristics.h.

// Represents an individual solution (chromosome) in the population
typedef struct Individual {
    unsigned int* path; // tour permutation
    double cost;        // tour cost
    double fitness;     // inverse cost (1 / cost)
} Individual;

// Represents the entire population
typedef struct Population {
    Individual* individuals;
    unsigned int* pathPool;     // one contiguous block backing every individual's path
    unsigned int numIndividuals;
} Population;

// Forward Declarations
static Population* AllocPopulation(unsigned int numVertices);
static Population* InitializePopulation(unsigned int numVertices);
static void CalculateFitness(const Graph* g, Population* pop);
static const Individual* SelectParent(const Population* pop);
static void Mutate(Individual* individual, unsigned int numVertices);
static void Crossover(const Individual* parent1, const Individual* parent2,
                      unsigned int numVertices, unsigned int* dest, int* isUsed);
static void SortPopulation(Population* pop, unsigned int numVertices);
static void DestroyPopulation(Population* pop);

Tour* GeneticAlgorithm_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices < 2) return NULL;

    // init Population
    Population* population = InitializePopulation(numVertices);
    if (!population) return NULL;

    // second, pre-allocated population we ping-pong into each generation (paths are overwritten every generation, so their initial contents don't matter).
    Population* bufferPop = AllocPopulation(numVertices);
    if (!bufferPop) { DestroyPopulation(population); return NULL; }

    // reused scratch buffer for Order Crossover (avoids a calloc per offspring).
    int* isUsed = malloc(numVertices * sizeof(int));
    if (!isUsed) { DestroyPopulation(population); DestroyPopulation(bufferPop); return NULL; }

    CalculateFitness(g, population);
    SortPopulation(population, numVertices);

    // track best tour found (own buffer, reused via memcpy on improvement)
    Individual bestOverall;
    bestOverall.path = malloc(numVertices * sizeof(unsigned int));
    if (!bestOverall.path) { free(isUsed); DestroyPopulation(population); DestroyPopulation(bufferPop); return NULL; }
    memcpy(bestOverall.path, population->individuals[0].path, numVertices * sizeof(unsigned int));
    bestOverall.cost = population->individuals[0].cost;
    bestOverall.fitness = population->individuals[0].fitness;

    // evolution loop
    for (unsigned int generation = 0; generation < GA_NUM_GENERATIONS; generation++) {
        // elitism (keep best individuals from current pop)
        for (unsigned int i = 0; i < GA_ELITISM_COUNT; i++)
            memcpy(bufferPop->individuals[i].path, population->individuals[i].path,
                   numVertices * sizeof(unsigned int));

        // crossover & mutation for rest of new pop
        for (unsigned int i = GA_ELITISM_COUNT; i < GA_POPULATION_SIZE; i++) {
            // selection (Tournament Selection) -- pointers into the current pop, no copies
            const Individual* parent1 = SelectParent(population);
            const Individual* parent2 = SelectParent(population);

            // crossover writes straight into the pre-allocated offspring buffer
            Individual* offspring = &bufferPop->individuals[i];
            Crossover(parent1, parent2, numVertices, offspring->path, isUsed);

            // mutation
            if (((double)rand() / RAND_MAX) < GA_MUTATION_RATE) Mutate(offspring, numVertices);
        }

        // eval new population and sort, then swap buffers (no free/realloc of paths)
        CalculateFitness(g, bufferPop);
        SortPopulation(bufferPop, numVertices);

        Population* tmp = population; population = bufferPop; bufferPop = tmp;

        // update overall best solution
        if (population->individuals[0].cost < bestOverall.cost) {
            memcpy(bestOverall.path, population->individuals[0].path, numVertices * sizeof(unsigned int));
            bestOverall.cost = population->individuals[0].cost;
            bestOverall.fitness = population->individuals[0].fitness;
        }
        TraceEmit(bestOverall.path, numVertices, numVertices, bestOverall.cost); // frame: best-so-far each generation
    }

    // finalize and return
    Tour* finalTour = TourCreate(numVertices);
    if (!finalTour) {
        free(bestOverall.path); free(isUsed);
        DestroyPopulation(population); DestroyPopulation(bufferPop);
        return NULL;
    }

    for (unsigned int i = 0; i < numVertices; i++) finalTour->path[i] = bestOverall.path[i];
    finalTour->path[numVertices] = finalTour->path[0]; // close cycle
    finalTour->cost = bestOverall.cost;

    free(bestOverall.path); free(isUsed);
    DestroyPopulation(population); DestroyPopulation(bufferPop);

    return finalTour;
}


// Allocates a population.
// All paths live in a single contiguous pool (a small arena): individual i's path is a slice of it.
// This turns ~POPULATION_SIZE mallocs into one and keeps the paths cache-friendly.
// Paths are left uninitialized; callers fill them.
static Population* AllocPopulation(unsigned int numVertices) {
    Population* pop = malloc(sizeof(Population));
    if (!pop) return NULL;

    pop->numIndividuals = GA_POPULATION_SIZE;
    pop->individuals = malloc(GA_POPULATION_SIZE * sizeof(Individual));
    pop->pathPool = malloc((size_t)GA_POPULATION_SIZE * numVertices * sizeof(unsigned int));
    if (!pop->individuals || !pop->pathPool) {
        free(pop->individuals); free(pop->pathPool); free(pop);
        return NULL;
    }

    for (unsigned int i = 0; i < GA_POPULATION_SIZE; i++) {
        pop->individuals[i].path = pop->pathPool + (size_t)i * numVertices;
        pop->individuals[i].cost = DBL_MAX;
        pop->individuals[i].fitness = 0.0;
    }
    return pop;
}

// Initializes population with random, valid tours
static Population* InitializePopulation(unsigned int numVertices) {
    Population* pop = AllocPopulation(numVertices);
    if (!pop) return NULL;

    for (unsigned int i = 0; i < GA_POPULATION_SIZE; i++) {
        // fill path w/ vertices 0 to numVertices-1
        for (unsigned int j = 0; j < numVertices; j++) pop->individuals[i].path[j] = j;

        // shuffle path to create rand tour (Fisher-Yates)
        // https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
        for (unsigned int j = numVertices - 1; j > 0; j--) {
            unsigned int k = rand() % (j + 1);
            unsigned int temp = pop->individuals[i].path[j];
            pop->individuals[i].path[j] = pop->individuals[i].path[k];
            pop->individuals[i].path[k] = temp;
        }
    }
    return pop;
}


// Calcs cost & fitness of individuals in population (cost computed directly over the path, closing the cycle via wraparound -- no temp Tour allocation).
static void CalculateFitness(const Graph* g, Population* pop) {
    unsigned int numVertices = GraphGetNumVertices(g);
    for (unsigned int i = 0; i < pop->numIndividuals; i++) {
        Individual* ind = &pop->individuals[i];
        double totalCost = 0.0;
        int reachable = 1;

        for (unsigned int j = 0; j < numVertices; j++) {
            unsigned int vA = ind->path[j];
            unsigned int vB = ind->path[(j + 1) % numVertices]; // wraparound closes the tour
            double edgeCost = GetEdgeWeight(g, vA, vB);
            if (edgeCost == DBL_MAX) { totalCost = DBL_MAX; reachable = 0; break; }
            totalCost += edgeCost;
        }

        ind->cost = totalCost;
        // fitness is inverse of cost (lower cost - higher fitness)
        if (reachable && totalCost > 0 && totalCost != DBL_MAX) ind->fitness = 1.0 / totalCost;
        else ind->fitness = 0.0;
    }
}

// Selects a parent w/ Tournament Selection.
// Returns a pointer into the population (read-only), so no allocation/copy is needed.
static const Individual* SelectParent(const Population* pop) {
    const Individual* best = NULL;

    for (unsigned int i = 0; i < GA_TOURNAMENT_SIZE; i++) {
        unsigned int randomIndex = rand() % pop->numIndividuals;
        const Individual* current = &pop->individuals[randomIndex];

        if (best == NULL || current->cost < best->cost) best = current;
    }

    return best;
}

// Creates new individual (offspring) from two parents using Order Crossover (OX), writing the permutation into dest.
// isUsed is a caller-provided scratch buffer of size numVertices (reused across calls to avoid per-offspring allocation).
static void Crossover(const Individual* parent1, const Individual* parent2,
                      unsigned int numVertices, unsigned int* dest, int* isUsed) {
    // choose 2 rand cut points (pt1 and pt2)
    unsigned int pt1 = rand() % numVertices;
    unsigned int pt2 = rand() % numVertices;
    if (pt1 > pt2) { unsigned int temp = pt1; pt1 = pt2; pt2 = temp; }
    if (pt1 == pt2) { pt2 = (pt1 + 1) % numVertices; }

    // copy central segment from parent1 to offspring
    for (unsigned int i = pt1; i <= pt2; i++) dest[i] = parent1->path[i];

    // fill remaining spots by order of genes in parent2
    memset(isUsed, 0, numVertices * sizeof(int));
    for (unsigned int i = pt1; i <= pt2; i++) isUsed[dest[i]] = 1;

    unsigned int currentOffspringIndex = (pt2 + 1) % numVertices;
    for (unsigned int i = 0; i < numVertices; i++) {
        unsigned int city = parent2->path[i];
        if (!isUsed[city]) {
            dest[currentOffspringIndex] = city;
            currentOffspringIndex = (currentOffspringIndex + 1) % numVertices;
        }
    }
}

// Does Swap Mutation (rand swaps 2 genes (cities) in path).
static void Mutate(Individual* individual, unsigned int numVertices) {
    unsigned int index1 = rand() % numVertices;
    unsigned int index2 = rand() % numVertices;

    if (index1 == index2) return;

    // swap cities (vertices)
    unsigned int temp = individual->path[index1];
    individual->path[index1] = individual->path[index2];
    individual->path[index2] = temp;
}

// Utilities

// Number of vertices, shared with the comparator for deterministic tie-breaking.
static unsigned int g_cmpNumVertices = 0;

// Compare individuals by cost, breaking ties deterministically by the path itself.
// qsort is not stable, so without a total order the post-sort arrangement of equal-cost tours would depend on their memory layout, making the run non-reproducible (and sensitive to how the population is allocated).
static int compareIndividuals(const void* a, const void* b) {
    // must receive const void* because that's what qsort uses...
    const Individual* indA = (const Individual*)a;
    const Individual* indB = (const Individual*)b;

    if (indA->cost < indB->cost) return -1;
    if (indA->cost > indB->cost) return 1;

    // tie-break: lexicographic order on the path (deterministic, layout-independent)
    for (unsigned int i = 0; i < g_cmpNumVertices; i++) {
        if (indA->path[i] < indB->path[i]) return -1;
        if (indA->path[i] > indB->path[i]) return 1;
    }
    return 0; // identical tours
}

// Sorts population by cost (compare function above...)
static void SortPopulation(Population* pop, unsigned int numVertices) {
    g_cmpNumVertices = numVertices; // for the comparator's deterministic tie-break
    qsort(pop->individuals, pop->numIndividuals, sizeof(Individual), compareIndividuals);
}

static void DestroyPopulation(Population* pop) {
    if (pop) {
        // paths are slices of pathPool, so they are freed as a single block (not one by one)
        free(pop->pathPool);
        free(pop->individuals);
        free(pop);
    }
}