# Planet-Scale Outage Simulator: Technical Documentation Summary

This document provides a detailed overview of all modules and files within the network simulator project. It is synthesized from the technical specifications found in the `code_docs/` directory.

---

## 1. Core Graph Infrastructure (`graph/`)
The foundation of the simulator, representing the physical and logical network.

### `graph/topology.h` & `graph/topology.c`
*   **Purpose**: Implements the core graph data structure using dynamic arrays and adjacency lists.
*   **Key Functions**: 
    *   `init_graph()`: Allocates memory for nodes, edges, and adjacency lists.
    *   `add_node()` & `add_edge()`: Dynamically grows the network.
    *   `free_graph()`: Ensures clean memory deallocation.
*   **Role**: The central data container used by every other module.

### `graph/node.h` & `graph/node.c`
*   **Purpose**: Defines the `Node` structure (Datacenters, Submarine Cables, Satellites).
*   **Key Data**: Stores ID, name, region, geographical coordinates (Lat/Long), capacity, load, and risk scores.
*   **Regional Logic**: Maps strings to integer region IDs (e.g., "North America" to `REGION_NA`).

### `graph/edge.h` & `graph/edge.c`
*   **Purpose**: Defines the `Edge` structure representing communication links.
*   **Key Data**: Bandwidth (Mbps), Latency (ms), Failure Probability, and "pheromone" levels for ACO-inspired routing.

---

## 2. Simulation Engine & Failures (`simulation/` & `failures/`)
Handles the "behavior" of the network over time.

### `simulation/scheduler.h` & `simulation/scheduler.c`
*   **Purpose**: A discrete-event simulation engine.
*   **Mechanics**: Manages a time-sorted linked list of `Event` nodes (Failure, Recovery, Cascade, etc.).
*   **AI Integration**: Triggers HMM failure predictions and Q-Learning reactions within the simulation loop.

### `simulation/metrics.h` & `simulation/metrics.c`
*   **Purpose**: Real-time performance tracking.
*   **Metrics**: Calculates regional Connectivity %, Packet Loss (based on overload), and Time to Restore (TTR).

### `failures/failure_engine.h` & `failures/failure_engine.c`
*   **Purpose**: Low-level "kill/revive" primitives.
*   **Logic**: Modifies the `is_active` status of nodes/edges and manages the downstream effects on connectivity.

### `failures/cascade.h` & `failures/cascade.c`
*   **Purpose**: Simulates "chain reaction" failures.
*   **Logic**: When a node fails, traffic is redistributed. If a neighboring edge's bandwidth is exceeded, a new `EVT_CASCADE` or `EVT_OVERLOAD` is scheduled.

---

## 3. Artificial Intelligence Modules (`ai/`)
Advanced logic for pathfinding, risk assessment, and strategic optimization.

### Pathfinding: `astar.c/h` & `dijkstra.c/h`
*   **Dijkstra**: Baseline shortest-path algorithm based on cumulative latency.
*   **A***: Optimized point-to-point pathfinding using the **Haversine formula** (geographic distance) as a heuristic. Used primarily for traffic rerouting.

### Risk Assessment: `heuristics.c/h` & `expert_system.c/h`
*   **Heuristics**: Estimates node importance using **Betweenness Centrality** (measuring reachability loss upon node removal).
*   **Expert System**: A rule-based engine applying qualitative rules (e.g., identifying single points of failure in specific geographic zones).

### Probabilistic & Predictive: `bayesian.c/h` & `hmm.c/h`
*   **Bayesian Network**: Models dependencies between nodes to calculate the probability of a node failing given its neighbors have already failed.
*   **HMM (Hidden Markov Model)**: Infers hidden "health states" (Healthy, Degraded, Critical) from observable metrics like load/latency to predict failures before they happen.

### Strategic Optimization: `genetic.c/h`, `minimax.c/h`, & `qlearning.c/h`
*   **Genetic Algorithm**: Evolves "hardening" plans (capacity upgrades) to find the best resilience improvement within a fixed budget.
*   **Minimax (with Alpha-Beta)**: Simulates an adversarial game between an Attacker (creating failures) and a Defender (restoring paths) to find worst-case scenarios.
*   **Q-Learning (RL)**: Trains an agent to manage network events (rerouting, throttling) by learning from rewards in a simulated environment.

---

## 4. Data Layer & Orchestration
### `data/loader.c/h` & `data/real_topology.c/h`
*   **Loader**: Supports CSV and JSON ingestion. Falls back to hardcoded data if files are missing.
*   **Real Topology**: A pre-defined dataset of major global infrastructure (e.g., Google-US-East, SEA-ME-WE cables).
*   **`data_formats.txt`**: Detailed specification for `topology.csv` and `topology.json` schemas.

### `main.c` & `Makefile`
*   **Main**: The CLI entry point. Orchestrates complex scenarios like "Solar Storm" and provides the interactive menu for all modules.
*   **Makefile**: Build system using `gcc`, configured for cross-platform support and linking the math library.

---

## 5. Output & Reporting (`output/`)
### `output/visualizer.h/c`
*   **Visuals**: Renders a stylized **ASCII global map** showing regional health and connectivity progress bars.

### `output/report.h/c`
*   **Analytics**: Generates the "AI Vulnerability Report" and human-readable hardening recommendations based on Expert System findings.
