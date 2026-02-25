# Contributing to the TSP Solver

Thank you for your interest in contributing. This project explores exact, heuristic, and meta-heuristic approaches to the Travelling Salesman Problem, implemented from scratch in pure C. Contributions of new algorithms, new graphs, bug fixes, and documentation improvements are all welcome.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Project Structure](#project-structure)
- [How to Contribute](#how-to-contribute)
  - [Reporting Bugs](#reporting-bugs)
  - [Proposing a New Algorithm](#proposing-a-new-algorithm)
  - [Adding a New Graph](#adding-a-new-graph)
  - [Other Improvements](#other-improvements)
- [Development Workflow](#development-workflow)
- [Code Style](#code-style)
- [Adding a New Algorithm — Step by Step](#adding-a-new-algorithm--step-by-step)
- [Adding a New Graph — Step by Step](#adding-a-new-graph--step-by-step)
- [Testing Guidelines](#testing-guidelines)
- [Commit Messages](#commit-messages)
- [Pull Request Checklist](#pull-request-checklist)

---

## Code of Conduct

Be respectful and constructive. This is an academic, educational project — everyone is here to learn.

---

## Getting Started

```bash
# Clone the repository
git clone https://github.com/nelsonramosua/TSP
cd TSP

# Build
make

# Run quick smoke test (3 graphs)
./TSP_COMPARISON 3

# Run with Valgrind (1 graph)
make runvc N=1
```

**Prerequisites:** GCC, Make, Valgrind (optional), Graphviz (optional for DOT rendering).

---

## Project Structure

```
TSP/
├── headers/                    # Public interfaces (.h)
├── implementations/
│   ├── exact/                  # Exact algorithms (ExhaustiveSearch, HeldKarp)
│   ├── heuristics/             # Constructive heuristics (Greedy, NearestNeighbour, ...)
│   ├── metaheuristics/         # Meta-heuristics (SA, ACO, GA, 2-Opt)
│   ├── graph/                  # Graph ADT, NamedGraph, HashMap, SortedList
│   ├── lowerBounds/            # Lower bound utilities (MST, HeldKarp)
│   └── mst/                    # Prim MST (shared utility)
├── GraphFactory.c              # Predefined graph constructors
├── Tour.c                      # Tour ADT
├── TSPTest.c                   # Comparison driver
└── TravelingSalesmanProblem.h  # Central project header
```

**Key rule:** all algorithms operate on `const Graph*` and return a `Tour*` (or `double` for lower bounds). They must not modify the graph.

---

## How to Contribute

### Reporting Bugs

Use the [Bug Report](.github/ISSUE_TEMPLATE/bug_report.yml) issue template. Include:
- Which algorithm and which graph trigger the bug.
- Exact commands to reproduce.
- Valgrind output if it's a memory issue (`make runvc N=1`).
- Compiler version and OS.

### Proposing a New Algorithm

Use the [New Algorithm](.github/ISSUE_TEMPLATE/new_algorithm.yml) issue template before opening a PR. This lets us discuss feasibility and avoid duplicate work.

Good candidates (not yet implemented):
- Lin-Kernighan heuristic.
- Or-Opt improvement.
- Branch & Bound (exact).
- ISPO (as mentioned in the README).
- Tabu Search.

### Adding a New Graph

Use the [New Graph](.github/ISSUE_TEMPLATE/new_graph.yml) issue template. Graphs should either be TSPLIB instances (with a known optimal) or real-world graphs (with a meaningful motivation). See [step-by-step instructions](#adding-a-new-graph--step-by-step) below.

### Other Improvements

- Performance improvements (e.g. using a min-heap in Greedy or MST Lower Bound).
- Refactoring for clarity.
- Documentation or README improvements.
- CI/CD improvements.

Open a [Feature Request](.github/ISSUE_TEMPLATE/feature_request.yml) or go straight to a PR for small, obvious fixes.

---

## Development Workflow

```bash
# 1. Fork and clone
git clone https://github.com/<your-username>/TSP
cd TSP

# 2. Create a feature branch
git checkout -b feature/lin-kernighan    # new algorithm
git checkout -b fix/greedy-memory-leak   # bug fix
git checkout -b graph/berlin52           # new graph

# 3. Develop, then verify
make
make CFLAGS="-Wall -Wextra -Werror -O3 -Iheaders"   # must be zero warnings
./TSP_COMPARISON 3
make runvc N=1                                        # Valgrind clean

# 4. Commit, push, open PR
git commit -m "feat: add Lin-Kernighan heuristic"
git push origin feature/lin-kernighan
```

---

## Code Style

Follow the style used throughout the project. Key points:

### File Header

Every `.c` file must start with:

```c
// MyAlgorithm.c - One-line description.
//
// Time complexity: O(...)
//
// Nelson Ramos, 124921.   <-- original author; add your name below it if you modify significantly.
//                                              if you created it from scratch, only your name at first.
//
// Month, Year.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// Resources used:
// https://...
```

### Includes

```c
#include "../../TravelingSalesmanProblem.h"   // always first
#include <stdio.h>
#include <stdlib.h>
// ... other standard headers
```

### Naming Conventions

```c
// Types: PascalCase
typedef struct _Tour { ... } Tour;

// Functions: AlgorithmName_Action or descriptiveLowerCamel for statics
Tour* NearestNeighbour_FindTour(const Graph* g, unsigned int startVertex);
static double computeTourCost(const Graph* g, unsigned int* path, unsigned int n);

// Variables: camelCase
unsigned int numVertices;
double bestCost;

// Macros / constants: UPPER_SNAKE_CASE (defined in headers/Metaheuristics.h)
#define SA_INITIAL_TEMP 10000.0
```

### Braces and Formatting

```c
// Opening brace on same line
for (unsigned int i = 0; i < n; i++) {
    doSomething(i);
}

// Single-line for trivial cleanup / guards
if (!ptr) { fprintf(stderr, "Error: alloc failed\n"); return NULL; }

// You can also single-line for very small code blocks
if (cost < best) best = cost;
```

### Memory

- **All allocations must be freed.** Verify with `make runvc N=1`.
- Prefer `malloc` + explicit `free` over any other pattern.
- Check every `malloc` / `calloc` return value:
  ```c
  Tour* t = TourCreate(n);
  if (!t) { fprintf(stderr, "Error: TourCreate failed\n"); return NULL; }
  ```
- Never `free` memory you don't own. Each algorithm creates and returns a `Tour*`; the caller destroys it with `TourDestroy`.

### Comments

- Add a comment block at the top of non-obvious functions explaining what they do.
- Cite any resource (paper, blog, code) used for the implementation, both in the file header and inline if relevant.
- No stray debug `printf` left in merged code.

---

## Adding a New Algorithm — Step by Step

**Example:** adding `LinKernighan`.

### 1. Create the implementation file

```
implementations/heuristics/LinKernighan.c
```

File must follow the header format described above.

Function signature must match the pattern used by the other algorithms:

```c
Tour* LinKernighan_FindTour(const Graph* g);
```

### 2. Add the prototype to `TravelingSalesmanProblem.h`

```c
// Heuristics
Tour* NearestNeighbour_FindTour(const Graph* g, unsigned int startVertex);
Tour* Greedy_FindTour(const Graph* g);
Tour* NearestInsertion_FindTour(const Graph* g);
Tour* Christofides_FindTour(const Graph* g);
Tour* LinKernighan_FindTour(const Graph* g);   // <-- add here
```

### 3. Add to `Makefile`

```makefile
C_SRCS = ... \
         $(HEUR_DIR)/LinKernighan.c \   # <-- add here
         ...
```

### 4. Register in `TSPTest.c`

Add an adapter and register in the `algorithms[]` array:

```c
// Adapter (in TSPTest.h or at the top of TSPTest.c)
static Tour* LinKernighan_Adapter(const Graph* g, void* unused) {
    (void)unused;
    return LinKernighan_FindTour(g);
}

// In algorithms[] array
{ LinKernighan_Adapter, "Lin-Kernighan", 150, NULL },
//                                        ^ max N before skipping
```

### 5. Update `README.md`

Add a row to the algorithm table in the **Implemented Algorithms** section:

```markdown
| **Lin-Kernighan** | Heuristic | O(N^3) | ... |
```

### 6. Update `CHANGELOG.md`

Add an entry under `[Unreleased]`:

```markdown
### Added
- Lin-Kernighan heuristic (`implementations/heuristics/LinKernighan.c`).
```

---

## Adding a New Graph — Step by Step

**Example:** adding `Berlin52` (a TSPLIB instance).

### 1. Add prototype to `GraphFactory.h`

```c
NamedGraph* CreateBerlin52Graph(void);
```

### 2. Implement in `GraphFactory.c`

```c
NamedGraph* CreateBerlin52Graph(void) {
    NamedGraph* ng = NamedGraphCreate(52);
    // Add edges by city name (or by index if unnamed)
    GraphFactory_AddEdgeByCityNames(ng, "City0", "City1", 565.0);
    // ...
    _writeDOT(ng, "Berlin52Graph");
    return ng;
}
```

Read the header comments in `GraphFactory.c` for the full pipeline and the `_writeDOT` / `GraphFactory_AddEdgeByCityNames` helpers.

### 3. Register in `TSPTest.c`

```c
static GraphTestCase testCases[] = {
    // ...
    { CreateBerlin52Graph, "Berlin 52 Cities (TSPLIB)", 7542.0 },
    //                                                   ^ known optimal; use -1 if unknown
};
```

---

## Testing Guidelines

| Check | Command |
|---|---|
| Builds cleanly | `make` |
| Zero-warning build | `make CFLAGS="-Wall -Wextra -Werror -O3 -Iheaders"` |
| Smoke test | `./TSP_COMPARISON 3` |
| Valgrind (1 graph) | `make runvc N=1` |
| Run all graphs | `./TSP_COMPARISON` |

There is no formal unit test binary for this project (TSPTest.c is the integration test driver). When adding a new algorithm, verify manually that:
- It produces a valid tour (all vertices visited exactly once - `TourInvariant` asserts this).
- The cost is plausible relative to other algorithms on the same graph.
- It does not crash on any of the predefined graphs.
- Valgrind shows no leaks.

---

## Commit Messages

Use the [Conventional Commits](https://www.conventionalcommits.org/) style:

```
feat: add Lin-Kernighan heuristic
fix: fix memory leak in GeneticAlgorithm population cleanup
perf: use min-heap in Greedy to reduce complexity to O(N^2 log N)
docs: update README algorithm table
refactor: extract tourCost helper in SimulatedAnnealing
test: add Berlin52 graph to TSPTest
ci: increase Valgrind timeout in nightly workflow
```

---

## Pull Request Checklist

Before submitting a PR, confirm:

- [ ] `make CFLAGS="-Wall -Wextra -Werror -O3 -Iheaders"` passes with no errors.
- [ ] `./TSP_COMPARISON 3` runs and produces reasonable output.
- [ ] `make runvc N=1` — Valgrind reports no leaks.
- [ ] New algorithm added to `TravelingSalesmanProblem.h`, `Makefile`, and `TSPTest.c`.
- [ ] Algorithm table in `README.md` updated.
- [ ] `CHANGELOG.md` updated under `[Unreleased]`.
- [ ] No `TODO` / `FIXME` left in the code you added.
- [ ] No stray debug `printf` left in merged code.
- [ ] Resources used are cited in the file header.