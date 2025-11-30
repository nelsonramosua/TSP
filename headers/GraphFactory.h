#ifndef GRAPH_FACTORY_H
#define GRAPH_FACTORY_H

#include "../TravelingSalesmanProblem.h"

// Predefined / matrix graphs
Graph* GraphFactory_CreateMatrixGraph15(void);
Graph* GraphFactory_CreateMatrixGraph20(void);

// Euclidean / coordinate-based graphs
Graph* GraphFactory_CreateEuclideanGraph15(void);

// Optionally: random Euclidean graph generator
Graph* GraphFactory_CreateRandomEuclideanGraph(unsigned int N, double maxX, double maxY);

#endif // GRAPH_FACTORY_H