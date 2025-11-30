# C and C++ compilers
CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -g -std=c99 -Iheaders
CXXFLAGS = -Wall -Wextra -g -std=c++17 -Iheaders
LDFLAGS = -lm

# Source directories
SRC_DIR = implementations
GRAPH_DIR = $(SRC_DIR)/graph
HEUR_DIR = $(SRC_DIR)/heuristics
META_DIR = $(SRC_DIR)/metaheuristics
MST_DIR = $(SRC_DIR)/mst
EXACT_DIR = $(SRC_DIR)/exact
TEST_DIR = tests

# C object files
C_OBJS = $(GRAPH_DIR)/Graph.o \
         $(GRAPH_DIR)/GraphHelpers.o \
         $(GRAPH_DIR)/SortedList.o \
         $(MST_DIR)/Prim_MST.o \
         $(MST_DIR)/LowerBound_MST.o \
         $(EXACT_DIR)/ExhaustiveSearch.o \
         $(HEUR_DIR)/NearestNeighbour.o \
         $(HEUR_DIR)/Greedy.o \
         $(HEUR_DIR)/Christofides.o \
         $(META_DIR)/TwoOpt.o \
         $(META_DIR)/SimulatedAnnealing.o \
         $(META_DIR)/AntColony.o \
         GraphFactory.o \
         Tour.o \
         TSPTest.o

# C++ object files (Blossom)
CPP_OBJS = $(HEUR_DIR)/blossom/Blossom.o \
           $(HEUR_DIR)/blossom/BlossomWrapper.o

# Executable
TSP_COMPARISON: $(C_OBJS) $(CPP_OBJS)
	$(CXX) $(C_OBJS) $(CPP_OBJS) -o $@ $(LDFLAGS)

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
.PHONY: clean
clean:
	rm -f $(C_OBJS) $(CPP_OBJS) TSP_COMPARISON