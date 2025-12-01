# Traveling Salesman Problem (TSP) Solver

This repository implements multiple algorithms to solve the **Traveling Salesman Problem (TSP)**, including exact methods, heuristics, and meta-heuristics. Its purpose is to experiment and compare solution quality and computational complexity.

---

## Project Structure

```
TSP/
├── headers/                # Header files
│   ├── Graph.h
│   ├── GraphFactory.h
│   └── SortedList.h
├── implementations/
│   ├── exact/              # Exact algorithms
│   │   └── ExhaustiveSearch.c
│   ├── graph/              # Graph utilities
│   │   ├── Graph.c
│   │   ├── GraphHelpers.c
│   │   └── SortedList.c
│   ├── heuristics/         # Heuristic algorithms
│   │   ├── Greedy.c
│   │   ├── NearestNeighbour.c
│   │   ├── Christofides.c
│   │   └── blossom/        # Blossom implementation (C++)
│   ├── metaheuristics/     # Meta-heuristics
│   │   ├── TwoOpt.c
│   │   ├── SimulatedAnnealing.c
│   │   └── AntColony.c
│   └── mst/                # Minimum Spanning Tree utilities
│       ├── Prim_MST.c
│       └── LowerBound_MST.c
├── graphs/                 # Predefined and generated graphs (.dot)
├── TSPTest.c               # Comparison driver
├── Tour.c                  # Tour structure and utilities
├── GraphFactory.c          # Predefined graph generation
├── Makefile
└── TravelingSalesmanProblem.h # Main project header
```

---

## Implemented Algorithms

| Algorithm                 | Type            | Complexity (worst case)                | Notes                                                                 |
| :------------------------ | :-------------: | :------------------------------------: | :------------------------------------------------------------------- |
| **Exhaustive Search**     | Exact           | $O(N!)$                                | Finds the optimal solution; infeasible for $N > 12–15$.              |
| **Nearest Neighbor**      | Heuristic       | $O(N^2)$                               | Fast, but solution quality may vary; starting point affects the tour. |
| **Greedy Heuristic**      | Heuristic       | $O(N^2 \log N)$                        | Builds a tour by repeatedly selecting the shortest available edge.   |
| **Christofides Algorithm**| Heuristic       | $O(N^3)$                               | Guarantees a tour $\le 1.5\times$ optimal for metric TSP.            |
| **2-Opt Improvement**     | Meta-heuristic  | $O(N^2 \times \text{iterations})$      | Local search to improve an existing tour; often used after Nearest Neighbor or Greedy. |
| **Simulated Annealing**   | Meta-heuristic  | $O(N^2 \times \text{iterations})$      | Probabilistic improvement using 2-opt swaps; good balance between exploration and runtime. |
| **Ant Colony Optimization** | Meta-heuristic | $O(N^2 \times \text{iterations} \times \text{ants})$ | Probabilistic search guided by pheromone trails; computationally heavier but can yield high-quality solutions. |
| **Lower Bound (MST)**     | Utility         | $O(E \log N)$                          | Provides a minimum cost estimate using a **Minimum Spanning Tree**. |

---

## Compilation Instructions

This project uses **GCC** and **G++** to compile C and C++ code (for the Blossom algorithm).

1. **Clone the repository:**

    ```bash
    git clone https://github.com/nelsonramosua/TSP
    cd TSP
    ```

2. **Build the project:**

    ```bash
    make
    ```

3. **Run the comparison driver:**

    ```bash
    ./TSP_COMPARISON
    ```

4. **Clean the build:**

    ```bash
    make clean
    ```

---

## Graphs

The project includes:

* **Predefined Matrix Graphs:** `GraphFactory_CreateMatrixGraph15` and `GraphFactory_CreateMatrixGraph20`.
* **Fixed Euclidean Graph:** `GraphFactory_CreateEuclideanGraph15`.
* **Random Euclidean Graphs:** `GraphFactory_CreateRandomEuclideanGraph(N, maxX, maxY)`.

Graph outputs in DOT format are saved in `graphs/` and can be visualized directly in VSCODE with **Graphviz** extension or via terminal:

```bash
dot -Tpng graphs/testGraph.dot -o graphs/testGraph.png
```

---

## Notes and Recommendations

* For small graphs ($N \le 12$), Exhaustive Search guarantees optimal results. However, it is disabled. Uncomment to enable.
* For medium-sized graphs ($N \le 100$), heuristics like Christofides, Greedy, and Nearest Neighbor are recommended.
* For large graphs ($N > 100$), meta-heuristics like Simulated Annealing and Ant Colony Optimization provide good approximations.
* Use 2-Opt Improvement as a post-processing step to refine heuristic solutions.
* I could not for the life of me get Christofides working well... I tried everything... As a last ditch effort, I included a C++ implementation of the Blossom algorithm for computing the Minimum Weight Perfect Matching (MWPM)... But that does not work well.

---

## Resources used
* **Main resource used:** https://youtu.be/GiDsjIBOVoA?si=zEtD3WFBlUYQO_LN 
* https://cse442-17f.github.io/Traveling-Salesman-Algorithms/
* https://algorithms.discrete.ma.tum.de/graph-algorithms/hierholzer/index_en.html
* https://alon.kr/posts/christofides
* https://en.wikipedia.org/wiki/Blossom_algorithm
* https://github.com/suddhabrato/edmonds-blossom-algorithm
* https://www.geeksforgeeks.org/dsa/travelling-salesman-problem-greedy-approach/
* https://youtu.be/oXb2nC-e_EA?si=3G2oxIMb31RO8N8n

--- 

## Author

Nelson Ramos (124921).