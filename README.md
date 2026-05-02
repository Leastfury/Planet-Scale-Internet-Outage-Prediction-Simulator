# Planet-Scale Internet Outage Simulator

A comprehensive terminal-based simulator that models global internet infrastructure and simulates cascading failure scenarios. The project uses multiple classical AI algorithms to analyze network vulnerabilities and predict failure patterns in real-world internet topology.

## Features
- **Real Global Internet Topology**: Models submarine cables, data centers, and satellite links across 6 major regions
- **Discrete Event Simulation**: Time-based event scheduler with chronological failure progression
- **Live ASCII Visualization**: Dynamic world map showing network status in real-time
- **8 Classical AI Algorithms**: Dijkstra, A*, HMM, Bayesian Networks, Q-Learning, Genetic Algorithm, Minimax, Expert Systems
- **Cascading Failure Engine**: Realistic load redistribution and overload simulation
- **Vulnerability Analysis**: Betweenness centrality and risk scoring for critical infrastructure
- **Multiple Data Sources**: Support for CSV, JSON, and hardcoded topology data

## Quick Start

### Windows Compilation
```bash
# Using Make (if available)
make clean
make
./simulator.exe

# Direct GCC compilation
gcc -std=c99 -Wall -o simulator.exe main.c graph/topology.c graph/node.c graph/edge.c failures/failure_engine.c failures/cascade.c ai/astar.c ai/hmm.c ai/bayesian.c ai/qlearning.c ai/genetic.c ai/minimax.c ai/expert_system.c ai/heuristics.c simulation/scheduler.c simulation/metrics.c output/visualizer.c output/report.c data/loader.c data/real_topology.c -lm
```

### Linux/macOS Compilation
```bash
make clean
make
./simulator
```

## AI Algorithms & Applications

The simulator implements 8 distinct classical AI approaches for network analysis:

| Algorithm | Purpose | Implementation |
|-----------|---------|----------------|
| **Dijkstra/A*** | Shortest path routing and emergency rerouting | `ai/astar.c` |
| **Hidden Markov Models** | Anomaly detection and failure pattern recognition | `ai/hmm.c` |
| **Bayesian Networks** | Probabilistic cascade failure prediction | `ai/bayesian.c` |
| **Q-Learning** | Reinforcement learning for adaptive network management | `ai/qlearning.c` |
| **Genetic Algorithm** | Network hardening optimization and resource allocation | `ai/genetic.c` |
| **Minimax** | Adversarial threat assessment and worst-case analysis | `ai/minimax.c` |
| **Expert System** | Rule-based vulnerability analysis and recommendations | `ai/expert_system.c` |
| **Heuristics** | Betweenness centrality and critical node identification | `ai/heuristics.c` |

## Simulation Scenarios

The simulator provides 13 interactive menu options:

1. **Submarine Cable Cut** - Simulate undersea cable failures
2. **Data Center Failure** - Model major cloud provider outages  
3. **Satellite Outage** - Test satellite communication disruptions
4. **Solar Storm** - Multi-failure cascade scenario with realistic timeline
5. **AI Vulnerability Scan** - Identify critical infrastructure bottlenecks
6. **AI Prediction Report** - Generate failure probability assessments
7. **Resilience Recommendations** - AI-generated network hardening advice
8. **Network Topology View** - Inspect current node and edge status
9. **Region Metrics Table** - Real-time connectivity and performance data
10. **Load Dataset** - Import custom topology from CSV/JSON files
11. **Reset Simulation** - Restore network to initial state
12. **Genetic Algorithm** - Optimize network hardening with budget constraints
13. **Adversarial Stress Test** - Minimax-based worst-case scenario analysis

## Architecture Overview

```
├── ai/                 # 8 AI algorithms for network analysis
│   ├── astar.c/h      # A* pathfinding and rerouting
│   ├── bayesian.c/h   # Probabilistic failure modeling
│   ├── dijkstra.c/h   # Shortest path calculations
│   ├── expert_system.c/h # Rule-based vulnerability assessment
│   ├── genetic.c/h    # Network optimization algorithms
│   ├── heuristics.c/h # Centrality and risk scoring
│   ├── hmm.c/h        # Hidden Markov Models for anomaly detection
│   ├── minimax.c/h    # Adversarial analysis
│   └── qlearning.c/h  # Reinforcement learning
├── data/              # Topology data management
│   ├── loader.c/h     # CSV/JSON/hardcoded data loading
│   └── real_topology.c/h # Real-world internet infrastructure data
├── failures/          # Failure simulation engine
│   ├── cascade.c/h    # Load redistribution and overload modeling
│   └── failure_engine.c/h # Event-driven failure injection
├── graph/             # Network representation
│   ├── edge.c/h       # Cable/link modeling with bandwidth/latency
│   ├── node.c/h       # Infrastructure nodes (cables, DCs, satellites)
│   └── topology.c/h   # Graph structure and adjacency management
├── output/            # Visualization and reporting
│   ├── report.c/h     # AI analysis report generation
│   └── visualizer.c/h # ASCII world map and real-time display
├── simulation/        # Core simulation engine
│   ├── metrics.c/h    # Network performance and connectivity tracking
│   └── scheduler.c/h  # Discrete event scheduling system
├── main.c             # Interactive menu and program entry point
├── Makefile           # Cross-platform build configuration
├── topology.csv       # Sample network topology data
└── topology.json      # Alternative JSON topology format
```

## Technical Details

### Network Modeling
- **Nodes**: Represent physical infrastructure (submarine cable stations, data centers, satellites)
- **Edges**: Model physical connections (fiber optic cables, satellite links) with realistic bandwidth and latency
- **Topology**: Efficient adjacency list representation for sparse global networks

### Simulation Engine
- **Discrete Event Scheduler**: Time-efficient event queue that skips idle periods
- **Event Types**: Failures, cascades, rerouting, overloads, and recoveries
- **Real-time Metrics**: Live tracking of connectivity, performance, and regional isolation

### AI-Driven Analysis
- **Vulnerability Scoring**: Betweenness centrality identifies critical bottleneck nodes
- **Predictive Modeling**: Bayesian networks assess cascade failure probabilities  
- **Optimization**: Genetic algorithms find optimal network hardening strategies
- **Adversarial Testing**: Minimax algorithms identify worst-case attack scenarios

## Command Line Options

```bash
# Train reinforcement learning model
./simulator --train-rl

# Standard interactive mode
./simulator
```

## Data Formats

### CSV Format (topology.csv)
```csv
node_id,name,type,region,capacity
edge_id,from_node,to_node,bandwidth,latency,failure_prob
```

### JSON Format (topology.json)
```json
{
  "nodes": [
    {"id": 0, "name": "SEA-ME-WE-4", "type": "SUBMARINE_CABLE", "region": "Asia", "capacity": 1000}
  ],
  "edges": [
    {"id": 0, "from": 0, "to": 1, "bandwidth": 10000, "latency": 50, "failure_probability": 0.01}
  ]
}
```

## Contributing

This project models real-world internet infrastructure for educational and research purposes. Contributions welcome for:
- Additional AI algorithms
- Enhanced visualization
- Real-world topology data
- Performance optimizations

## License

Open source educational project. See individual algorithm implementations for specific licensing details.