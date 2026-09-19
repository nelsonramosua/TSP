# Changelog

All notable changes to the TSP Solver will be documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).  
Versioning follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added
- **Branch & Bound** (`implementations/exact/BranchAndBound.c`): exact DFS with an admissible lower bound (each vertex still needing an outgoing edge contributes at least its cheapest incident edge), seeded with a Nearest Neighbour incumbent and expanding children nearest-first. 
Prunes far more than the cost-only pruned brute force; verified to match Held-Karp's optimum on every gated instance, valgrind-clean. 
Still exponential worst-case, so the driver gates it to N ≤ 15.
- **PriorityQueue ADT** (`headers/PriorityQueue.h`, `implementations/graph/PriorityQueue.c`): an indexed binary min-heap over integer items with O(log n) insert / extract-min / decrease-key and O(1) contains (ties broken by item id for determinism). 
This is the auxiliary ADT the README/Greedy/Prim notes kept pointing at.
- **NeighbourList ADT** (`headers/NeighbourList.h`, `implementations/graph/NeighbourList.c`): per-vertex k-nearest-neighbour candidate lists, built with the PriorityQueue; used by Lin-Kernighan to restrict moves to promising partners.
- **DistanceMatrix helper** (`headers/DistanceMatrix.h`, `implementations/graph/DistanceMatrix.c`): builds a dense `N*N` edge-weight cache once (`DIST_AT` indexes it) so hot inner loops get `O(1)` weight lookups instead of the `O(degree)` `GetEdgeWeight`. 
Factored out of 2-Opt / Or-Opt / Simulated Annealing, which previously each carried an identical private copy.
- **Lin-Kernighan** (`implementations/metaheuristics/LinKernighan.c`): variable-depth local search (chained edge exchanges via the gain criterion, realised as 2-Opt reversals, keeping the best-improving prefix) using the candidate lists. 
The strongest local search in the project; verified tour-valid with tracked cost == recomputed cost, never worsens its seed, valgrind-clean. 
Reaches the known optimum on several instances (e.g. Bays29, Swiss42) and is competitive with 3-Opt on the rest.
- **Farthest Insertion** (`implementations/heuristics/FarthestInsertion.c`): constructive heuristic that inserts the vertex farthest from the current tour at its cheapest position; typically beats Nearest Insertion. 
(No separate "Cheapest Insertion" — Greedy already is cheapest insertion.)
- **Clarke-Wright Savings** (`implementations/heuristics/ClarkeWright.c`): a new *class* of construction heuristic -- merges chains by decreasing savings around a depot, guarded by union-find; O(N² * log N).
- **Tabu Search** (`implementations/metaheuristics/TabuSearch.c`): best-improvement 2-Opt search with a tabu list (recently-added edges protected) and an aspiration criterion; accepts worsening moves to escape local optima. 
Rounds out the metaheuristic family (SA/ACO/GA). 
Verified tour-valid, best cost == recomputed, valgrind-clean.
Reaches the known optimum on several instances (e.g. Bays29, Matrix20).
- **GRASP** (`implementations/metaheuristics/GRASP.c`): multi-start metaheuristic -- each restart builds a randomized-greedy tour (value-based restricted-candidate-list nearest neighbour, tuned by `GRASP_ALPHA`) and improves it with 2-Opt, keeping the best over `GRASP_ITERATIONS` restarts. 
Reuses one prebuilt NeighbourList + DistanceMatrix across all restarts (its 2-Opt is a quiet, self-contained variant). 
Verified tour-valid, best cost == recomputed, valgrind-clean. 
Reaches the optimum on Swiss42/Bays29/Oliver30 and is A280's best result here (2708 vs 2720 for Lin-Kernighan).
- **ISPO** (`implementations/metaheuristics/ISPO.c`): the discrete-PSO method of Wang, Mu & Zhu (2013). 
The velocity is a "mobile sequence" of "mobile operators" (each moves a city *k* steps along the tour); position update applies the sequence, velocity update combines the inertia-scaled old velocity with the pbest- and gbest-difference sequences (each operator retained probabilistically, per the paper's Def. 6). 
Hybridised with a memetic 2-Opt descent on every particle (the paper's SA-neighbourhood role) and seeded with one Nearest-Neighbour tour, which is what makes it competitive -- pure discrete PSO drifts far from the optimum. 
Verified tour-valid, best cost == recomputed, valgrind-clean; near-optimal on the tested instances (optimal on Bays29/Oliver30/Swiss42, +0.05% on kroA100). 
Gated to $N \le 100$ in the driver.
- **Or-opt** (`implementations/metaheuristics/OrOpt.c`): local search that relocates chains of 1–3 consecutive cities (optionally reversed), complementing 2-Opt.
- **3-opt** (`implementations/metaheuristics/ThreeOpt.c`): local search removing three edges and trying all 7 reconnections; a strictly larger neighbourhood than 2-Opt. 
Reaches the known optimum on several TSPLIB/known instances.
- All four verified: tours pass `TourInvariant`, tracked cost equals a fresh recomputation (so the Or-opt/3-opt incremental deltas are exact), improvements never worsen their seed, and valgrind reports no leaks. 
Wired into the comparison driver (Or-Opt/3-Opt refine the Nearest Neighbour tour).
- **Test graphs** (`GraphFactory.c`): four new instances in the default benchmark -- `CreateKroA100Graph` (TSPLIB kroA100, opt 21 282) fills the size gap between Eil51 and A280; `CreateNonMetricGraph8` deliberately violates the triangle inequality (Christofides lands ~30% over the HK optimum here, showing its metric-only guarantee); and `CreateTriangleGraph3` / `CreateSquareGraph4` cover the boundary cases (single tour; first graph where a 2-Opt move helps). 
All produce valid tours across every algorithm, valgrind-clean.

### Fixed
- **Simulated Annealing -- infinite loop on N < 4**: the "pick two non-adjacent edges" `do/while` never terminated for N < 4 (a triangle has no non-adjacent edge pair, so no 2-Opt move exists). 
SA now skips annealing for N < 4 and returns the trivial tour. 
Surfaced by the new `CreateTriangleGraph3` instance.
- **Christofides / Blossom MWPM**: the weighted Blossom matching returned a *valid but non-minimum* perfect matching whenever odd blossoms formed (approx. 13% of random instances), which degraded Christofides tours. 
Rewrote it as a correct primal-dual weighted blossom (with blossom duals and expansion), verified against a brute-force minimum-weight perfect-matching oracle: 0 mismatches over 2000 integer + 500 double-weight trials.
Christofides now builds on a genuinely minimum matching and respects its 1.5× guarantee.

### Changed
- **`GetEdgeWeight` -- no longer allocates per call**: it was implemented via the copying accessors `GraphGetAdjacentsTo` / `GraphGetDistancesToAdjacents`, so every edge-weight lookup malloc'd and freed two $N$-sized arrays. 
It now walks the vertex's edge list directly (same adjacency-list representation, no ADT interface change), returning the same values with zero allocation. 
This alone cut the full benchmark from ~1m58s to ~37s and sped up every caller that isn't matrix-cached (Nearest/Farthest Insertion on A280 ~8.7 s -> ~2 s, Greedy 2×, etc.). 
The lookup is still $O(\text{degree})$; inner loops that need $O(1)$ use the distance-matrix cache below.
- **2-Opt / Or-Opt -- now scale to large instances**: both local searches were rewritten to search only the $k$-nearest **candidate lists** of each edge's endpoints (via the NeighbourList ADT) instead of all $O(N^2)$ pairs, and to read edge weights from a cached $N \times N$ distance matrix ($O(1)$ vs the $O(\text{degree})$ `GetEdgeWeight`). 
Per pass is now **O(N · k)** rather than **O(N²)**; on TSPLIB A280 ($N=280$) 2-Opt drops to ~0.17 s and Or-Opt from ~16 s to ~0.20 s. 
For $N \le$ candidate-list size the neighbourhood is the whole graph, i.e. identical to the old full search. 
Verified tour-valid, tracked cost == recomputed cost, never worse than the seed, valgrind-clean.
- **Simulated Annealing -- distance-matrix cache**: the Metropolis loop called `GetEdgeWeight` four times per step; it now reads a cached $N \times N$ matrix ($O(1)$ lookups). 
This lets SA run on A280 (previously > 45 s, effectively unusable). Same search, same results.
- **Ant Colony -- per-iteration allocation removed**: the per-iteration ant tours, costs, `visited` and `probs` were stack VLAs re-created every iteration (a ~313 KB VLA per iteration on A280). 
They are now heap buffers allocated **once** (ant tours share one contiguous pool). 
The RNG call order is unchanged, so results are identical.
- **Comparison driver -- A280 enabled**: `CreateA280Graph` ($N=280$) is now part of the default benchmark. 
The methods that do not scale (3-Opt, Tabu Search, Ant Colony) are gated to $N \le 60$, joining the Genetic Algorithm's existing $N \le 55$ gate (the gate was tightened from 100 to 60 once kroA100 showed those methods take 10--16 s each at $N=100$ for worse results than GRASP's sub-second answer).
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