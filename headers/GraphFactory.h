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

Graph* CreateGraphAula(void);

Graph* CreateMatrixGraph15(void);
Graph* CreateMatrixGraph20(void);
Graph* CreateEuclideanGraph15(void);

Graph* CreateRandomEuclideanGraph(unsigned int N, double maxX, double maxY);

// TSPLIB graphs
// see here: https://github.com/mastqe/tsplib/
Graph* CreateEil51Graph(void);
Graph* CreateOliver30Graph(void);
Graph* CreateSwiss42Graph(void);
Graph* CreateBays29Graph(void);
Graph* CreateA280Graph(void);

// Add your own!
// You can also read Graphs from files (see Graph ADT interface)!

#endif // GRAPH_FACTORY_H