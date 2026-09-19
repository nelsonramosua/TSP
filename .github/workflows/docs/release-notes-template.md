# TSP Solver Release Notes

See [CHANGELOG.md](https://github.com/nelsonramosua/TSP/blob/main/CHANGELOG.md) for the full list of changes in this release.

## Included Algorithms

| Category | Algorithms |
|---|---|
| Exact | Exhaustive Search, Exhaustive Search w/ Pruning, Branch & Bound, Held-Karp |
| Heuristic | Nearest Neighbour, Greedy, Nearest Insertion, Farthest Insertion, Clarke-Wright Savings, Christofides |
| Local search | 2-Opt, Or-Opt, 3-Opt, Lin-Kernighan |
| Meta-heuristic | Simulated Annealing, Ant Colony, Genetic Algorithm, Tabu Search, GRASP, ISPO |
| Lower Bounds | MST, Held-Karp Lagrangian Relaxation |

## Usage

```bash
# Build
make

# Run all graphs
./TSP_COMPARISON

# Run first N graphs
./TSP_COMPARISON N

# Run with Valgrind
make runvc N=1
```