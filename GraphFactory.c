// GraphFactory.c - Creates some test graphs.
//
// Nelson Ramos, 124921. 
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit.

// Generated with help from AI!

#include "headers/GraphFactory.h"
#include "headers/Graph.h"
#include <stdlib.h>
#include <math.h>

// helper to write graph automatically to /graphs/<name>.dot
static void _writeDOT(const Graph* g, const char* name) {
    char path[256];
    snprintf(path, sizeof(path), "graphs/%s.dot", name); // try to avoid buffer overflow...
    GraphWriteDOT(g, path);
}

Graph* GraphFactory_CreateMatrixGraph15(void) {
    unsigned int numVertices = 15;
    Graph* g = GraphCreate(numVertices, 0, 1);
    if (!g) return NULL;

    double w[15][15] = {
        {0, 12, 7, 23, 9, 34, 15, 28, 18, 14, 30, 16, 25, 20, 10},
        {12, 0, 8, 17, 11, 22, 13, 19, 21, 25, 29, 14, 18, 16, 12},
        {7, 8, 0, 15, 14, 20, 12, 17, 22, 19, 24, 10, 16, 18, 11},
        {23, 17, 15, 0, 9, 11, 8, 20, 12, 25, 30, 14, 19, 21, 17},
        {9, 11, 14, 9, 0, 7, 16, 18, 22, 15, 20, 12, 13, 10, 8},
        {34, 22, 20, 11, 7, 0, 14, 19, 17, 25, 12, 16, 18, 15, 13},
        {15, 13, 12, 8, 16, 14, 0, 6, 19, 21, 23, 17, 14, 12, 9},
        {28, 19, 17, 20, 18, 19, 6, 0, 11, 15, 14, 12, 13, 16, 10},
        {18, 21, 22, 12, 22, 17, 19, 11, 0, 9, 13, 14, 15, 12, 8},
        {14, 25, 19, 25, 15, 25, 21, 15, 9, 0, 10, 11, 18, 14, 12},
        {30, 29, 24, 30, 20, 12, 23, 14, 13, 10, 0, 7, 16, 19, 15},
        {16, 14, 10, 14, 12, 16, 17, 12, 14, 11, 7, 0, 8, 10, 9},
        {25, 18, 16, 19, 13, 18, 14, 13, 15, 18, 16, 8, 0, 5, 12},
        {20, 16, 18, 21, 10, 15, 12, 16, 12, 14, 19, 10, 5, 0, 11},
        {10, 12, 11, 17, 8, 13, 9, 10, 8, 12, 15, 9, 12, 11, 0}
    };

    for (unsigned int vert1 = 0; vert1 < numVertices; vert1++)
        for (unsigned int j = vert1 + 1; j < numVertices; j++)
            GraphAddWeightedEdge(g, vert1, j, w[vert1][j]);
    
    _writeDOT(g, "MatrixGraph15");

    return g;
}

Graph* GraphFactory_CreateMatrixGraph20(void) {
    unsigned int numVertices = 20;
    Graph* g = GraphCreate(numVertices, 0, 1);
    if (!g) return NULL;

    double w[20][20] = {
        {0,2,4,6,7,5,8,10,9,11,12,14,13,15,16,17,18,19,20,21},
        {2,0,2,4,5,3,6,8,7,9,10,12,11,13,14,15,16,17,18,19},
        {4,2,0,2,3,1,4,6,5,7,8,10,9,11,12,13,14,15,16,17},
        {6,4,2,0,1,3,2,4,3,5,6,8,7,9,10,11,12,13,14,15},
        {7,5,3,1,0,2,1,3,2,4,5,7,6,8,9,10,11,12,13,14},
        {5,3,1,3,2,0,2,4,3,5,6,8,7,9,10,11,12,13,14,15},
        {8,6,4,2,1,2,0,2,1,3,4,6,5,7,8,9,10,11,12,13},
        {10,8,6,4,3,4,2,0,1,2,4,5,6,7,8,9,10,11,12,13},
        {9,7,5,3,2,3,1,1,0,1,3,4,5,6,7,8,9,10,11,12},
        {11,9,7,5,4,5,3,2,1,0,2,3,4,5,6,7,8,9,10,11},
        {12,10,8,6,5,6,4,4,3,2,0,2,3,4,5,6,7,8,9,10},
        {14,12,10,8,7,8,6,5,4,3,2,0,1,2,3,4,5,6,7,8},
        {13,11,9,7,6,7,5,6,5,4,3,1,0,1,2,3,4,5,6,7},
        {15,13,11,9,8,9,7,7,6,5,4,2,1,0,1,2,3,4,5,6},
        {16,14,12,10,9,10,8,8,7,6,5,3,2,1,0,1,2,3,4,5},
        {17,15,13,11,10,11,9,9,8,7,6,4,3,2,1,0,1,2,3,4},
        {18,16,14,12,11,12,10,10,9,8,7,5,4,3,2,1,0,1,2,3},
        {19,17,15,13,12,13,11,11,10,9,8,6,5,4,3,2,1,0,1,2},
        {20,18,16,14,13,14,12,12,11,10,9,7,6,5,4,3,2,1,0,1},
        {21,19,17,15,14,15,13,13,12,11,10,8,7,6,5,4,3,2,1,0}
    };

    for (unsigned int i = 0; i < numVertices; i++)
        for (unsigned int j = i + 1; j < numVertices; j++)
            GraphAddWeightedEdge(g, i, j, w[i][j]);

    _writeDOT(g, "MatrixGraph20");

    return g;
}

// ---------------------- EUCLIDEAN GRAPHS ----------------------

Graph* GraphFactory_CreateEuclideanGraph15(void) {
    unsigned int numVertices = 15;
    Graph* g = GraphCreate(numVertices, 0, 1);
    if (!g) return NULL;

    double coords[15][2] = {
        {0,0}, {2,0}, {1,1},
        {10,0}, {12,0}, {11,1},
        {0,10}, {2,10}, {1,11},
        {10,10}, {12,10}, {11,11},
        {5,5}, {6,5}, {5.5,6}
    };

    for (unsigned int i = 0; i < numVertices; i++)
        for (unsigned int j = i + 1; j < numVertices; j++) {
            double dx = coords[i][0] - coords[j][0];
            double dy = coords[i][1] - coords[j][1];
            GraphAddWeightedEdge(g, i, j, sqrt(dx*dx + dy*dy));
        }

    _writeDOT(g, "EuclideanGraph15");

    return g;
}

// ---------------------- RANDOM EUCLIDEAN GRAPHS ----------------------

Graph* GraphFactory_CreateRandomEuclideanGraph(unsigned int numVertices, double maxX, double maxY) {
    Graph* g = GraphCreate(numVertices, 0, 1);
    if (!g) return NULL;

    double* x = malloc(numVertices * sizeof(double));
    double* y = malloc(numVertices * sizeof(double));
    if (!x || !y) { free(x); free(y); GraphDestroy(&g); return NULL; }

    for (unsigned int i = 0; i < numVertices; i++) {
        x[i] = ((double)rand() / RAND_MAX) * maxX;
        y[i] = ((double)rand() / RAND_MAX) * maxY;
    }

    for (unsigned int i = 0; i < numVertices; i++)
        for (unsigned int j = i + 1; j < numVertices; j++) {
            double dx = x[i] - x[j];
            double dy = y[i] - y[j];
            GraphAddWeightedEdge(g, i, j, sqrt(dx*dx + dy*dy));
        }

    free(x); free(y);

    _writeDOT(g, "RandomEuclideanGraph");
    
    return g;
}