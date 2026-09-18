# Changelog

All notable changes to the TSP Solver will be documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).  
Versioning follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added
- **PriorityQueue ADT** (`headers/PriorityQueue.h`, `implementations/graph/PriorityQueue.c`): an indexed binary min-heap over integer items with O(log n) insert / extract-min / decrease-key and O(1) contains (ties broken by item id for determinism). 
This is the auxiliary ADT the README/Greedy/Prim notes kept pointing at.
- **Farthest Insertion** (`implementations/heuristics/FarthestInsertion.c`): constructive heuristic that inserts the vertex farthest from the current tour at its cheapest position; typically beats Nearest Insertion. 
(No separate "Cheapest Insertion" — Greedy already is cheapest insertion.)
- **Clarke-Wright Savings** (`implementations/heuristics/ClarkeWright.c`): a new *class* of construction heuristic -- merges chains by decreasing savings around a depot, guarded by union-find; O(N² * log N).
- **Or-opt** (`implementations/metaheuristics/OrOpt.c`): local search that relocates chains of 1–3 consecutive cities (optionally reversed), complementing 2-Opt.
- **3-opt** (`implementations/metaheuristics/ThreeOpt.c`): local search removing three edges and trying all 7 reconnections; a strictly larger neighbourhood than 2-Opt. 
Reaches the known optimum on several TSPLIB/known instances.
- All four verified: tours pass `TourInvariant`, tracked cost equals a fresh recomputation (so the Or-opt/3-opt incremental deltas are exact), improvements never worsen their seed, and valgrind reports no leaks. 
Wired into the comparison driver (Or-Opt/3-Opt refine the Nearest Neighbour tour).

### Fixed
- **Christofides / Blossom MWPM**: the weighted Blossom matching returned a *valid but non-minimum* perfect matching whenever odd blossoms formed (approx. 13% of random instances), which degraded Christofides tours. 
Rewrote it as a correct primal-dual weighted blossom (with blossom duals and expansion), verified against a brute-force minimum-weight perfect-matching oracle: 0 mismatches over 2000 integer + 500 double-weight trials.
Christofides now builds on a genuinely minimum matching and respects its 1.5× guarantee.

### Changed
- **Genetic Algorithm -- memory overhead**: removed the per-generation allocation churn (a temp `Tour` per fitness eval, deep-copied parents per selection, and a full population rebuilt every generation). 
It now computes cost in place, selects parents by pointer, ping-pongs between two pre-allocated populations, and keeps each population's paths in one contiguous pool. 
Allocation is now **O(population)** for the whole run instead of **O(population * generations)**; the search itself is unchanged.
- **Genetic Algorithm -- reproducibility**: the population sort now uses a deterministic tie-break (lexicographic on the path), so a given RNG seed yields identical results regardless of memory layout.
- **Greedy**: now uses the PriorityQueue to drive cheapest-insertion, dropping it from **O(N³)** to **O(N² * log N)** (each unvisited vertex keeps its cheapest insertion cost in the heap; only the two new edges are updated per insertion, via decrease-key). 
Output is byte-for-byte identical to the previous O(N³) version.
- **Complexity notes corrected** (`README.md`, `Greedy.c`, `Prim_MST.c`): a min-heap helps Greedy, but is a *pessimization* for Prim/MST on the complete graphs used here -- heap-based Prim would be O(N² * log N), worse than the dense O(N²) array form.
Nearest Neighbour and Nearest Insertion are noted as similar (array beats heap). 
The old README claim that a PQ would speed up MST is thus retracted.

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
- ~~Christofides has a known bug causing it to return suboptimal tours on some graphs.~~ **Fixed in [Unreleased].**
- Exhaustive Search is disabled for N > 10 (too slow).
- HeldKarp exact is practical only for N ≤ 20.
- Genetic Algorithm becomes very slow for N ≥ 55 with default parameters.
- No command-line flag to select individual algorithms (all algorithms run for each graph).

---

[Unreleased]: https://github.com/nelsonramosua/TSP/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/nelsonramosua/TSP/releases/tag/v1.0.0