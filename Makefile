CC = /usr/bin/gcc
CFLAGS = -Wall -Wextra -g -Iheaders -O3 -march=native
CFLAGS_VALGRIND = -Wall -Wextra -g -Iheaders -O3
# Note: add -march=native back to CFLAGS_VALGRIND if on native Linux (not WSL).

LDFLAGS = -lm

SRC_DIR = implementations

GRAPH_DIR = $(SRC_DIR)/graph
HEUR_DIR = $(SRC_DIR)/heuristics
META_DIR = $(SRC_DIR)/metaheuristics
MST_DIR = $(SRC_DIR)/mst
LOWER_BOUNDS_DIR = $(SRC_DIR)/lowerBounds
EXACT_DIR = $(SRC_DIR)/exact

BUILD_DIR = builds

C_SRCS = $(GRAPH_DIR)/Graph.c \
         $(GRAPH_DIR)/SortedList.c \
         $(GRAPH_DIR)/NamedGraph.c \
         $(GRAPH_DIR)/HashMap.c \
         $(GRAPH_DIR)/PriorityQueue.c \
         $(GRAPH_DIR)/NeighbourList.c \
         $(GRAPH_DIR)/DistanceMatrix.c \
         $(GRAPH_DIR)/Trace.c \
         $(MST_DIR)/Prim_MST.c \
         $(LOWER_BOUNDS_DIR)/LowerBound_MST.c \
         $(LOWER_BOUNDS_DIR)/LowerBound_HeldKarp.c \
         $(EXACT_DIR)/ExhaustiveSearch.c \
         $(EXACT_DIR)/ExhaustiveSearchPruning.c \
         $(EXACT_DIR)/BranchAndBound.c \
         $(EXACT_DIR)/HeldKarp.c \
         $(HEUR_DIR)/NearestNeighbour.c \
         $(HEUR_DIR)/Greedy.c \
         $(HEUR_DIR)/NearestInsertion.c \
         $(HEUR_DIR)/FarthestInsertion.c \
         $(HEUR_DIR)/ClarkeWright.c \
         $(HEUR_DIR)/Christofides.c \
         $(HEUR_DIR)/blossom/BlossomWrapper.c \
         $(META_DIR)/TwoOpt.c \
         $(META_DIR)/OrOpt.c \
         $(META_DIR)/ThreeOpt.c \
         $(META_DIR)/LinKernighan.c \
         $(META_DIR)/TabuSearch.c \
         $(META_DIR)/GRASP.c \
         $(META_DIR)/ISPO.c \
         $(META_DIR)/SimulatedAnnealing.c \
         $(META_DIR)/AntColony.c \
         $(META_DIR)/GeneticAlgorithm.c \
         GraphFactory.c \
         Tour.c \
         TSPTest.c

C_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SRCS))

TSP_COMPARISON = TSP_COMPARISON

# Build targets

all: $(TSP_COMPARISON)

$(TSP_COMPARISON): $(C_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

rebuild: clean all

# Run targets

run: $(TSP_COMPARISON)
	./$(TSP_COMPARISON) $(N)

runvc: 
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS_VALGRIND)" $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all -s ./$(TARGET) $(N)
	$(MAKE) clean

# Utility targets

# Local CodeQL (local static analysis) run. 
# Override CODEQL if the CLI is not on your PATH, e.g.: make codeql CODEQL=/path/to/codeql-bundle/codeql/codeql
CODEQL = codeql
CODEQL_DB = .codeql-db
CODEQL_SUITE = cpp-security-and-quality.qls
CODEQL_SARIF = codeql-results.sarif

# First checks the CLI is installed (else prints how to get it), then builds a database by tracing a clean build and analyses it with the security-and-quality suite. 
# Needs the CodeQL CLI *bundle* (ships the C/C++ query packs).
codeql:
	@command -v $(CODEQL) >/dev/null 2>&1 || { \
		echo "CodeQL CLI not found (tried '$(CODEQL)')."; \
		echo "Install the bundle (includes the query packs) from:"; \
		echo "  https://github.com/github/codeql-action/releases  (codeql-bundle-<os>.tar.gz)"; \
		echo "then add its 'codeql/' directory to PATH, or point this target at it:"; \
		echo "  make codeql CODEQL=/path/to/codeql-bundle/codeql/codeql"; \
		exit 1; \
	}
	@echo "Using $$($(CODEQL) version | head -n 1)"
	$(CODEQL) database create $(CODEQL_DB) --language=cpp --overwrite --command="$(MAKE) rebuild"
	$(CODEQL) database analyze $(CODEQL_DB) $(CODEQL_SUITE) --format=sarif-latest --output=$(CODEQL_SARIF) --threads=0
	@echo ""
	@echo "SARIF written to $(CODEQL_SARIF)."
	@if command -v jq >/dev/null 2>&1; then \
		echo "Findings: $$(jq '[.runs[].results[]] | length' $(CODEQL_SARIF))"; \
		jq -r '.runs[].results[] | "  \(.rule.id)\t\(.locations[0].physicalLocation.artifactLocation.uri):\(.locations[0].physicalLocation.region.startLine)"' $(CODEQL_SARIF); \
	else \
		echo "(install 'jq' to print a findings summary here, or open the SARIF in your editor.)"; \
	fi

gifs:
	@bash tools/make_gifs.sh

clean:
	rm -rf $(BUILD_DIR) $(TSP_COMPARISON) $(CODEQL_DB) $(CODEQL_SARIF)

loc:
	@echo "Lines of code (C/H source files):"
	@find . \( -name '*.c' -o -name '*.h' -o -name '*Makefile' \) | grep -v builds | xargs wc -l | sort -rn | head -20

help:
	@echo ""
	@echo "========================================"
	@echo "  TSP Solver — Makefile Targets"
	@echo "========================================"
	@echo ""
	@echo "Build targets:"
	@echo "  make              - Build TSP_COMPARISON binary"
	@echo "  make rebuild      - Clean then build"
	@echo "  make clean        - Remove build directory and binary"
	@echo ""
	@echo "Run targets:"
	@echo "  make run          - Build and run all graphs"
	@echo "  make run N=3      - Build and run first 3 graphs"
	@echo "  make runvc        - Build (no -march=native), run with Valgrind, clean"
	@echo "  make runvc N=1    - Valgrind on 1 graph (fast, recommended)"
	@echo ""
	@echo "Analysis targets:"
	@echo "  make codeql       - Check CodeQL CLI, build a DB and run security-and-quality"
	@echo "  make codeql CODEQL=/path/to/codeql - Use a CodeQL CLI not on PATH"
	@echo ""
	@echo "Info targets:"
	@echo "  make loc          - Line count per source file"
	@echo "  make help         - Show this message"
	@echo ""
	@echo "Cleaning:"
	@echo "  make clean        - Remove build directory, the binary and the local CodeQL database and SARIF"
	@echo ""

.PHONY: all run runvc clean rebuild loc help codeql gifs