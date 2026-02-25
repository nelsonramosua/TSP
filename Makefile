CC = gcc
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
         $(MST_DIR)/Prim_MST.c \
         $(LOWER_BOUNDS_DIR)/LowerBound_MST.c \
         $(LOWER_BOUNDS_DIR)/LowerBound_HeldKarp.c \
         $(EXACT_DIR)/ExhaustiveSearch.c \
         $(EXACT_DIR)/ExhaustiveSearchPruning.c \
         $(EXACT_DIR)/HeldKarp.c \
         $(HEUR_DIR)/NearestNeighbour.c \
         $(HEUR_DIR)/Greedy.c \
         $(HEUR_DIR)/NearestInsertion.c \
         $(HEUR_DIR)/Christofides.c \
         $(HEUR_DIR)/blossom/BlossomWrapper.c \
         $(META_DIR)/TwoOpt.c \
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

clean:
	rm -rf $(BUILD_DIR) $(TSP_COMPARISON)

loc:
	@echo "Lines of code (C/H source files):"
	@find . \( -name '*.c' -o -name '*.h' \) | grep -v builds | xargs wc -l | sort -rn | head -20

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
	@echo "Info targets:"
	@echo "  make loc          - Line count per source file"
	@echo "  make help         - Show this message"
	@echo ""

.PHONY: all run runvc clean rebuild loc help