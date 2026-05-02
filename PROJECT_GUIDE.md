# Planet-Scale Internet Outage Simulator: Developer Architecture Guide

This comprehensive guide explains the internal architecture, AI implementations, and compilation details for the Planet-Scale Internet Outage Simulator. Designed for developers and engineers who want to understand the technical implementation and extend the codebase.

---

## 1. Compilation and Build System

### Windows Compilation
The project includes a cross-platform Makefile that automatically detects Windows and generates `simulator.exe`:

```bash
# Recommended approach
make clean
make

# Direct GCC compilation (if Make unavailable)
gcc -std=c99 -Wall -o simulator.exe main.c graph/topology.c graph/node.c graph/edge.c failures/failure_engine.c failures/cascade.c ai/astar.c ai/hmm.c ai/bayesian.c ai/qlearning.c ai/genetic.c ai/minimax.c ai/expert_system.c ai/heuristics.c simulation/scheduler.c simulation/metrics.c output/visualizer.c output/report.c data/loader.c data/real_topology.c -lm
```

### Linux/macOS Compilation
```bash
make clean
make
./simulator
```

### Build Configuration
- **Compiler**: GCC with C99 standard
- **Flags**: `-Wall -Wextra` for comprehensive warnings
- **Math Library**: `-lm` for mathematical functions in AI algorithms
- **Cross-platform**: Automatic Windows/Unix detection

---

## 2. Core Architecture: Graph-Based Network Modeling

The entire global internet infrastructure is represented as a **weighted, undirected graph** with sophisticated node and edge properties:

### Node Architecture (`graph/node.c`)
Nodes represent physical internet infrastructure with the following types:
- **SUBMARINE_CABLE**: Undersea fiber optic cable landing stations
- **DATACENTER**: Major cloud provider facilities (Google, AWS, Azure)
- **SATELLITE**: Communication satellites and ground stations

**Node Properties:**
```c
typedef struct {
    int id;                    // Unique identifier
    char name[64];            // Human-readable name
    char region[32];          // Geographic region
    int type;                 // Infrastructure type
    int capacity;             // Maximum traffic handling capacity
    int is_active;            // Status: 1=operational, 0=failed
    double risk_score;        // AI-calculated vulnerability score
} Node;
```

### Edge Architecture (`graph/edge.c`)
Edges model physical connections between infrastructure nodes:

**Edge Properties:**
```c
typedef struct {
    int id;                   // Unique identifier
    int from, to;            // Connected node IDs
    int bandwidth;           // Maximum data throughput (Mbps)
    int latency;             // Signal propagation delay (ms)
    double failure_probability; // Statistical failure likelihood
    int is_active;           // Connection status
} Edge;
```

### Topology Management (`graph/topology.c`)
Implements an **adjacency list** representation optimized for sparse global networks:
- Memory-efficient storage for large-scale topologies
- Fast neighbor lookup for pathfinding algorithms
- Dynamic edge activation/deactivation for failure simulation

---

## 3. Discrete Event Simulation Engine

The simulator uses a sophisticated **discrete event scheduler** (`simulation/scheduler.c`) that provides time-efficient simulation without busy-waiting:

### Event Queue Architecture
- **Priority Queue**: Events sorted by scheduled execution time
- **Event Types**: 
  - `EVT_FAILURE`: Infrastructure component failure
  - `EVT_CASCADE`: Overload-induced secondary failure
  - `EVT_REROUTE`: AI-driven traffic rerouting
  - `EVT_OVERLOAD`: Capacity threshold exceeded
  - `EVT_RECOVERY`: Repair completion

### Time Management
```c
typedef struct Event {
    int time;                 // Scheduled execution time
    int type;                 // Event type identifier
    int node_id;             // Target node (if applicable)
    int edge_id;             // Target edge (if applicable)
    char description[128];    // Human-readable event description
    struct Event* next;       // Linked list pointer
} Event;
```

### Simulation Flow
1. **Event Scheduling**: Future events added to priority queue
2. **Time Advancement**: Simulator jumps directly to next event time
3. **Event Processing**: Execute event logic and trigger cascades
4. **Metrics Update**: Recalculate network performance metrics
5. **Visualization**: Update ASCII world map display

---

## 4. Failure Simulation and Cascade Modeling

### Primary Failure Engine (`failures/failure_engine.c`)
Handles initial infrastructure failures with realistic propagation:

**Failure Injection Process:**
1. Target node `is_active` flag set to `0`
2. All connected edges automatically deactivated
3. Traffic load redistribution calculated
4. Cascade events scheduled based on overload thresholds

### Cascade Engine (`failures/cascade.c`)
Models realistic load redistribution and secondary failures:

**Cascade Algorithm:**
```c
// Simplified cascade logic
void process_cascade(Graph* g, int failed_node_id) {
    int redistributed_load = g->nodes[failed_node_id].current_load;
    
    // Find active neighbors
    for (int i = 0; i < g->node_count; i++) {
        if (is_neighbor(g, failed_node_id, i) && g->nodes[i].is_active) {
            // Distribute load proportionally
            int additional_load = redistributed_load / active_neighbor_count;
            g->nodes[i].current_load += additional_load;
            
            // Check for overload condition
            if (g->nodes[i].current_load > g->nodes[i].capacity * 0.95) {
                schedule_overload_event(i);
            }
        }
    }
}
```

**Cascade Characteristics:**
- **Load Redistribution**: Failed node traffic redistributed to neighbors
- **Capacity Thresholds**: Overload triggers at 95% capacity
- **Propagation Delay**: Realistic time delays for cascade propagation
- **Recovery Modeling**: Gradual capacity restoration after repairs

---

## 5. AI Algorithm Implementations

The simulator implements **8 distinct classical AI algorithms** for comprehensive network analysis:

### A. Pathfinding and Routing

#### Dijkstra's Algorithm (`ai/dijkstra.c`)
**Purpose**: Emergency rerouting and shortest path calculation
```c
// Core Dijkstra implementation
int* dijkstra_shortest_path(Graph* g, int source, int destination) {
    // Priority queue with distance tracking
    // Ignores failed nodes (is_active == 0)
    // Uses edge latency as path cost
    // Returns path array or NULL if isolated
}
```

#### A* Algorithm (`ai/astar.c`)
**Purpose**: Heuristic-guided pathfinding with geographic awareness
- **Heuristic**: Geographic distance estimation
- **Application**: Faster routing in large topologies
- **Optimization**: Reduces search space for continental routing

### B. Machine Learning Approaches

#### Hidden Markov Models (`ai/hmm.c`)
**Purpose**: Anomaly detection and failure pattern recognition
```c
typedef struct {
    double** transition_matrix;    // State transition probabilities
    double** emission_matrix;      // Observation probabilities
    double* initial_state;         // Starting state distribution
    int num_states;               // Hidden state count
    int num_observations;         // Observable event types
} HMM;
```

#### Q-Learning (`ai/qlearning.c`)
**Purpose**: Reinforcement learning for adaptive network management
- **State Space**: Network configuration and load distribution
- **Action Space**: Routing decisions and capacity allocation
- **Reward Function**: Network performance and resilience metrics
- **Training**: `--train-rl` command line option

#### Bayesian Networks (`ai/bayesian.c`)
**Purpose**: Probabilistic cascade failure prediction
- **Node Dependencies**: Conditional probability tables
- **Inference**: Belief propagation for failure probability
- **Applications**: Risk assessment and preventive maintenance

#### Genetic Algorithm (`ai/genetic.c`)
**Purpose**: Network hardening optimization
```c
typedef struct {
    int* chromosome;              // Network upgrade decisions
    double fitness;               // Network resilience score
    int length;                   // Number of upgrade options
} Individual;
```

### C. Strategic Analysis

#### Minimax Algorithm (`ai/minimax.c`)
**Purpose**: Adversarial threat assessment and worst-case analysis
- **Game Theory**: Attacker vs. Defender scenarios
- **Evaluation**: Network vulnerability under coordinated attacks
- **Applications**: Security planning and critical infrastructure protection

### D. Knowledge-Based Systems

#### Expert System (`ai/expert_system.c`)
**Purpose**: Rule-based vulnerability analysis
```c
// Example expert rules
if (node.type == DATACENTER && connected_edges == 1) {
    risk_score += 50;  // Single point of failure
}
if (betweenness_centrality > 0.8) {
    risk_score += 75;  // Critical bottleneck
}
```

#### Heuristics Engine (`ai/heuristics.c`)
**Purpose**: Betweenness centrality and critical node identification
```c
double calculate_betweenness_centrality(Graph* g, int node_id) {
    // For each pair of nodes (s,t):
    // Count shortest paths through node_id
    // Normalize by total shortest paths
    // Higher values indicate critical bottlenecks
}
```

---

## 6. Real-Time Metrics and Visualization

### Metrics Engine (`simulation/metrics.c`)
Continuously monitors network health with comprehensive statistics:

**Global Connectivity Metrics:**
```c
typedef struct {
    double connectivity_percentage;    // Percentage of node pairs connected
    double average_latency;           // Mean path latency across regions
    double network_efficiency;        // Ratio of actual vs. optimal performance
    int isolated_regions;             // Count of disconnected regions
    double total_capacity_utilization; // Network-wide load percentage
} NetworkMetrics;
```

**Regional Analysis:**
- **Inter-regional Connectivity**: Path availability between major regions
- **Latency Degradation**: Performance impact of rerouting
- **Capacity Utilization**: Load distribution and bottleneck identification
- **Isolation Detection**: Regions cut off from global internet

### ASCII Visualization (`output/visualizer.c`)
Real-time world map display with status indicators:

**Visual Elements:**
- **Regional Status**: `ACTIVE`, `DEGRADED XX%`, `OFFLINE`
- **Connection Lines**: ASCII art showing major cable routes
- **Failure Indicators**: Highlighted failed infrastructure
- **Performance Metrics**: Live connectivity and latency statistics

**Display Updates:**
- Automatic refresh after each simulation event
- Color coding for different failure states
- Dynamic route visualization showing active paths 

---

## 7. Data Management and Topology Loading

### Multi-Format Data Support (`data/loader.c`)
The simulator supports three data input methods:

**CSV Format (`topology.csv`):**
```csv
# Node definitions
node,0,SEA-ME-WE-4,SUBMARINE_CABLE,Asia,1000
node,1,Google-US-East,DATACENTER,North America,2000

# Edge definitions  
edge,0,0,1,10000,50,0.01
```

**JSON Format (`topology.json`):**
```json
{
  "nodes": [
    {
      "id": 0,
      "name": "SEA-ME-WE-4", 
      "type": "SUBMARINE_CABLE",
      "region": "Asia",
      "capacity": 1000
    }
  ],
  "edges": [
    {
      "id": 0,
      "from": 0,
      "to": 1,
      "bandwidth": 10000,
      "latency": 50,
      "failure_probability": 0.01
    }
  ]
}
```

**Hardcoded Topology (`data/real_topology.c`):**
- Built-in realistic global internet infrastructure
- Major submarine cables, data centers, and satellite links
- Geographically accurate regional distribution

### Data Loading Priority:
1. `topology.csv` (if present)
2. `topology.json` (if CSV not found)
3. Hardcoded topology (fallback)

---

## 8. Report Generation and Analysis

### AI Analysis Reports (`output/report.c`)
Generates comprehensive network analysis documents:

**Vulnerability Assessment Report:**
- Critical node identification with risk scores
- Betweenness centrality analysis
- Single points of failure detection
- Cascade failure probability assessment

**Resilience Recommendations:**
- AI-generated infrastructure improvements
- Cost-benefit analysis for network hardening
- Priority ranking for upgrade projects
- Geographic distribution optimization

**Prediction Reports:**
- Bayesian network failure probability forecasts
- HMM-based anomaly detection results
- Q-learning optimal routing recommendations
- Genetic algorithm optimization results

---

## 9. Advanced Simulation Scenarios

### Solar Storm Multi-Failure Scenario
Realistic timeline-based cascade simulation:

```c
// Example event sequence
add_event(s, 0, EVT_FAILURE, "SEA-ME-WE-4", "Earthquake severs cable");
add_event(s, 3, EVT_REROUTE, "APCN-2", "Traffic rerouting begins");
add_event(s, 8, EVT_OVERLOAD, "APCN-2", "Reaches 95% capacity");
add_event(s, 15, EVT_CASCADE, "APCN-2", "Fails under load");
add_event(s, 75, EVT_RECOVERY, "SEA-ME-WE-4", "Repair ship completes work");
```

### Genetic Algorithm Optimization
Budget-constrained network hardening:

**Optimization Parameters:**
- **Budget**: Total available upgrade funding
- **Generations**: Evolution iterations
- **Population Size**: Candidate solutions per generation
- **Fitness Function**: Network resilience improvement

**Upgrade Options:**
- Bandwidth increases for existing links
- New redundant connections
- Capacity upgrades for critical nodes
- Geographic diversity improvements

---

## 10. Performance Considerations

### Memory Management
- **Dynamic Allocation**: Graphs resize based on topology size
- **Efficient Structures**: Adjacency lists for sparse networks
- **Event Queue**: Linked list with O(n) insertion, O(1) removal
- **Cleanup**: Proper memory deallocation on exit

### Computational Complexity
- **Dijkstra**: O((V + E) log V) with priority queue
- **Betweenness Centrality**: O(VE) for unweighted graphs
- **Genetic Algorithm**: O(generations × population × fitness_evaluation)
- **Event Processing**: O(1) per event with sorted queue

### Scalability Limits
- **Maximum Nodes**: ~1000 nodes (configurable in graph initialization)
- **Maximum Edges**: ~5000 edges (sparse network assumption)
- **Event Queue**: Limited by available memory
- **AI Algorithms**: Performance degrades with network size

---

## 11. Extension and Customization

### Adding New AI Algorithms
1. Create new files in `ai/` directory
2. Implement algorithm-specific functions
3. Add to Makefile source list
4. Include header in `main.c`
5. Add menu option for algorithm execution

### Custom Topology Data
1. Prepare CSV or JSON file with required format
2. Place in project root directory
3. Use "Load New Dataset File" menu option
4. Verify topology with "View Current Network Topology"

### New Event Types
1. Define event constant in `simulation/scheduler.h`
2. Implement event handler in `simulation/scheduler.c`
3. Add event scheduling logic in relevant modules
4. Update visualization for new event types

### Performance Optimization
- **Compiler Flags**: Add `-O3` for production builds
- **Data Structures**: Consider hash tables for large topologies
- **Parallel Processing**: OpenMP for AI algorithm parallelization
- **Memory Pools**: Pre-allocated memory for frequent allocations

This architecture guide provides the foundation for understanding, extending, and optimizing the Planet-Scale Internet Outage Simulator. The modular design enables easy addition of new AI algorithms, visualization methods, and simulation scenarios.