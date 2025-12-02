// GraphFactory.h - generates some graphs to test ADT implementations.
//
// Nelson Ramos, 124921.
//
// November, 2025.
// 
// You may freely use and change this code, it has no warranty, and it is not necessary to keep my credit. 

#ifndef GRAPH_FACTORY_H
#define GRAPH_FACTORY_H

#include "../TravelingSalesmanProblem.h"

Graph* GraphFactory_CreateMatrixGraph15(void);
Graph* GraphFactory_CreateMatrixGraph20(void);
Graph* GraphFactory_CreateEuclideanGraph15(void);
Graph* GraphFactory_CreateRandomEuclideanGraph(unsigned int N, double maxX, double maxY);

// tsplib graphs
// see here: https://github.com/mastqe/tsplib/
Graph* GraphFactory_CreateEil51Graph(void);
Graph* GraphFactory_CreateOliver30Graph(void);
Graph* GraphFactory_CreateSwiss42Graph(void);
Graph* GraphFactory_CreateBays29Graph(void);
Graph* GraphFactory_CreateA280Graph(void);

// Add your own!
// You can also read Graphs from files (see Graph ADT interface)!

#endif // GRAPH_FACTORY_H