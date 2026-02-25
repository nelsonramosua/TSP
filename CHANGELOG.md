# Changelog

All notable changes to the TSP Solver will be documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).  
Versioning follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

*No unreleased changes yet.*

---

## [1.0.0] — 2026-02

### Initial Release

**11 algorithms** implemented, benchmarked, and compared on real-world and TSPLIB graphs.

#### Exact Algorithms
- **Exhaustive Search** — brute-force O(N!); disabled for N > 10.
- **Exhaustive Search with Pruning** — branch-and-bound pruning over exhaustive search.
- **Held-Karp** — dynamic programming exact algorithm, O(N² × 2^N); practical for N ≤ 20.

#### Heuristic Algorithms
- **Nearest Neighbour** — greedy construction from a given start vertex, O(N²).
- **Greedy** — builds tour by repeatedly selecting the cheapest available edge, O(N³).
- **Nearest Insertion** — constructive; inserts the nearest unvisited city into the partial tour, O(N³).
- **Christofides** — 1.5-approximation for metric TSP using MST + MWPM (Blossom), O(N³).

#### Meta-heuristic Algorithms
- **2-Opt Improvement** — iterative local search over edge swaps, used as a post-processor.
- **Simulated Annealing** — probabilistic 2-opt with temperature cooling.
- **Ant Colony Optimization** — pheromone-guided probabilistic construction.
- **Genetic Algorithm** — population-based search with selection, crossover, and mutation.

#### Lower Bounds
- **MST Lower Bound** — minimum spanning tree cost as a lower bound estimate, O(N²).
- **Held-Karp Lagrangian Relaxation Lower Bound** — tighter lower bound via subgradient optimisation.

#### Predefined Graphs
- `CreateGraphAula` — AED lecture example graph.
- `CreatePortugal12CitiesGraph` — 12 Portuguese cities (real distances).
- `CreateEurope12CitiesGraph` — 12 European cities (real distances; known optimal: 9057.46 km).
- `CreateMatrixGraph15` — 15-vertex matrix graph.
- `CreateMatrixGraph20` — 20-vertex matrix graph.
- `CreateEuclideanGraph15` — 15-vertex fixed Euclidean graph.
- `CreateEil51Graph` — TSPLIB eil51 instance (N=51).
- `CreateOliver30Graph` — TSPLIB Oliver30 instance (N=30).
- `CreateSwiss42Graph` — TSPLIB Swiss42 instance (N=42).
- `CreateBays29Graph` — TSPLIB Bays29 instance (N=29).
- `CreateA280Graph` — TSPLIB A280 instance (N=280).

#### Infrastructure
- `Tour` ADT with `TourCreate`, `TourDeepCopy`, `TourDestroy`, `TourDisplay`, `TourInvariant`, `TourMapCityNames`.
- `NamedGraph` abstraction layer mapping vertex indices to city names.
- `HashMap` auxiliary ADT for vertex-to-name lookup.
- `GraphFactory` helper pipeline for constructing and exporting graphs to DOT format.
- `TSPTest.c` comparison driver supporting selective graph count via command-line argument.
- DOT export (`graphs/` directory); compatible with Graphviz and VSCode Graphviz extension.
- Makefile targets: `make`, `make run [N=n]`, `make runvc [N=n]`, `make clean`.
- CI/CD pipeline (GitHub Actions): build on Linux and macOS, Valgrind leak check, static analysis.

#### Known Issues / Limitations
- Christofides has a known bug causing it to return suboptimal tours on some graphs.
- Exhaustive Search is disabled for N > 10 (too slow).
- HeldKarp exact is practical only for N ≤ 20.
- Genetic Algorithm becomes very slow for N ≥ 55 with default parameters.
- No command-line flag to select individual algorithms (all algorithms run for each graph).

---

[Unreleased]: https://github.com/nelsonramosua/TSP/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/nelsonramosua/TSP/releases/tag/v1.0.0