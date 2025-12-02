# Traveling Salesman Problem (TSP) Solver

This repository implements multiple algorithms to solve the **Traveling Salesman Problem (TSP)**, including exact methods, heuristics, and meta-heuristics. It aims to compare the (best) Tours (paths, if you want) found by each.

I do not deserve full credit for this project. I based my implementations on resources available on the internet (research papers, youtube videos, blog posts, etc.); furthermore, I had AI assistance (Claude Opus 4.5) for some of the implementations.

---

## Project Structure

```
TSP/
├── headers/                # Header files
│   ├── Graph.h
│   ├── GraphFactory.h
│   ├── Metaheuristics.h
│   └── SortedList.h
├── implementations/
│   ├── exact/              # Exact algorithms
│   │   └── ExhaustiveSearch.c
│   ├── graph/              # Graph utilities
│   │   ├── Graph.c
│   │   └── SortedList.c
│   ├── heuristics/         # Heuristic algorithms
│   │   ├── Greedy.c
│   │   ├── NearestNeighbour.c
│   │   ├── NearestInsertion.c
│   │   ├── Christofides.c
│   │   └── blossom/        # Blossom implementation (C++) (imported)
│   ├── metaheuristics/     # Meta-heuristics
│   │   ├── TwoOpt.c
│   │   ├── SimulatedAnnealing.c
│   │   ├── GeneticAlgorithm.c
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

**Note**: In the future, I would like to implement IPSO, as described in this paper: https://www.researchgate.net/publication/271285365_ISPO_A_New_Way_to_Solve_Traveling_Salesman_Problem.

--- 
## Metaheuristic Algorithms

The macro configurations for the metaheuristic algorithms can be tuned in headers/Metaheuristics.h.

---

## Implemented Algorithms

| Algorithm                 | Type            | Complexity (worst case)                | Notes                                                                 |
| :------------------------ | :-------------: | :------------------------------------: | :------------------------------------------------------------------- |
| **Exhaustive Search**     | Exact           | $O(N!)$                                | Finds the optimal solution; infeasible for $N > 12–15$.              |
| **Nearest Neighbor**      | Heuristic       | $O(N^2)$                               | Fast, but solution quality may vary; starting point affects the tour. |
| **Greedy Heuristic**      | Heuristic       | $O(N^2 \log N)$                        | Builds a tour by repeatedly selecting the shortest available edge.   |
| **Nearest Insertion** | Heuristic | $O(N^2)$ | Constructive method: selects the unvisited node closest to any edge in the current partial tour. |
| **Christofides Algorithm**| Heuristic       | $O(N^3)$                               | Guarantees a tour $\le 1.5\times$ optimal for metric TSP. Could not get it working correctly. |
| **2-Opt Improvement**     | Meta-heuristic  | $O(N^2 \times \text{iterations})$      | Local search to improve an existing tour; often used after other algorithms. |
| **Simulated Annealing**   | Meta-heuristic  | $O(N^2 \times \text{iterations})$      | Probabilistic improvement using 2-opt swaps. |
| **Ant Colony Optimization** | Meta-heuristic | $O(N^2 \times \text{iterations} \times \text{ants})$ | Probabilistic search guided by pheromone trails; computationally heavier but can yield high-quality solutions. |
| **Genetic Algorithm (GA)** | Meta-heuristic | $O(N^2 \times \text{Generations} \times \text{Population})$ | Population-based search simulating evolution (selection, crossover, mutation). |
| **Lower Bound (MST)**     | Utility         | $O(E \log N)$                          | Provides a minimum cost estimate using a **Minimum Spanning Tree**. |

**Note**: Time complexity does not tell the whole story... Meta-heuristic algorithms are more computationaly heavy than Christofides, being guided by their parameters/configuration, even though Christofides' algorithm has the worst, worst-case time complexity ($O(N^3)$).

---

## Compilation Instructions

This project uses **GCC** and **G++** to compile C and C++ code (for the imported Blossom algorithm).

1. **Clone the repository:**

    ```bash
    git clone https://github.com/nelsonramosua/TSP
    cd TSP
    ```

2. **Build the project:**

    ```bash
    make
    ```

3. **Run the comparison tester:**

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
* Genetic Algorithm performance is highly dependent on parameters. 
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
* https://www.researchgate.net/publication/271285365_ISPO_A_New_Way_to_Solve_Traveling_Salesman_Problem
* (and a few more, indicated in their respective C files)

--- 

## Author

Nelson Ramos (124921).