// GeneticAlgorithm.c - Implements the Genetic Algorithm (GA) metaheuristic for TSP.
// 
// O(N^2 * Generations * Population)
//
// Nelson Ramos, 124921.
//
// November, 2025.
// 
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit. 

// Resources used (severely... :') ):
// https://towardsdatascience.com/solving-the-travelling-salesman-problem-using-a-genetic-algorithm-c3e87f37f1de/
// https://github.com/hassanzadehmahdi/Traveling-Salesman-Problem-using-Genetic-Algorithm
// https://www.geeksforgeeks.org/dsa/traveling-salesman-problem-using-genetic-algorithm/

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/Metaheuristics.h"

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
    unsigned int numIndividuals;
} Population;

// Forward Declarations
static Population* InitializePopulation(unsigned int numVertices);
static void CalculateFitness(const Graph* g, Population* pop);
static Individual SelectParent(const Population* pop, unsigned int numVertices);
static void Mutate(Individual* individual, unsigned int numVertices);
static Individual Crossover(const Individual* parent1, const Individual* parent2, unsigned int numVertices);
static void SortPopulation(Population* pop);
static void DestroyIndividual(Individual* ind);
static void DestroyPopulation(Population* pop);
static Individual CopyIndividual(const Individual* original, unsigned int numVertices);
static double calculateTourCost(const Graph* g, const Tour* tour);

Tour* GeneticAlgorithm_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices < 2) return NULL;
    
    srand(time(NULL));

    // init Population
    Population* population = InitializePopulation(numVertices);
    if (!population) return NULL;
    
    CalculateFitness(g, population);
    SortPopulation(population);

    // track best tour found
    Individual bestOverall = CopyIndividual(&(population->individuals[0]), numVertices);

    // evolution loop
    for (unsigned int generation = 0; generation < GA_NUM_GENERATIONS; generation++) {
        Population* newPopulation = malloc(sizeof(Population));
        newPopulation->numIndividuals = GA_POPULATION_SIZE;
        newPopulation->individuals = malloc(GA_POPULATION_SIZE * sizeof(Individual));
        
        // elitism (keep best individuals from current pop)
        for (unsigned int i = 0; i < GA_ELITISM_COUNT; i++) {
            newPopulation->individuals[i] = CopyIndividual(&(population->individuals[i]), numVertices);
        }
        
        // crossover & mutation for rest of new pop
        for (unsigned int i = GA_ELITISM_COUNT; i < GA_POPULATION_SIZE; i++) {
            // selection (Tournament Selection)
            Individual parent1 = SelectParent(population, numVertices);
            Individual parent2 = SelectParent(population, numVertices);
            
            // crossover
            Individual offspring = Crossover(&parent1, &parent2, numVertices);
            
            // mutation
            if (((double)rand() / RAND_MAX) < GA_MUTATION_RATE) {
                Mutate(&offspring, numVertices);
            }
            
            // cleanup temp parent copies
            DestroyIndividual(&parent1);
            DestroyIndividual(&parent2);
            
            newPopulation->individuals[i] = offspring;
        }

        // eval new population and sort
        DestroyPopulation(population); 
        population = newPopulation;
        CalculateFitness(g, population);
        SortPopulation(population);

        // update overall best solution
        if (population->individuals[0].cost < bestOverall.cost) {
            DestroyIndividual(&bestOverall);
            bestOverall = CopyIndividual(&(population->individuals[0]), numVertices);
        }
    }

    // finalize and return
    Tour* finalTour = TourCreate(numVertices);
    if (!finalTour) { DestroyPopulation(population); DestroyIndividual(&bestOverall); return NULL; }
    
    for (unsigned int i = 0; i < numVertices; i++) {
        finalTour->path[i] = bestOverall.path[i];
    }
    finalTour->path[numVertices] = finalTour->path[0]; // close cycle
    finalTour->cost = bestOverall.cost;

    DestroyPopulation(population);
    DestroyIndividual(&bestOverall);

    return finalTour;
}


// Initializes population with random, valid tours
static Population* InitializePopulation(unsigned int numVertices) {
    Population* pop = malloc(sizeof(Population));
    if (!pop) return NULL;

    pop->numIndividuals = GA_POPULATION_SIZE;
    pop->individuals = malloc(GA_POPULATION_SIZE * sizeof(Individual));
    if (!pop->individuals) { free(pop); return NULL; }

    for (unsigned int i = 0; i < GA_POPULATION_SIZE; i++) {
        pop->individuals[i].path = malloc(numVertices * sizeof(unsigned int));
        if (!pop->individuals[i].path) return NULL; 
        
        // fill path w/ vertices 0 to numVertices-1
        for (unsigned int j = 0; j < numVertices; j++) {
            pop->individuals[i].path[j] = j;
        }

        // shuffle path to create rand tour (Fisher-Yates)
        // https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
        for (unsigned int j = numVertices - 1; j > 0; j--) {
            unsigned int k = rand() % (j + 1);
            unsigned int temp = pop->individuals[i].path[j];
            pop->individuals[i].path[j] = pop->individuals[i].path[k];
            pop->individuals[i].path[k] = temp;
        }
        
        pop->individuals[i].cost = DBL_MAX; // calc later
        pop->individuals[i].fitness = 0.0;
    }
    return pop;
}


// Calcs cost & fitness of individuals in population.
static void CalculateFitness(const Graph* g, Population* pop) {
    for (unsigned int i = 0; i < pop->numIndividuals; i++) {
        // create a temp Tour for cost calc
        Tour tempTour;
        tempTour.numVertices = GraphGetNumVertices(g) + 1; // numVertices + closing edge
        tempTour.path = malloc(tempTour.numVertices * sizeof(unsigned int));
        
        for(unsigned int j = 0; j < GraphGetNumVertices(g); j++) {
            tempTour.path[j] = pop->individuals[i].path[j];
        }
        tempTour.path[GraphGetNumVertices(g)] = pop->individuals[i].path[0];
        tempTour.cost = DBL_MAX; // placeholder (calc after)
        
        pop->individuals[i].cost = calculateTourCost(g, &tempTour);
        
        // fitness is inverse of cost (lower cost - higher fitness)
        if (pop->individuals[i].cost > 0 && pop->individuals[i].cost != DBL_MAX) {
            pop->individuals[i].fitness = 1.0 / pop->individuals[i].cost;
        } else {
            pop->individuals[i].fitness = 0.0;
        }
        
        free(tempTour.path);
    }
}

// Selects parent w/ Tournament Selection.
static Individual SelectParent(const Population* pop, unsigned int numVertices) {
    Individual best = {NULL, DBL_MAX, 0.0};
    
    for (unsigned int i = 0; i < GA_TOURNAMENT_SIZE; i++) {
        unsigned int randomIndex = rand() % pop->numIndividuals;
        Individual current = pop->individuals[randomIndex];
        
        if (current.cost < best.cost) {
            // new best individual in tournament
            if (best.path) DestroyIndividual(&best); 
            best = CopyIndividual(&current, numVertices);        
        }
    }
    
    return best;
}

// Creates new individual (offspring) from two parents using Order Crossover (OX).
static Individual Crossover(const Individual* parent1, const Individual* parent2, unsigned int numVertices) {
    Individual offspring;
    offspring.path = malloc(numVertices * sizeof(unsigned int));
    offspring.cost = DBL_MAX;
    offspring.fitness = 0.0;

    // choose 2 rand cut points (p1 and p2)
    unsigned int p1 = rand() % numVertices;
    unsigned int p2 = rand() % numVertices;
    if (p1 > p2) { unsigned int temp = p1; p1 = p2; p2 = temp; }
    if (p1 == p2) { p2 = (p1 + 1) % numVertices; } 

    // copy central segment from p1 to offspring
    for (unsigned int i = p1; i <= p2; i++) {
        offspring.path[i] = parent1->path[i];
    }
    
    // fill remaining spots by order of genes in p2
    int* isUsed = (int*) calloc(numVertices, sizeof(int));
    for (unsigned int i = p1; i <= p2; i++) {
        isUsed[offspring.path[i]] = 1;
    }

    unsigned int currentOffspringIndex = (p2 + 1) % numVertices;
    for (unsigned int i = 0; i < numVertices; i++) {
        unsigned int city = parent2->path[i];
        if (!isUsed[city]) {
            offspring.path[currentOffspringIndex] = city;
            currentOffspringIndex = (currentOffspringIndex + 1) % numVertices;
        }
    }
    
    free(isUsed);
    return offspring;
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

// Compare individuals.
static int compareIndividuals(const void* a, const void* b) {
    // must receive const void* because that's what qsort uses...
    const Individual* indA = (const Individual*)a;
    const Individual* indB = (const Individual*)b;
    
    if (indA->cost < indB->cost) return -1;
    if (indA->cost > indB->cost) return 1;
    return 0; // Equal
}

// Sorts population by cost (compare function above...)
static void SortPopulation(Population* pop) {
    qsort(pop->individuals, pop->numIndividuals, sizeof(Individual), compareIndividuals);
}

// Creates deep copy of Individual.
static Individual CopyIndividual(const Individual* original, unsigned int numVertices) {
    Individual copy;
    copy.path = malloc(numVertices * sizeof(unsigned int));
    memcpy(copy.path, original->path, numVertices * sizeof(unsigned int));
    copy.cost = original->cost;
    copy.fitness = original->fitness;
    return copy;
}

static void DestroyIndividual(Individual* ind) {
    if (ind->path) {
        free(ind->path);
        ind->path = NULL;
    }
    ind->cost = DBL_MAX;
    ind->fitness = 0.0;
}

static void DestroyPopulation(Population* pop) {
    if (pop) {
        if (pop->individuals) {
            for (unsigned int i = 0; i < pop->numIndividuals; i++) {
                DestroyIndividual(&(pop->individuals[i]));
            }
            free(pop->individuals);
        }
        free(pop);
    }
}

// Calcs cost of tour
static double calculateTourCost(const Graph* g, const Tour* tour) {
    double totalCost = 0.0;
    unsigned int numEdges = tour->numVertices - 1;

    for (unsigned int i = 0; i < numEdges; i++) {
        unsigned int vA = tour->path[i];
        unsigned int vB = tour->path[i + 1];
        double edgeCost = GetEdgeWeight(g, vA, vB);

        if (edgeCost == DBL_MAX) return DBL_MAX;

        totalCost += edgeCost;
    }

    return totalCost;
}