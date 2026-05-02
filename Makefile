CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

SRCS = main.c \
       graph/topology.c graph/node.c graph/edge.c \
       failures/failure_engine.c failures/cascade.c \
       ai/astar.c ai/hmm.c ai/bayesian.c ai/qlearning.c ai/genetic.c ai/minimax.c ai/expert_system.c ai/heuristics.c \
       simulation/scheduler.c simulation/metrics.c \
       output/visualizer.c output/report.c \
       data/loader.c data/real_topology.c

TARGET = simulator

ifeq ($(OS),Windows_NT)
    TARGET = simulator.exe
endif

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lm

clean:
ifeq ($(OS),Windows_NT)
	del /Q /F $(TARGET) 2>nul || echo "File not found"
else
	rm -f $(TARGET)
endif

run: all
	./$(TARGET)
