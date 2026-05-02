# Planet-Scale Internet Outage Simulator - Context Guide

## Project Overview

This is a **terminal-based discrete event simulator** that models planet-scale internet infrastructure and simulates cascading failure scenarios. The system uses **8 different AI algorithms** to predict vulnerabilities, optimize network hardening, and provide intelligent failure analysis.

### Core Concept
The simulator models the global internet as a **graph network** with nodes representing critical infrastructure (submarine cables, data centers, satellites) and edges representing connections with bandwidth/latency properties. It can inject failures and observe how they cascade through the network, while AI algorithms provide predictive analysis and optimization recommendations.

---

## System Architecture

### 1. **Graph Network Foundation** (`graph/`)
- **`topology.h/c`**: Core graph structure with adjacency lists
- **`node.h/c`**: Network nodes (submarine cables, data centers, satellites)
- **`edge.h/c`**: Network connections with bandwidth/latency/failure probability

**Key Data Structures:**
```c
typedef struct Graph {
    Node*  nodes;           // Array of network infrastructure nodes
    Edge*  edges;           // Array of network connections
    int    node_count;      // Current number of nodes
    int    edge_count;      // Current number of edges
    int**  adj;             // Adjacency list for fast pathfinding
    int*   adj_count;       // Edge count per node
} Graph;

typedef struct Node {
    int   id;               // Unique identifier
    char  name[64];         // Human-readable name (e.g., "SEA-ME-WE-4")
    char  region[32];       // Geographic region
    int   type;             // SUBMARINE_CABLE, DATACENTER, SATELLITE
    int   capacity;         // Bandwidth capacity in Mbps
    int   current_load;     // Current traffic load
    int   is_active;        // 1=online, 0=failed
    float risk_score;       // AI-calculated vulnerability (0-100)
    float latitude, longitude; // Geographic coordinates
} Node;
```

### 2. **Discrete Event Simulation Engine** (`simulation/`)
- **`scheduler.h/c`**: Time-ordered event queue with event types:
  - `EVT_FAILURE`: Infrastructure component fails
  - `EVT_CASCADE`: Failure spreads to connected components
  - `EVT_RECOVERY`: Component comes back online
  - `EVT_REROUTE`: Traffic rerouting occurs
  - `EVT_OVERLOAD`: Component exceeds capacity
- **`metrics.h/c`**: Real-time network health metrics by region

**Event System:**
```c
typedef struct Event {
    int   time;             // Simulation minute when event occurs
    int   type;             // Event type (failure, cascade, recovery, etc.)
    int   target_node;      // Which node is affected (-1 if N/A)
    int   target_edge;      // Which edge is affected (-1 if N/A)
    char  description[128]; // Human-readable event description
    struct Event* next;     // Linked list for chronological ordering
} Event;
```

### 3. **Failure Modeling System** (`failures/`)
- **`failure_engine.h/c`**: Applies node/edge failures and recoveries
- **`cascade.h/c`**: Models how failures propagate through the network

### 4. **AI Algorithm Suite** (`ai/`)

#### **4.1 Hidden Markov Models (HMM)** - `hmm.h/c`
- **Purpose**: Anomaly detection and failure prediction
- **States**: HEALTHY → DEGRADED → CRITICAL → FAILED
- **Observations**: Network latency and load ratios
- **Use Case**: Predicts when a node will likely fail based on observed performance patterns

#### **4.2 Bayesian Networks** - `bayesian.h/c`
- **Purpose**: Cascade probability prediction
- **Method**: Builds conditional probability tables from network topology
- **Use Case**: "If node A fails, what's the probability node B will also fail?"

#### **4.3 Q-Learning (Reinforcement Learning)** - `qlearning.h/c`
- **Purpose**: Learns optimal network management policies
- **Actions**: REROUTE, ACTIVATE_BACKUP, THROTTLE, INCREASE_CAP, DO_NOTHING
- **Use Case**: Trains an agent to make optimal decisions during network stress

#### **4.4 Genetic Algorithm** - `genetic.h/c`
- **Purpose**: Network hardening optimization
- **Chromosome**: Array of capacity upgrades for each node
- **Fitness**: Network resilience after applying upgrades
- **Use Case**: "Given budget X, which nodes should we upgrade for maximum resilience?"

#### **4.5 Minimax Algorithm** - `minimax.h/c`
- **Purpose**: Adversarial threat assessment
- **Method**: Game theory - attacker vs. defender
- **Use Case**: Identifies worst-case attack scenarios

#### **4.6 Expert System** - `expert_system.h/c`
- **Purpose**: Rule-based vulnerability analysis
- **Rules**: IF-THEN conditions based on network topology
- **Use Case**: Provides human-readable vulnerability assessments

#### **4.7 A* Pathfinding** - `astar.h/c`
- **Purpose**: Optimal routing and rerouting
- **Use Case**: Finds best paths when primary routes fail

#### **4.8 Heuristics** - `heuristics.h/c`
- **Purpose**: Betweenness centrality analysis
- **Use Case**: Identifies critical nodes whose failure would most impact connectivity

### 5. **Data Loading System** (`data/`)
- **`loader.h/c`**: Loads network topology from multiple sources
- **`real_topology.h/c`**: Real-world internet infrastructure data
- **Supported Formats**: CSV, JSON, hardcoded fallback

### 6. **Output & Visualization** (`output/`)
- **`visualizer.h/c`**: ASCII world map showing network status
- **`report.h/c`**: AI-generated prediction and resilience reports

---

## Network Topology Data

### **Real Infrastructure Modeled:**
- **Submarine Cables**: SEA-ME-WE-4, FLAG-Cable, TAT-14, SEACOM, SAm-1, Southern-Cross
- **Data Centers**: AWS regions, Google Cloud regions, Microsoft Azure regions
- **Satellites**: Intelsat, SES, Eutelsat networks
- **Geographic Regions**: North America, Europe, Asia-Pacific, South America, Africa, Oceania

### **Data Format (topology.csv):**
```csv
type,name,region,lat,lon,capacity
SUBMARINE,SEA-ME-WE-4,Asia-Pacific,13.0,80.0,1280000
DATACENTER,AWS-us-east,North America,39.0,-77.0,999999
SATELLITE,Intelsat-Asia,Asia-Pacific,0.0,100.0,50000
```

---

## Main Program Flow (`main.c`)

### **Interactive Menu System:**
1. **Failure Simulations**: Inject specific failure types (cable cuts, DC failures, satellite outages)
2. **Multi-Failure Scenarios**: Complex cascading failure simulations (e.g., solar storm)
3. **AI Analysis**: Run vulnerability scans, generate predictions, optimization recommendations
4. **Network Inspection**: View topology, metrics, real-time status
5. **Data Management**: Load new datasets, reset simulation state
6. **Advanced AI**: Genetic optimization, adversarial testing

### **Simulation Loop:**
1. Load network topology from data files
2. Initialize AI algorithms and metrics
3. Present interactive menu to user
4. Execute selected simulation or analysis
5. Display results via ASCII visualization and reports
6. Return to menu for next operation

---

## Key Features

### **Real-Time Visualization:**
- ASCII world map showing node status by region
- Color-coded health indicators (HEALTHY/DEGRADED/CRITICAL/FAILED)
- Live metrics table with connectivity percentages, latency, packet loss

### **AI-Driven Analysis:**
- **Predictive**: HMM and Bayesian networks predict future failures
- **Optimization**: Genetic algorithms optimize network hardening strategies
- **Adversarial**: Minimax identifies worst-case attack scenarios
- **Reactive**: Q-Learning learns optimal response policies

### **Realistic Failure Modeling:**
- **Cascading Effects**: Failures propagate through network dependencies
- **Load Redistribution**: Traffic reroutes when primary paths fail
- **Capacity Constraints**: Overloaded links can trigger additional failures
- **Recovery Scenarios**: Gradual restoration with realistic timelines

---

## Build & Execution

### **Compilation:**
```bash
make clean && make
```

### **Execution:**
```bash
./simulator.exe          # Interactive menu mode
./simulator --train-rl   # Train Q-Learning agent
```

### **Dependencies:**
- **C99 Standard**: Modern C features
- **Math Library**: `-lm` for floating-point calculations
- **No External Dependencies**: Pure C implementation

---

## File Organization

```
├── ai/                    # 8 AI algorithms (HMM, Bayesian, Q-Learning, etc.)
│   ├── astar.c/h         # A* pathfinding
│   ├── bayesian.c/h      # Bayesian network inference
│   ├── expert_system.c/h # Rule-based analysis
│   ├── genetic.c/h       # Genetic algorithm optimization
│   ├── heuristics.c/h    # Betweenness centrality
│   ├── hmm.c/h           # Hidden Markov Models
│   ├── minimax.c/h       # Game theory analysis
│   └── qlearning.c/h     # Reinforcement learning
├── data/                  # Data loading and topology management
│   ├── loader.c/h        # Multi-format data loader
│   └── real_topology.c/h # Real-world infrastructure data
├── failures/              # Failure simulation engine
│   ├── cascade.c/h       # Cascading failure modeling
│   └── failure_engine.c/h # Core failure mechanics
├── graph/                 # Network graph data structures
│   ├── edge.c/h          # Network connections
│   ├── node.c/h          # Infrastructure nodes
│   └── topology.c/h      # Graph management
├── output/                # Visualization and reporting
│   ├── report.c/h        # AI-generated reports
│   └── visualizer.c/h    # ASCII world map
├── simulation/            # Discrete event simulation
│   ├── metrics.c/h       # Network health metrics
│   └── scheduler.c/h     # Event queue management
├── main.c                 # Main program and menu system
├── Makefile              # Build configuration
├── topology.csv          # Network topology data (CSV format)
├── topology.json         # Network topology data (JSON format)
└── README.md             # Project documentation
```

---

## Usage Scenarios

### **Research & Education:**
- Study cascading failure patterns in global internet infrastructure
- Experiment with different AI approaches to network resilience
- Analyze real-world topology vulnerabilities

### **Network Planning:**
- Optimize infrastructure investments using genetic algorithms
- Identify critical nodes using centrality analysis
- Test resilience against various failure scenarios

### **Security Analysis:**
- Model adversarial attacks using minimax algorithms
- Assess vulnerability to coordinated infrastructure attacks
- Develop response strategies using reinforcement learning

---

## Technical Notes

### **Performance Characteristics:**
- **Graph Operations**: O(V + E) for most traversals
- **AI Algorithms**: Varies by algorithm (HMM: O(T×N²), Genetic: O(G×P×N))
- **Memory Usage**: Scales with network size (typically <100MB for global topology)

### **Extensibility:**
- **Modular Design**: Each AI algorithm is self-contained
- **Data Format Support**: Easy to add new topology data sources
- **Event Types**: Simple to add new failure/recovery scenarios
- **Visualization**: ASCII output can be extended to other formats

### **Limitations:**
- **Terminal-Only**: No GUI (OpenGL implementation was removed due to stability issues)
- **Single-Threaded**: No parallel processing of AI algorithms
- **Static Topology**: Network structure doesn't change during simulation (only status)

---

This simulator provides a comprehensive platform for studying internet resilience using multiple AI approaches, making it valuable for research, education, and practical network planning scenarios.