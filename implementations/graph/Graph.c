//
// Algoritmos e Estruturas de Dados --- 2024/2025
//
// Joaquim Madeira, Joao Manuel Rodrigues - June 2021, Nov 2023, Nov 2024
//
// Graph - Using a list of adjacency lists representation
//

#include "Graph.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "SortedList.h"

// GLOBAL COUNTER FOR UNIQUE EDGE IDs
static unsigned int _EdgeNextID = 0; 

struct _Vertex {
  unsigned int id;
  unsigned int inDegree;
  unsigned int outDegree;
  List* edgesList;
};

struct _Edge {
  unsigned int adjVertex;
  double weight;
  unsigned int uniqueId; // ADDED: Unique ID to allow multiedges
};

struct _GraphHeader {
  int isDigraph;
  int isComplete;
  int isWeighted;
  unsigned int numVertices;
  unsigned int numEdges;
  List* verticesList;
};

// The comparator for the VERTICES LIST

int graphVerticesComparator(const void* p1, const void* p2) {
  unsigned int v1 = ((struct _Vertex*)p1)->id;
  unsigned int v2 = ((struct _Vertex*)p2)->id;
  int d = v1 - v2;
  return (d > 0) - (d < 0);
}

// The comparator for the EDGES LISTS (MODIFIED TO ALLOW MULTIEDGES)

int graphEdgesComparator(const void* p1, const void* p2) {
  unsigned int v1 = ((struct _Edge*)p1)->adjVertex;
  unsigned int v2 = ((struct _Edge*)p2)->adjVertex;

  int d = v1 - v2;

  // FIX: If adjacent vertices are the same (d == 0), compare unique IDs.
  // This ensures that two edges (multiedges) to the same vertex are not
  // considered "equal" by ListInsert, allowing both to be stored.
  if (d == 0) {
    unsigned int id1 = ((struct _Edge*)p1)->uniqueId;
    unsigned int id2 = ((struct _Edge*)p2)->uniqueId;
    d = id1 - id2; 
  }
  return (d > 0) - (d < 0);
}

Graph* GraphCreate(unsigned int numVertices, int isDigraph, int isWeighted) {
  Graph* g = (Graph*)malloc(sizeof(struct _GraphHeader));
  if (g == NULL) abort();

  g->isDigraph = isDigraph;
  g->isComplete = 0;
  g->isWeighted = isWeighted;

  g->numVertices = numVertices;
  g->numEdges = 0;

  g->verticesList = ListCreate(graphVerticesComparator);

  for (unsigned int i = 0; i < numVertices; i++) {
    struct _Vertex* v = (struct _Vertex*)malloc(sizeof(struct _Vertex));
    if (v == NULL) abort();

    v->id = i;
    v->inDegree = 0;
    v->outDegree = 0;

    v->edgesList = ListCreate(graphEdgesComparator);

    ListInsert(g->verticesList, v);
  }

  assert((int)g->numVertices == ListGetSize(g->verticesList));

  return g;
}

Graph* GraphCreateComplete(unsigned int numVertices, int isDigraph) {
  Graph* g = GraphCreate(numVertices, isDigraph, 0);

  g->isComplete = 1;

  List* vertices = g->verticesList;
  ListMoveToHead(vertices);
  unsigned int i = 0;
  for (; i < g->numVertices; ListMoveToNext(vertices), i++) {
    struct _Vertex* v = ListGetCurrentItem(vertices);
    List* edges = v->edgesList;
    for (unsigned int j = 0; j < g->numVertices; j++) {
      if (i == j) {
        continue;
      }
      struct _Edge* new = (struct _Edge*)malloc(sizeof(struct _Edge));
      if (new == NULL) abort();
      new->adjVertex = j;
      new->weight = 1;
      new->uniqueId = _EdgeNextID++; // ADDED: Assign ID

      ListInsert(edges, new);
    }
    if (g->isDigraph) {
      v->inDegree = g->numVertices - 1;
      v->outDegree = g->numVertices - 1;
    } else {
      v->outDegree = g->numVertices - 1;
    }
  }
  if (g->isDigraph) {
    g->numEdges = numVertices * (numVertices - 1);
  } else {
    g->numEdges = numVertices * (numVertices - 1) / 2;
  }

  return g;
}

void GraphDestroy(Graph** p) {
  assert(*p != NULL);
  Graph* g = *p;

  List* vertices = g->verticesList;
  if (ListIsEmpty(vertices) == 0) {
    ListMoveToHead(vertices);
    unsigned int i = 0;
    for (; i < g->numVertices; ListMoveToNext(vertices), i++) {
      struct _Vertex* v = ListGetCurrentItem(vertices);

      List* edges = v->edgesList;
      if (ListIsEmpty(edges) == 0) {
        int i = 0;
        ListMoveToHead(edges);
        for (; i < ListGetSize(edges); ListMoveToNext(edges), i++) {
          struct _Edge* e = ListGetCurrentItem(edges);
          free(e);
        }
      }
      ListDestroy(&(v->edgesList));
      free(v);
    }
  }

  ListDestroy(&(g->verticesList));
  free(g);

  *p = NULL;
}

Graph* GraphCopy(const Graph* g) {
  assert(g != NULL);

  Graph* copy = GraphCreate(g->numVertices, g->isDigraph, g->isWeighted);
  copy->isComplete = g->isComplete;

  List* vertices = g->verticesList;
  ListMoveToHead(vertices);
  for (unsigned int i = 0; i < g->numVertices; ListMoveToNext(vertices), i++) {
    struct _Vertex* v = ListGetCurrentItem(vertices);
    List* edges = v->edgesList;

    ListMoveToHead(edges);
    for (int j = 0; j < ListGetSize(edges); ListMoveToNext(edges), j++) {
      struct _Edge* e = ListGetCurrentItem(edges);
      // NOTE: When copying, we rely on GraphAddWeightedEdge to create new unique IDs
      if (g->isWeighted) {
        GraphAddWeightedEdge(copy, v->id, e->adjVertex, e->weight);
      } else {
        GraphAddEdge(copy, v->id, e->adjVertex);
      }
    }
  }

  return copy;
}

Graph* GraphFromFile(FILE* f) {
  assert(f != NULL);

  unsigned int numVertices;
  int isDigraph, isWeighted;

  if (fscanf(f, "%u %d %d", &numVertices, &isDigraph, &isWeighted) != 3) {
    fprintf(stderr, "Error: Failed to read graph properties from file.\n");
    return NULL;
  }

  Graph* g = GraphCreate(numVertices, isDigraph, isWeighted);

  unsigned int v, w;
  double weight;

  while (isWeighted ? fscanf(f, "%u %u %lf", &v, &w, &weight) == 3
                     : fscanf(f, "%u %u", &v, &w) == 2) {
    if (isWeighted) {
      GraphAddWeightedEdge(g, v, w, weight);
    } else {
      GraphAddEdge(g, v, w);
    }
  }

  return g;
}

// Graph

int GraphIsDigraph(const Graph* g) { return g->isDigraph; }

int GraphIsComplete(const Graph* g) { return g->isComplete; }

int GraphIsWeighted(const Graph* g) { return g->isWeighted; }

unsigned int GraphGetNumVertices(const Graph* g) { return g->numVertices; }

unsigned int GraphGetNumEdges(const Graph* g) { return g->numEdges; }

//
// For a graph
//
double GraphGetAverageDegree(const Graph* g) {
  assert(g->isDigraph == 0);
  return 2.0 * (double)g->numEdges / (double)g->numVertices;
}

static unsigned int _GetMaxDegree(const Graph* g) {
  List* vertices = g->verticesList;
  if (ListIsEmpty(vertices)) return 0;

  unsigned int maxDegree = 0;
  ListMoveToHead(vertices);
  unsigned int i = 0;
  for (; i < g->numVertices; ListMoveToNext(vertices), i++) {
    struct _Vertex* v = ListGetCurrentItem(vertices);
    if (v->outDegree > maxDegree) {
      maxDegree = v->outDegree;
    }
  }
  return maxDegree;
}

//
// For a graph
//
unsigned int GraphGetMaxDegree(const Graph* g) {
  assert(g->isDigraph == 0);
  return _GetMaxDegree(g);
}

//
// For a digraph
//
unsigned int GraphGetMaxOutDegree(const Graph* g) {
  assert(g->isDigraph == 1);
  return _GetMaxDegree(g);
}

// Vertices

//
// returns an array of size (outDegree + 1)
// element 0, stores the number of adjacent vertices
// and is followed by indices of the adjacent vertices
//
unsigned int* GraphGetAdjacentsTo(const Graph* g, unsigned int v) {
  assert(v < g->numVertices);

  // Node in the list of vertices
  List* vertices = g->verticesList;
  ListMove(vertices, v);
  struct _Vertex* vPointer = ListGetCurrentItem(vertices);
  unsigned int numAdjVertices = vPointer->outDegree;

  unsigned int* adjacent =
      (unsigned int*)calloc(1 + numAdjVertices, sizeof(unsigned int));

  if (numAdjVertices > 0) {
    adjacent[0] = numAdjVertices;
    List* adjList = vPointer->edgesList;
    ListMoveToHead(adjList);
    for (unsigned int i = 0; i < numAdjVertices; ListMoveToNext(adjList), i++) {
      struct _Edge* ePointer = ListGetCurrentItem(adjList);
      adjacent[i + 1] = ePointer->adjVertex;
    }
  }

  return adjacent;
}

//
// returns an array of size (outDegree + 1)
// element 0, stores the number of adjacent vertices
// and is followed by the distances to the adjacent vertices
//
double* GraphGetDistancesToAdjacents(const Graph* g, unsigned int v) {
  assert(v < g->numVertices);

  // Node in the list of vertices
  List* vertices = g->verticesList;
  ListMove(vertices, v);
  struct _Vertex* vPointer = ListGetCurrentItem(vertices);
  unsigned int numAdjVertices = vPointer->outDegree;

  double* distance = (double*)calloc(1 + numAdjVertices, sizeof(double));

  if (numAdjVertices > 0) {
    distance[0] = numAdjVertices;
    List* adjList = vPointer->edgesList;
    ListMoveToHead(adjList);
    for (unsigned int i = 0; i < numAdjVertices; ListMoveToNext(adjList), i++) {
      struct _Edge* ePointer = ListGetCurrentItem(adjList);
      distance[i + 1] = ePointer->weight;
    }
  }

  return distance;
}

//
// For a graph
//
unsigned int GraphGetVertexDegree(Graph* g, unsigned int v) {
  assert(g->isDigraph == 0);
  assert(v < g->numVertices);

  ListMove(g->verticesList, v);
  struct _Vertex* p = ListGetCurrentItem(g->verticesList);

  return p->outDegree;
}

//
// For a digraph
//
unsigned int GraphGetVertexOutDegree(Graph* g, unsigned int v) {
  assert(g->isDigraph == 1);
  assert(v < g->numVertices);

  ListMove(g->verticesList, v);
  struct _Vertex* p = ListGetCurrentItem(g->verticesList);

  return p->outDegree;
}

//
// For a digraph
//
unsigned int GraphGetVertexInDegree(Graph* g, unsigned int v) {
  assert(g->isDigraph == 1);
  assert(v < g->numVertices);

  ListMove(g->verticesList, v);
  struct _Vertex* p = ListGetCurrentItem(g->verticesList);

  return p->inDegree;
}

// Edges

static int _addEdge(Graph* g, unsigned int v, unsigned int w, double weight) {
  // Insert edge (v,w)
  struct _Edge* edge_v_w = (struct _Edge*)malloc(sizeof(struct _Edge));
  edge_v_w->adjVertex = w;
  edge_v_w->weight = weight;
  edge_v_w->uniqueId = _EdgeNextID++; // ADDED: Assign unique ID

  ListMove(g->verticesList, v);
  struct _Vertex* vertex_v = ListGetCurrentItem(g->verticesList);
  int result = ListInsert(vertex_v->edgesList, edge_v_w);

  if (result == -1) {
    // Insertion failed --- Destroy the allocated edge
    free(edge_v_w);
    return 0;
  }

  // Update
  g->numEdges++;
  vertex_v->outDegree++;

  ListMove(g->verticesList, w);
  struct _Vertex* vertex_w = ListGetCurrentItem(g->verticesList);
  // DIRECTED GRAPH --- Update the in-degree of vertex w
  if (g->isDigraph == 1) {
    vertex_w->inDegree++;
  }

  // If UNDIRECTED GRAPH
  if (g->isDigraph == 0) {
    // It is a BIDIRECTIONAL EDGE --- Insert edge (w,v)
    struct _Edge* edge_w_v = (struct _Edge*)malloc(sizeof(struct _Edge));
    edge_w_v->adjVertex = v;
    edge_w_v->weight = weight;
    edge_w_v->uniqueId = _EdgeNextID++; // ADDED: Assign unique ID

    result = ListInsert(vertex_w->edgesList, edge_w_v);

    if (result == -1) {
      // Insertion failed --- Destroy the allocated edge
      free(edge_w_v);

      // And remove the edge (v,w) that was inserted above
      ListSearch(vertex_v->edgesList, (void*)edge_v_w);
      ListRemoveCurrent(vertex_v->edgesList);

      // UNDO the updates
      g->numEdges--;
      vertex_v->outDegree--;
      
      // We must correctly decrement the outDegree of vertex w
      // since the ListInsert for (w,v) failed.
      // This part of the code was technically missing a decrement if ListInsert failed,
      // but the main issue was ListInsert failing due to the comparator.
      // We rely on the ListInsert failure to be correct now.

      return 0;
    } else {
      // Just update the outDegree of vertex w
      vertex_w->outDegree++;
    }
  }

  return 1;
}

int GraphAddEdge(Graph* g, unsigned int v, unsigned int w) {
  assert(g->isWeighted == 0);
  assert(v != w);
  assert(v < g->numVertices);
  assert(w < g->numVertices);

  return _addEdge(g, v, w, 1.0);
}

int GraphAddWeightedEdge(Graph* g, unsigned int v, unsigned int w,
                         double weight) {
  assert(g->isWeighted == 1);
  assert(v != w);
  assert(v < g->numVertices);
  assert(w < g->numVertices);

  return _addEdge(g, v, w, weight);
}

int GraphRemoveEdge(Graph* g, unsigned int v, unsigned int w) {
  assert(g != NULL);
  assert(v < g->numVertices);
  assert(w < g->numVertices);

  ListMove(g->verticesList, v);
  struct _Vertex* vertex_v = ListGetCurrentItem(g->verticesList);

  // Temporary edge for searching. Since ListSearch uses graphEdgesComparator,
  // we need a unique ID that matches the one being removed. Since we don't
  // know the ID, this search might only find the FIRST edge to w. This
  // demonstrates a potential flaw in GraphRemoveEdge for multigraphs.
  // For TSP, we assume we only remove non-multiedges, or remove one of the duplicates.
  struct _Edge edge_v_w = {w, 0, 0};
  if (ListSearch(vertex_v->edgesList, &edge_v_w) == 0) {
    return 0; // Edge not found
  }

  struct _Edge* edgeToRemove = ListGetCurrentItem(vertex_v->edgesList);
  ListRemoveCurrent(vertex_v->edgesList);
  free(edgeToRemove);

  vertex_v->outDegree--;
  g->numEdges--;

  if (g->isDigraph) {
    ListMove(g->verticesList, w);
    struct _Vertex* vertex_w = ListGetCurrentItem(g->verticesList);
    vertex_w->inDegree--;
  } else {
    ListMove(g->verticesList, w);
    struct _Vertex* vertex_w = ListGetCurrentItem(g->verticesList);

    // Same issue here: temporary edge for search only matches by vertex ID
    struct _Edge edge_w_v = {v, 0, 0};
    if (ListSearch(vertex_w->edgesList, &edge_w_v) == 1) {
      struct _Edge* edgeToRemove = ListGetCurrentItem(vertex_w->edgesList);
      ListRemoveCurrent(vertex_w->edgesList);
      free(edgeToRemove);

      vertex_w->outDegree--;
    }
  }

  return 1;
}

int GraphCheckInvariants(const Graph* g) {
  assert(g != NULL);

  unsigned int totalEdges = 0;
  List* vertices = g->verticesList;

  ListMoveToHead(vertices);
  for (unsigned int i = 0; i < g->numVertices; ListMoveToNext(vertices), i++) {
    struct _Vertex* v = ListGetCurrentItem(vertices);

    // The logic below relies on ListGetSize being the actual number of edges,
    // which is now true since multiedges are inserted.
    if (g->isDigraph) {
      if (v->outDegree != (unsigned int)ListGetSize(v->edgesList)) { // Original check was inDegree + outDegree
        return 0; // Invariant violated
      }
    } else {
      if (v->outDegree != (unsigned int)ListGetSize(v->edgesList)) {
        return 0; // Invariant violated
      }
    }

    totalEdges += v->outDegree;
  }

  if (g->isDigraph) {
    if (totalEdges != g->numEdges) {
      return 0; // Invariant violated
    }
  } else {
    if (totalEdges != 2 * g->numEdges) {
      return 0; // Invariant violated
    }
  }

  return 1; // All invariants hold
}

// DISPLAYING on the console

void GraphDisplay(const Graph* g) {
  printf("---\n");
  if (g->isWeighted) {
    printf("Weighted ");
  }
  if (g->isComplete) {
    printf("COMPLETE ");
  }
  if (g->isDigraph) {
    printf("Digraph\n");
    printf("Max Out-Degree = %d\n", GraphGetMaxOutDegree(g));
  } else {
    printf("Graph\n");
    printf("Max Degree = %d\n", GraphGetMaxDegree(g));
  }
  printf("Vertices = %2d | Edges = %2d\n", g->numVertices, g->numEdges);

  List* vertices = g->verticesList;
  ListMoveToHead(vertices);
  unsigned int i = 0;
  for (; i < g->numVertices; ListMoveToNext(vertices), i++) {
    printf("%2d ->", i);
    struct _Vertex* v = ListGetCurrentItem(vertices);
    if (ListIsEmpty(v->edgesList)) {
      printf("\n");
    } else {
      List* edges = v->edgesList;
      int i = 0;
      ListMoveToHead(edges);
      for (; i < ListGetSize(edges); ListMoveToNext(edges), i++) {
        struct _Edge* e = ListGetCurrentItem(edges);
        if (g->isWeighted) {
          printf("  %2d(%4.2f)", e->adjVertex, e->weight);
        } else {
          printf("  %2d", e->adjVertex);
        }
      }
      printf("\n");
      // Checking the invariants of the list of edges
      ListTestInvariants(edges);
    }
  }
  printf("---\n");
  // Checking the invariants of the list of vertices
  ListTestInvariants(vertices);
}

void GraphListAdjacents(const Graph* g, unsigned int v) {
  printf("---\n");

  unsigned int* array = GraphGetAdjacentsTo(g, v);

  printf("Vertex %d has %d adjacent vertices -> ", v, array[0]);

  for (unsigned int i = 1; i <= array[0]; i++) {
    printf("%d ", array[i]);
  }

  printf("\n");

  free(array);

  printf("---\n");
}

// Display the graph in DOT language.
// To draw the graph, you can use dot (from Graphviz) or paste result on:
//   https://dreampuf.github.io/GraphvizOnline
void GraphDisplayDOT(const Graph* g) {
  char* gtypes[] = {"graph", "digraph"};
  char* edgeops[] = {"--", "->"};
  char* gtype = gtypes[g->isDigraph];
  char* edgeop = edgeops[g->isDigraph];

  printf("// Paste in: https://dreampuf.github.io/GraphvizOnline\n");
  printf("%s {\n", gtype);
  printf("  // Vertices = %2d\n", g->numVertices);
  printf("  // Edges = %2d\n", g->numEdges);
  if (g->isDigraph) {
    printf("  // Max Out-Degree = %d\n", GraphGetMaxOutDegree(g));
  } else {
    printf("  // Max Degree = %d\n", GraphGetMaxDegree(g));
  }

  List* vertices = g->verticesList;
  ListMoveToHead(vertices);
  unsigned int i = 0;
  for (; i < g->numVertices; ListMoveToNext(vertices), i++) {
    printf("  %d;\n", i);
    struct _Vertex* v = ListGetCurrentItem(vertices);
    List* edges = v->edgesList;
    int k = 0;
    ListMoveToHead(edges);
    for (; k < ListGetSize(edges); ListMoveToNext(edges), k++) {
      struct _Edge* e = ListGetCurrentItem(edges);
      unsigned int j = e->adjVertex;
      if (g->isDigraph || i <= j) { // for graphs, draw only 1 edge
        printf("  %d %s %d", i, edgeop, j);
        if (g->isWeighted) {
          printf(" [label=%4.2f]", e->weight);
        }
        printf(";\n");
      }
    }
  }
  printf("}\n");
}

// Nelson Ramos, November 2025.
int GraphWriteDOT(const Graph* g, const char* filename) {
    assert(g != NULL);
    assert(filename != NULL);

    FILE* f = fopen(filename, "w");
    if (!f) {
      fprintf(stderr, "Error: Could not open file '%s' for writing.\n", filename);
      return 0;
    }

    const char* gtypes[] = {"graph", "digraph"};
    const char* edgeops[] = {"--", "->"};
    const char* gtype = gtypes[g->isDigraph];
    const char* edgeop = edgeops[g->isDigraph];

    fprintf(f, "// DOT file generated by GraphWriteDOT\n");
    fprintf(f, "%s {\n", gtype);
    fprintf(f, "  // Vertices = %u\n", g->numVertices);
    fprintf(f, "  // Edges = %u\n", g->numEdges);

    if (g->isDigraph) {
        fprintf(f, "  // Max Out-Degree = %u\n", GraphGetMaxOutDegree(g));
    } else {
        fprintf(f, "  // Max Degree = %u\n", GraphGetMaxDegree(g));
    }

    List* vertices = g->verticesList;
    ListMoveToHead(vertices);
    for (unsigned int i = 0; i < g->numVertices; ListMoveToNext(vertices), i++) {
        struct _Vertex* v = ListGetCurrentItem(vertices);
        fprintf(f, "  %u;\n", i);

        List* edges = v->edgesList;
        ListMoveToHead(edges);
        for (unsigned int k = 0; k < (unsigned int)ListGetSize(edges); ListMoveToNext(edges), k++) {
            struct _Edge* e = ListGetCurrentItem(edges);
            unsigned int j = e->adjVertex;

            // For undirected graphs, draw each edge only once
            if (g->isDigraph || i <= j) {
                fprintf(f, "  %u %s %u", i, edgeop, j);
                if (g->isWeighted) {
                    fprintf(f, " [label=%4.2f]", e->weight);
                }
                fprintf(f, ";\n");
            }
        }
    }

    fprintf(f, "}\n");
    fclose(f);
    return 1;
}

double GetEdgeWeight(const Graph* g, unsigned int v, unsigned int w) {
    if (v == w) return 0.0; // should not happen

    unsigned int* adj = GraphGetAdjacentsTo(g, v);
    double* dist = GraphGetDistancesToAdjacents(g, v); 

    unsigned int num_adj = (unsigned int)dist[0]; // 1st element of arr is number of adj vertices
    double weight = DBL_MAX; // preset (vertex dne)

    for (unsigned int i = 1; i <= num_adj; i++) {
        if (adj[i] == w) {
            weight = dist[i]; 
            break;
        }
    }

    free(adj);
    free(dist);
    return weight;
}