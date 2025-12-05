Graph outputs in DOT format are saved in this folder and can be visualized directly in VSCODE with **Graphviz** extension or via terminal (or even pasted on their website):

```bash
dot -Tpng graphs/testGraph.dot -o graphs/testGraph.png
```

**Note**: Only if cities names are set for each vertex will the Tour and DOT generation use those names. Otherwise, the default is the vertex index (0 upto numVertices). See, pex, graphs/GraphAula.dot & Bays29Graph.dot.\
**Read `GraphFactory.c`** for more info on this.