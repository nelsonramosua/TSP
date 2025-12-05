# Traveling Salesman Problem (TSP) Solver

This repository implements multiple algorithms to solve the **Traveling Salesman Problem (TSP)**, including exact methods, heuristics, and meta-heuristics. It aims to compare the (best) Tours (paths, if you want) found by each.

For purposes of this project, I chose to reuse the Graph implementation by AED Professors, which also uses a Sorted List. It would, in fact, have been better to create a new ADT, but for time reasons, I reused that one.

I do not deserve (full) credit for this project. I based my implementations on resources available on the internet (research papers, youtube videos, blog posts, etc.); furthermore, I had AI assistance (Claude Opus 4.5) for some of the implementations.
Some of these are mere transcriptions of code available online (mostly C++ code). Thus, I do not take credit for those implementations. This was done only for comparison.

This project was done for academic and fun purposes, and thus should not be taken seriously into a production environment.

---

## Project Structure

```
TSP/
├── headers/                    # Header files
│   ├── Graph.h
│   ├── GraphFactory.h
│   ├── HashMap.h
│   ├── LowerBounds.h
│   ├── Metaheuristics.h
│   ├── NamedGraph.h            # Abstraction layer for graphs with names as vertices.
│   ├── TSPTest.h               # Test driver header
│   └── SortedList.h
├── implementations/
│   ├── exact/                  # Exact algorithms
│   |   ├── HeldKarp.c
│   |   ├── ExhaustiveSearchPruning.c
│   │   └── ExhaustiveSearch.c
│   ├── graph/                  # Graph utilities
│   │   ├── Graph.c
│   │   ├── HashMap.c           # !SIMPLE! AuxiliaryADT used in NamedGraph to keep mapping (vertexId, cityName).
│   │   ├── NamedGraph.c        # Abstraction layer for graphs with names as vertices.
│   │   └── SortedList.c
│   ├── heuristics/             # Heuristic algorithms
│   │   ├── Greedy.c
│   │   ├── NearestNeighbour.c
│   │   ├── NearestInsertion.c
│   │   ├── Christofides.c
│   │   └── blossom/            # Blossom implementation (C++) (imported!)
│   ├── lowerBounds/            # Lower Bound algorithms
│   │   ├── LowerBound_HeldKarp.c
│   │   └── LowerBound_MST.c
│   ├── metaheuristics/         # Meta-heuristics
│   │   ├── TwoOpt.c
│   │   ├── SimulatedAnnealing.c
│   │   ├── GeneticAlgorithm.c
│   │   └── AntColony.c
│   └── mst/                    # Minimum Spanning Tree utilities (used by Christofides.c & LowerBound_MST.c).
│       └── Prim_MST.c
├── graphs/                     # Predefined and generated graphs (.dot)
├── build/                      # build objects, when you run make, will go here.
├── TSPTest.c                   # Comparison driver
├── Tour.c                      # Tour structure and utilities
├── GraphFactory.c              # Predefined graph generation
├── Makefile
└── TravelingSalesmanProblem.h  # Main project header
```

**Note**: In the future, I would like to implement IPSO, as described in this paper: https://www.researchgate.net/publication/271285365_ISPO_A_New_Way_to_Solve_Traveling_Salesman_Problem.\
For the moment, I will leave that as an exercise to the reader. :)

--- 
## Metaheuristic Algorithms

The macro configurations for the metaheuristic algorithms can be tuned in headers/Metaheuristics.h.

---

## Implemented Algorithms

| Algorithm                 | Type            | Time Complexity (worst case)                | Notes                                                                 |
| :------------------------ | :-------------: | :------------------------------------: | :------------------------------------------------------------------- |
| **Exhaustive Search**     | Exact           | $O(N!)$                                | Finds the optimal solution; infeasible for $N \ge 12$.              |
| **Exhaustive Search with Pruning** | Exact           | $O(N!)$                                | Optimized exhaustive search that prunes branches based on cost bounds. Still infeasible for $N \ge 12$.|
| **Held-Karp Algorithm**            | Exact           | $O(N^2 \times 2^N)$                    | Dynamic programming approach; finds the optimal solution for small graphs. Too slow for $N \ge 20$.|
| **Nearest Neighbour**      | Heuristic       | $O(N^2)$                               | Fast, but solution quality may vary; starting point affects the tour. |
| **Greedy Heuristic**      | Heuristic       | $O(N^3)$                        | Builds a tour by repeatedly selecting the shortest available edge.   |
| **Nearest Insertion** | Heuristic | $O(N^3)$ | Constructive method: selects the unvisited node closest to any edge in the current partial tour. |
| **Christofides Algorithm**| Heuristic       | $O(N^3)$                               | Guarantees a tour $\le 1.5\times$ optimal for metric TSP. Could not get it working correctly. |
| **2-Opt Improvement**     | Meta-heuristic  | $O(N^3)$      | Local search to improve an existing tour; often used after other algorithms. |
| **Simulated Annealing**   | Meta-heuristic  | $O(N^2 \times \text{Iterations}) = O(N^3 \times \text{SA-MULTIPLIER})$      | Probabilistic improvement using 2-opt swaps. |
| **Ant Colony Optimization** | Meta-heuristic | $O(N^2 \times \text{Iterations} \times \text{Ants}) = O(N^3 \times \text{Iterations})$ | Probabilistic search guided by pheromone trails; computationally heavier but can yield high-quality solutions. |
| **Genetic Algorithm (GA)** | Meta-heuristic | $O(N^2 \times \text{Generations} \times \text{Population})$ | Population-based search simulating evolution (selection, crossover, mutation). Depending on the # Generations, it can be very slow. For the current parameters, that's for around $N \ge 55$.|
| **Lower Bound MST**     | Utility         | $O(N^2)$                          | Provides a minimum cost estimate using a **Minimum Spanning Tree**. |
| **Lower Bound Held-Karp Lagrangian Relaxation**     | Utility         | $O(N^2)$                          | Provides a minimum cost estimate using a **Held-Karp Lagrangian Relaxation**. |

**Notes**: 
Time complexity does not tell the whole story... even more so for small $N$ (numVertices). 
Meta-heuristic algorithms are more computationaly heavy than Christofides, being guided by their parameters/configuration, even though Christofides' algorithm has the worst, worst-case time complexity, $O(N^3)$. \
Some of these (pex: MST Lower Bound & Greedy) could be improved if an auxiliary ADT, min-heap / priority queue, had been introduced and used. They would then have complexities, respectively, $O(E \times log N)$ and $O(N^2 \times log N)$. For simplicity purposes, it wasn't. **Go ahead and try! :)**

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

**Note**: you can also run `` make run `` to compile and run automatically, and even ``make runvc`` to compile, run with valgrind and then clean, automatically.\
However, I caution you to not run past the first test graph with valgring, ad it will incredibly slow (test it!).
**See Makefile for more info on this**.

---

## Graphs

The project includes, out-of-the-box:

* The **Graph presented in AED Class 23**, `CreateGraphAula`.
* A **real-world Graph**, based on **Portuguese cities**, `CreatePortugal12CitiesGraph`.
* Another **real-world Graph**, based on **European cities**, `CreateEurope12CitiesGraph`, corroborating the results of this paper: https://www.researchgate.net/publication/362233733_Determining_Best_Travelling_Salesman_Route_of_12_Cities_of_Europe.
* **Predefined Matrix Graphs:** `CreateMatrixGraph15` and `CreateMatrixGraph20`.
* **Fixed Euclidean Graph:** `CreateEuclideanGraph15`.
* **TSPLIB Graphs**: `CreateEil51Graph`, `CreateOliver30Graph`, `CreateSwiss42Graph`, `CreateBays29Graph` and `CreateA280Graph` (for which ACTUAL best tour costs are presented).

For $N \le 20$, Held-Karp provides ACTUAL best tour costs for the other algorithms (non-TSPLIB). For $N$ over that it would be infeasible to run that algorithm. So, unless you KNOW the actual best tour cost for your algorithm, you won't have any guarantee, because for $N \ge 20$ the exact algorithms do not have runtime to proceed (testing would be too slow)... \
**Read `GraphFactory.c`** for more info on how you can pass the actualBestTourCost parameter to the Test Driver functions.

Graph outputs in DOT format are saved in `graphs/` and can be visualized directly in VSCODE with **Graphviz** extension or via terminal (or even pasted on their website):

```bash
dot -Tpng graphs/testGraph.dot -o graphs/testGraph.png
```
**Note**: Only if cities names are set for each vertex will the Tour and DOT generation use those names. Otherwise, the default is the vertex index (0 upto numVertices). See, pex, graphs/GraphAula.dot & Bays29Graph.dot.\
**Read `GraphFactory.c`** for more info on this.

---

## Notes
* Some of these will fail for very sparse graphs... Test it and try. That's connected to how TSP is defined.
* For small graphs ($N \le 12$), Exhaustive Search guarantees optimal results. However, it is disabled. Uncomment to enable.
* For medium-sized graphs ($N \le 100$), heuristics like Christofides, Greedy, and Nearest Neighbor are recommended and will provide good approximations.
* For large graphs ($N > 100$), meta-heuristics like Simulated Annealing and Ant Colony Optimization provide the best **approximations**.
* Use 2-Opt Improvement as a post-processing step to refine heuristic solutions. At the moment, that's only being done for the Nearest Neighbour Heuristic.
* Genetic Algorithm performance is highly dependent on parameters (the other metaheuristic algorithms are as well, but, because in these specific implementations, GA allocates and deallocates the most memory (creation and destruction of Populations and Individuals across the number of Generations defined), it will be the slowest (this can be shown further by running with valgrind); the others (SA and ACO) used `unsigned int*` to describe paths, which is more memory efficient, despite not being as descriptive (and that would take us into analyzing Space Complexity as well... but that was not done here)).
* I could not for the life of me get Christofides working well... I tried everything... As a last ditch effort, I included a C++ implementation of the Blossom algorithm for computing the Minimum Weight Perfect Matching (MWPM)... But that does not work that well either.

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
* https://www.sciencedirect.com/science/article/pii/S0377221796002147
* https://github.com/corail-research/learning-hk-bound/blob/main/solver/src/hk2.cc
* (and a few more, indicated in their respective C files)

--- 

## Author

Nelson Ramos (124921),\
November 2025.

(This project, once again I remind, used work of other people on the internet, that can be found in the links throughout its implementations)!