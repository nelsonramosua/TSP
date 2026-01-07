CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -g -Iheaders -O3 -march=native
CFLAGS_VALGRIND = -Wall -Wextra -g -Iheaders -O3 # if you're using native linux OS, you can run valgrind with binary compiled with -march=native (add it back!)
                                                 # if, however, you're using WSL, you have to run without march-native flag (it will be slower).
CXXFLAGS = -Wall -Wextra -g -Iheaders
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
         $(META_DIR)/TwoOpt.c \
         $(META_DIR)/SimulatedAnnealing.c \
         $(META_DIR)/AntColony.c \
         $(META_DIR)/GeneticAlgorithm.c \
         GraphFactory.c \
         Tour.c \
         TSPTest.c

CPP_SRCS = $(HEUR_DIR)/blossom/Blossom.cpp \
           $(HEUR_DIR)/blossom/BlossomWrapper.cpp

C_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SRCS))
CPP_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(CPP_SRCS))

TSP_COMPARISON = TSP_COMPARISON

all: $(TSP_COMPARISON)

$(TSP_COMPARISON): $(C_OBJS) $(CPP_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: run
run: $(TSP_COMPARISON)
	./$(TSP_COMPARISON)

.PHONY: runvc
runvc: 
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS_VALGRIND)" CXXFLAGS="$(CXXFLAGS)" $(TSP_COMPARISON)
	valgrind --leak-check=full --show-leak-kinds=all -s ./$(TSP_COMPARISON)
	$(MAKE) clean

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TSP_COMPARISON)