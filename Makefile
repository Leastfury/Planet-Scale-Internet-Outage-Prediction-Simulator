CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

SRCS = main.c \
       graph/topology.c graph/node.c graph/edge.c \
       failures/failure_engine.c failures/cascade.c \
       ai/dijkstra.c ai/expert_system.c ai/heuristics.c \
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
	rm -f $(TARGET)

run: all
	./$(TARGET)
