// animate.c - visualization driver: runs ONE algorithm on TSPLIB Eil51 with the Trace sink installed, writing each frame to stdout for tools/render_gif.py to turn into a GIF.
//
// Not part of the main build (TSP_COMPARISON).
// Build via tools/make_gifs.sh.
//
// Output format (plain text):
//   COORDS <N>
//   <x0> <y0>
//   ... (N lines)
//   FRAME <count> <cost>
//   <v0> <v1> ... <v_{count-1}>
//   ... (one FRAME block per emitted frame)
//
// Nelson Ramos, 124921. September, 2026.

#include "../TravelingSalesmanProblem.h"
#include "../headers/GraphFactory.h"
#include "../headers/NamedGraph.h"
#include "../headers/Trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Eil51 coordinates (TSPLIB), vertex i <-> coords[i]; kept here so the viz tool needs no ADT change.
static const double EIL51[51][2] = {
    {37,52},{49,49},{52,64},{20,26},{40,30},{21,47},{17,63},{31,62},{52,33},{51,21},
    {42,41},{31,32},{5,25},{12,42},{36,16},{52,41},{27,23},{17,33},{13,13},{57,58},
    {62,42},{42,57},{16,57},{8,52},{7,38},{27,68},{30,48},{43,67},{58,48},{58,27},
    {37,69},{38,46},{46,10},{61,33},{62,63},{63,69},{32,22},{45,35},{59,15},{5,6},
    {10,17},{21,10},{5,64},{30,15},{39,10},{32,39},{25,32},{25,55},{48,28},{56,37},
    {30,40}
};

static FILE* g_out;

static void sink(const unsigned int* path, unsigned int count, unsigned int n, double cost, void* ctx) {
    (void)ctx; (void)n;
    fprintf(g_out, "FRAME %u %.2f\n", count, cost);
    for (unsigned int i = 0; i < count; i++) fprintf(g_out, "%u%s", path[i], i + 1 < count ? " " : "\n");
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s <algo> [seed]\n", argv[0]); return 2; }
    const char* algo = argv[1];
    srand(argc > 2 ? (unsigned)atoi(argv[2]) : 42u);

    NamedGraph* ng = CreateEil51Graph();
    if (!ng) return 1;
    Graph* g = ng->g;
    unsigned int n = GraphGetNumVertices(g);

    g_out = stdout;
    fprintf(g_out, "COORDS %u\n", n);
    for (unsigned int i = 0; i < n; i++) fprintf(g_out, "%.1f %.1f\n", EIL51[i][0], EIL51[i][1]);

    TraceSet(sink, NULL);

    Tour* t = NULL;
    // improvement algorithms need a seed tour (Nearest Neighbour).
    // The seed's own frames are not traced here because we install the sink only for the algorithm under study.
    if (!strcmp(algo, "nn")) {
        t = NearestNeighbour_FindTour(g, 0);
    } else if (!strcmp(algo, "farthest")) {
        t = FarthestInsertion_FindTour(g);
    } else if (!strcmp(algo, "christofides")) {
        t = Christofides_FindTour(g);
    } else if (!strcmp(algo, "aco")) {
        t = AntColony_FindTour(g);
    } else if (!strcmp(algo, "ga")) {
        t = GeneticAlgorithm_FindTour(g);
    } else if (!strcmp(algo, "grasp")) {
        t = GRASP_FindTour(g);
    } else {
        // seed-based: build NN seed WITHOUT tracing, then trace the improvement
        TraceSet(NULL, NULL);
        Tour* seed = NearestNeighbour_FindTour(g, 0);
        TraceSet(sink, NULL);
        if (!strcmp(algo, "2opt"))      t = TwoOpt_ImproveTour(g, seed);
        else if (!strcmp(algo, "oropt")) t = OrOpt_ImproveTour(g, seed);
        else if (!strcmp(algo, "lk"))    t = LinKernighan_ImproveTour(g, seed);
        else if (!strcmp(algo, "sa")) {
            unsigned int* p = malloc((n + 1) * sizeof(unsigned int));
            memcpy(p, seed->path, (n + 1) * sizeof(unsigned int));
            t = SimulatedAnnealing_FindTour(g, p);
            free(p); TourDestroy(&seed);
        } else { fprintf(stderr, "unknown algo '%s'\n", algo); TourDestroy(&seed); NamedGraphDestroy(&ng); return 2; }
    }

    TraceSet(NULL, NULL);
    if (t) TourDestroy(&t);
    NamedGraphDestroy(&ng);
    return 0;
}