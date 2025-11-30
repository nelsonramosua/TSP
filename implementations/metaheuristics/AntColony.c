// AntColony.c
#include "../../TravelingSalesmanProblem.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <time.h>

// Compute total tour cost
static double tourCost(const Graph* g, unsigned int* tour, unsigned int N) {
    double cost = 0.0;
    for (unsigned int i = 0; i < N; i++) {
        double w = GetEdgeWeight(g, tour[i], tour[(i + 1) % N]);
        if (w == DBL_MAX) return DBL_MAX;
        cost += w;
    }
    return cost;
}

// Probabilistic selection for next city
static unsigned int selectNextCity(unsigned int current, int* visited, 
                                   double** tau, double** eta, double alpha, double beta, unsigned int N) {
    double sum = 0.0;
    double probs[N];
    for (unsigned int j = 0; j < N; j++) {
        if (!visited[j]) {
            probs[j] = pow(tau[current][j], alpha) * pow(eta[current][j], beta);
            sum += probs[j];
        } else {
            probs[j] = 0.0;
        }
    }

    if (sum == 0.0) {
        // Pick first unvisited city
        for (unsigned int j = 0; j < N; j++) if (!visited[j]) return j;
    }

    double r = ((double)rand() / RAND_MAX) * sum;
    double cumulative = 0.0;
    for (unsigned int j = 0; j < N; j++) {
        cumulative += probs[j];
        if (!visited[j] && cumulative >= r) return j;
    }

    // Fallback (should not happen)
    for (unsigned int j = 0; j < N; j++) if (!visited[j]) return j;
    return 0;
}

Tour* AntColony_FindTour(const Graph* g) {
    unsigned int N = GraphGetNumVertices(g);
    if (N < 2) return NULL;

    Tour* bestTour = TourCreate(N);
    if (!bestTour) return NULL;

    srand((unsigned int)time(NULL));

    // ACO parameters
    unsigned int numAnts = N;
    unsigned int numIterations = 100;
    double alpha = 1.0;  // pheromone influence
    double beta = 5.0;   // heuristic influence
    double rho = 0.5;    // pheromone evaporation
    double Q = 100.0;    // pheromone deposit factor

    // Allocate pheromone and heuristic matrices
    double** tau = malloc(N * sizeof(double*));
    double** eta = malloc(N * sizeof(double*));
    for (unsigned int i = 0; i < N; i++) {
        tau[i] = malloc(N * sizeof(double));
        eta[i] = malloc(N * sizeof(double));
        for (unsigned int j = 0; j < N; j++) {
            if (i == j) {
                tau[i][j] = 0.0;
                eta[i][j] = 0.0;
            } else {
                double w = GetEdgeWeight(g, i, j);
                tau[i][j] = 1.0;              // initial pheromone
                eta[i][j] = (w > 0.0) ? 1.0 / w : 0.0; // heuristic = 1/distance
            }
        }
    }

    double bestCost = DBL_MAX;

    for (unsigned int iter = 0; iter < numIterations; iter++) {
        unsigned int antTours[numAnts][N];
        double antCosts[numAnts];

        // Each ant builds a tour
        for (unsigned int k = 0; k < numAnts; k++) {
            int visited[N];
            for (unsigned int i = 0; i < N; i++) visited[i] = 0;

            // Start city
            unsigned int current = rand() % N;
            antTours[k][0] = current;
            visited[current] = 1;

            for (unsigned int step = 1; step < N; step++) {
                unsigned int next = selectNextCity(current, visited, tau, eta, alpha, beta, N);
                antTours[k][step] = next;
                visited[next] = 1;
                current = next;
            }

            antCosts[k] = tourCost(g, antTours[k], N);

            if (antCosts[k] < bestCost) {
                bestCost = antCosts[k];
                memcpy(bestTour->path, antTours[k], N * sizeof(unsigned int));
            }
        }

        // Evaporate pheromones
        for (unsigned int i = 0; i < N; i++)
            for (unsigned int j = 0; j < N; j++)
                tau[i][j] *= (1.0 - rho);

        // Deposit pheromones based on ant tours
        for (unsigned int k = 0; k < numAnts; k++) {
            double delta = Q / antCosts[k];
            for (unsigned int i = 0; i < N; i++) {
                unsigned int a = antTours[k][i];
                unsigned int b = antTours[k][(i + 1) % N];
                tau[a][b] += delta;
                tau[b][a] += delta; // undirected
            }
        }
    }

    bestTour->path[N] = bestTour->path[0]; // close the cycle
    bestTour->cost = bestCost;

    // Free matrices
    for (unsigned int i = 0; i < N; i++) {
        free(tau[i]);
        free(eta[i]);
    }
    free(tau);
    free(eta);

    return bestTour;
}