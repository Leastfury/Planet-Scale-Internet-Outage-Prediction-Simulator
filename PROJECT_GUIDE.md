# Planet-Scale Internet Outage Simulator: Deep-Dive Architecture Guide

Welcome to the internal workings of the Planet-Scale Internet Outage Simulator! This guide is meant for developers and engineers who want to understand *exactly* how the project ticks, how the AI works, and how to compile it natively on Windows.

---

## 1. How to Compile and Run on Windows
You may have noticed that `make run` fails in PowerShell. This is because Windows does not come with GNU `make` out of the box. 

Instead of `make`, you can compile the code directly using the GCC compiler. 

### Compilation Command:
Open your terminal in the project folder and paste:
```powershell
gcc -std=c99 -Wall -o simulator.exe main.c graph/topology.c graph/node.c graph/edge.c failures/failure_engine.c failures/cascade.c ai/dijkstra.c ai/expert_system.c ai/heuristics.c simulation/scheduler.c simulation/metrics.c output/visualizer.c output/report.c data/loader.c data/real_topology.c
```
*Note: This command tells the GCC compiler to bundle all the `.c` modules into a single `simulator.exe` executable using the C99 standard.*

### Running the Simulator:
After compiling, simply run the executable:
```powershell
.\simulator.exe
```

---

## 2. Core Architecture: The Graph
The entire internet is modeled as an **undirected, weighted Graph**.

- **Nodes (`graph/node.c`)**: These represent the physical hardware—Submarine Cable stations, Data Centers, and Satellites. Each node has a `capacity` (how much traffic it can handle before failing) and an `is_active` state (1 = alive, 0 = broken).
- **Edges (`graph/edge.c`)**: These are the physical fiber-optic cables or satellite beams connecting nodes. Edges have `bandwidth`, `latency` (used for calculating the fastest routes), and a `failure_probability`.
- **Topology (`graph/topology.c`)**: This orchestrates the nodes and edges into an **Adjacency List**, a memory-efficient way to represent sparse networks.

---

## 3. The Time Engine (Discrete Event Scheduler)
The simulator doesn't run in a fast ```while(true)``` loop that freezes your CPU. Instead, it uses a **Discrete Event Scheduler (`simulation/scheduler.c`)**.

1. The scheduler maintains an **Event Queue** (a sorted Linked List).
2. Events are scheduled to occur at specific times in the future:
   - `EVT_FAILURE` (Something broke)
   - `EVT_CASCADE` (Overload destroyed a node)
   - `EVT_REROUTE` (The AI found a new path)
   - `EVT_RECOVERY` (The repair crews fixed the break)
3. The engine skips "dead time" and instantly jumps to the exact minute the next event is supposed to happen.

---

## 4. The Failures & Cascade Engine
When a node is manually broken (or broken by the Solar Storm scenario), the Failure Engine (`failures/failure_engine.c`) triggers:

1. The targeted Node's `is_active` flag is set to `0`.
2. All connecting Edges attached to that node are instantly deactivated.
3. **The Cascade (`failures/cascade.c`)**: Traffic that *was* flowing through the dead node must go somewhere else. The capacity load is mathematically split and shoved onto neighboring nodes. 
4. If a neighbor receives too much traffic and exceeds its physical bandwidth limits, the cascade engine pushes an `EVT_OVERLOAD` into the future. If the overload isn't fixed, it pushes an `EVT_CASCADE` (the neighbor literally burns out and fails).

---

## 5. The Classical AI Engines
This project uses **three** distinct Classical AI algorithms (no Machine Learning, purely mathematical logic) to analyze the network:

### A. Dijkstra's Algorithm (`ai/dijkstra.c`)
Used for Rerouting. When a cable breaks, Dijkstra calculates the new "Shortest Path" between regions. 
- The AI treats the `latency` of the cables as the "cost".
- It dynamically ignores any nodes or edges where `is_active == 0`.
- If a route is found, traffic shifts successfully. If no route is found, the region is marked **ISOLATED**.

### B. Heuristics & Betweenness Centrality (`ai/heuristics.c`)
Used for Vulnerability Scanning. "Betweenness Centrality" identifies "Bridge Nodes".
- The AI plays a game: "If I want to get from Node A to Node B, do I *have* to pass through Node C?"
- If Node C is part of almost every shortest path in the world (like a major Submarine Cable landing station), it gets a massive **Risk Score** because it's a critical bottleneck. Let this node fail, and half the world goes offline.

### C. Expert Rule System (`ai/expert_system.c`)
Used for generating Human-Readable Advice. This is a **Forward-Chaining Rule Engine**.
- It evaluates strict `IF-THEN` conditions on every node.
- Example Rule: `IF (node.type == DATACENTER) AND (connected_edges == 1) THEN Risk += 50 (Single Point of Failure)`.
- The outputs of these rules are compiled into the `Generate Resilience Recommendations` menu option to tell network engineers exactly where they need to build new cables to save the internet.

---

## 6. Real-Time Tracking Matrices
The `simulation/metrics.c` file runs a global scan every time the clock ticks. It verifies:
- Are regions fully connected to each other?
- Exactly what percentage of paths are intact?
- If degraded, what is the new average latency globally now that packets have to take the long way around?

This data feeds directly into the ASCII map (`output/visualizer.c`) to flash `DEGRADED 66%` or `OFFLINE <30%` live on the dashboard! 
