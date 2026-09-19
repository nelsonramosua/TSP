// AntColony.c - Implements the Ant Colony Optimization (ACO) metaheuristic for TSP.
//
// O(N^2 * iterations * ants) = O(N^3 * iterations).
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Resources used:
// https://youtu.be/oXb2nC-e_EA?si=3G2oxIMb31RO8N8n
// https://www.researchgate.net/publication/324417976_Using_the_Bees_Algorithm_to_solve_a_stochastic_optimisation_problem

// Tune parameters in headers/Metaheuristics.h.

#include "../../TravelingSalesmanProblem.h"
#include "../../headers/Metaheuristics.h"
#include "../../headers/Trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <time.h>

// Forward declarations:

static unsigned int selectNextCity(unsigned int current, const int* visited, double** tau, double** eta, double alpha, double beta, unsigned int numVertices, double* probs);
static double tourCost(const Graph* g, const unsigned int* tour, unsigned int numVertices);

Tour* AntColony_FindTour(const Graph* g) {
    unsigned int numVertices = GraphGetNumVertices(g);
    if (numVertices < 2) return NULL;

    Tour* bestTour = TourCreate(numVertices);
    if (!bestTour) return NULL;

    // ACO parameters (change in Metaheuristics.h) (we could read MACROS direclty, but this is cleaner)
    unsigned int numAnts = numVertices;
    unsigned int numIterations = ACO_ITERATIONS;
    double alpha = ACO_ALPHA;  // pheromone influence
    double beta = ACO_BETA;   // heuristic influence
    double rho = ACO_RHO;    // pheromone evaporation rate
    double Q = ACO_Q;    // pheromone deposit factor

    // alloc pheromone and heuristic matrices
    double** tau = malloc(numVertices * sizeof(double*));
    double** eta = malloc(numVertices * sizeof(double*));
    if (!tau || !eta) { free(tau); free(eta); TourDestroy(&bestTour); return NULL; }
    for (unsigned int i = 0; i < numVertices; i++) {
        tau[i] = malloc(numVertices * sizeof(double));
        eta[i] = malloc(numVertices * sizeof(double));

        for (unsigned int j = 0; j < numVertices; j++) {
            if (i == j) { tau[i][j] = 0.0; eta[i][j] = 0.0; }
            else {
                double w = GetEdgeWeight(g, i, j);
                tau[i][j] = 1.0;              // init uniform pheromone
                eta[i][j] = (w > 0.0 && w != DBL_MAX) ? 1.0 / w : 0.0; // heuristic = 1/distance (for minimzation)
            }
        }
    }

    // Reusable scratch, allocated ONCE (not per iteration): the ant tours live in one contiguous pool (ant k occupies antTours[k*numVertices .. k*numVertices+numVertices-1]), and visited/probs are reset each ant instead of being re-created on the stack.
    unsigned int* antTours = malloc((size_t)numAnts * numVertices * sizeof(unsigned int));
    double* antCosts = malloc(numAnts * sizeof(double));
    int* visited = malloc(numVertices * sizeof(int));
    double* probs = malloc(numVertices * sizeof(double));
    if (!antTours || !antCosts || !visited || !probs) {
        free(antTours); free(antCosts); free(visited); free(probs);
        for (unsigned int i = 0; i < numVertices; i++) { free(tau[i]); free(eta[i]); }
        free(tau); free(eta); TourDestroy(&bestTour); return NULL;
    }

    double bestCost = DBL_MAX;

    for (unsigned int iter = 0; iter < numIterations; iter++) {
        // each ant builds a tour
        for (unsigned int k = 0; k < numAnts; k++) {
            unsigned int* antTour = &antTours[(size_t)k * numVertices];
            for (unsigned int i = 0; i < numVertices; i++) visited[i] = 0;

            // start ant at rand vertex
            unsigned int current = rand() % numVertices;
            antTour[0] = current;
            visited[current] = 1;

            // build rest of path (numVertices-1 steps)
            for (unsigned int step = 1; step < numVertices; step++) {
                unsigned int next = selectNextCity(current, visited, tau, eta, alpha, beta, numVertices, probs);
                antTour[step] = next;
                visited[next] = 1;
                current = next;
            }

            // calc cost of completed tour
            antCosts[k] = tourCost(g, antTour, numVertices);

            // update best tour
            if (antCosts[k] < bestCost) {
                bestCost = antCosts[k];
                memcpy(bestTour->path, antTour, numVertices * sizeof(unsigned int));
            }
        }

        // evaporate pheromones on all edgfes
        for (unsigned int i = 0; i < numVertices; i++)
            for (unsigned int j = 0; j < numVertices; j++)
                tau[i][j] *= (1.0 - rho);

        // deposit (new) pheromones based on how good ant tours wwere
        for (unsigned int k = 0; k < numAnts; k++) {
            const unsigned int* antTour = &antTours[(size_t)k * numVertices];
            // delta is the amount of pheromone deposited Q / tourCost
            double delta = Q / antCosts[k];

            for (unsigned int i = 0; i < numVertices; i++) {
                unsigned int a = antTour[i];
                unsigned int b = antTour[(i + 1) % numVertices];

                // deposit pheromones (undirected graph: deposit on both (u,v) and (v,u))
                tau[a][b] += delta;
                tau[b][a] += delta;
            }
        }
        TraceEmit(bestTour->path, numVertices, numVertices, bestCost); // frame: best tour after this iteration
    }

    bestTour->path[numVertices] = bestTour->path[0]; // close the cycle
    bestTour->cost = bestCost;

    // Free scratch and matrices
    free(antTours); free(antCosts); free(visited); free(probs);
    for (unsigned int i = 0; i < numVertices; i++) { free(tau[i]); free(eta[i]); }
    free(tau); free(eta);

    return bestTour;
}

// tau is pheromone level on each edge.
// eta is desirability/visibility of each edge (1/distance).
// alpha is pheromone influence factor.
// beta is heuristic influence factor.
// probs is a caller-owned scratch buffer of numVertices doubles (reused across calls, not allocated here).
static unsigned int selectNextCity(unsigned int current, const int* visited, double** tau, double** eta, double alpha, double beta, unsigned int numVertices, double* probs) {
    double sum = 0.0;

    // calc. product Pheromone^(alpha) * Heuristic^(beta) for unvisited neighbors
    for (unsigned int j = 0; j < numVertices; j++) {
        if (!visited[j]) { probs[j] = pow(tau[current][j], alpha) * pow(eta[current][j], beta); sum += probs[j];}
        else probs[j] = 0.0;
    }

    // if sum=0, it means no unvisited neighbors (shouldn't happen in a complete graph)
    // fallback? pick first unvisited vert.
    if (sum == 0.0) for (unsigned int j = 0; j < numVertices; j++) if (!visited[j]) return j;

    double r = ((double)rand() / RAND_MAX) * sum;
    double cumulative = 0.0;

    for (unsigned int j = 0; j < numVertices; j++) {
        cumulative += probs[j];
        // if rand value falls in current segment, select this vertex
        if (!visited[j] && cumulative >= r) return j;
    }

    // Fallback (should not happen)
    for (unsigned int j = 0; j < numVertices; j++) if (!visited[j]) return j;
    return 0; // return vertex 0 if all else fails...
}

static double tourCost(const Graph* g, const unsigned int* tour, unsigned int numVertices) {
    double cost = 0.0;
    // Iterate numVertices times (numVertices edges in the cycle)
    for (unsigned int i = 0; i < numVertices; i++) {
        // Edge is (tour[i], tour[(i + 1) % numVertices])
        double w = GetEdgeWeight(g, tour[i], tour[(i + 1) % numVertices]);
        if (w == DBL_MAX) return DBL_MAX; // invalid edge
        cost += w;
    }
    return cost;
}