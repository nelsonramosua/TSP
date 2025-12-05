// GraphFactory.h - generates some graphs to test ADT implementations.
//
// Nelson Ramos, 124921.
//
// November, 2025.
// 
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

#ifndef GRAPH_FACTORY_H
#define GRAPH_FACTORY_H

#include "../TravelingSalesmanProblem.h"

NamedGraph* CreateGraphAula(void);
NamedGraph* CreatePortugal12CitiesGraph(void);
NamedGraph* CreateEurope12CitiesGraph(void);

NamedGraph* CreateMatrixGraph15(void);
NamedGraph* CreateMatrixGraph20(void);
NamedGraph* CreateEuclideanGraph15(void);

NamedGraph* CreateRandomEuclideanGraph(unsigned int N, double maxX, double maxY); // not being tested atm. do it!

// TSPLIB graphs
// see here: https://github.com/mastqe/tsplib/
NamedGraph* CreateEil51Graph(void);
NamedGraph* CreateOliver30Graph(void);
NamedGraph* CreateSwiss42Graph(void);
NamedGraph* CreateBays29Graph(void);
NamedGraph* CreateA280Graph(void);

// Add your own! See implementation for details!
// You can also read Graphs from files (see Graph ADT interface)!

#endif // GRAPH_FACTORY_H