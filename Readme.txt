# Parallel Connected Components Computation

**Author:** AI25MTECH14008  
**Course:** Parallel & Concurrent Programming  
**Project:** Parallel Connected Components Computation

---

## 1. Project Overview

This project implements and compares two parallel algorithms for computing **Connected Components (CC)** in an undirected graph:

- **Hybrid Algorithm (BFS + Shiloach-Vishkin)**
- **Parallel Disjoint Set Union (DSU)**

The algorithms are evaluated using synthetic graphs under different graph sizes, graph structures, edge densities, component distributions, and thread configurations.

---

## 2. Directory Structure

```text
PCP_Project/
│
├── algorithms/
│   ├── bfs_sv/
│   │   ├── Src_Prjt-AI25MTECH14008-Hybrid.cpp
│   │   └── Src_Prjt-AI25MTECH14008-Hybrid.hpp
│   │
│   └── dsu/
│       ├── Src_Prjt-AI25MTECH14008-DSU.cpp
│       └── Src_Prjt-AI25MTECH14008-DSU.hpp
│
├── experiments/
│   ├── config.hpp
│   └── Src_Prjt-AI25MTECH14008-compare.cpp
│
├── generator/
│   ├── SyntheticGenerator.cpp
│   └── SyntheticGenerator.hpp
│
├── graph/
│   ├── Graph.cpp
│   ├── Graph.hpp
│   └── EdgeList.hpp
│
├── parallel/
│   └── ParallelFor.hpp
│
├── utils/
│   └── Logger.hpp
│
├── logs/
│   ├── exp1_size/
│   ├── exp2_type/
│   ├── exp3_density/
│   ├── exp4_components/
│   └── exp5_threads/
│
├── visualization/
│   ├── stats.py
│   └── results/
│       ├── exp1_size_balanced.png
│       ├── exp1_size_mixed_dense.png
│       ├── exp1_size_mixed_sparse.png
│       ├── exp2_runtime_vs_type.png
│       ├── exp3_runtime_vs_density.png
│       ├── exp4_runtime_vs_components.png
│       └── exp5_speedup_vs_threads.png
│
├── compare.exe
└── README.md
```

---

## 3. Algorithms

### Hybrid BFS + Shiloach-Vishkin

The Hybrid algorithm combines:

- **Parallel BFS** for processing large connected components.
- **Shiloach-Vishkin (SV)** for processing the remaining graph.

The SV implementation uses:

- Parallel hooking
- Pointer jumping
- Path compression

### Parallel DSU

The Parallel Disjoint Set Union implementation uses:

- Parallel edge processing
- Atomic **Compare-and-Swap (CAS)** operations
- Path compression
- Concurrent union operations

---

## 4. Compilation

Open a terminal in the project directory and run:

```bash
clang++ -std=c++20 -O2 -pthread \
experiments/Src_Prjt-AI25MTECH14008-compare.cpp \
graph/Graph.cpp \
generator/SyntheticGenerator.cpp \
algorithms/bfs_sv/Src_Prjt-AI25MTECH14008-Hybrid.cpp \
algorithms/dsu/Src_Prjt-AI25MTECH14008-DSU.cpp \
-o compare
```

This generates the executable:

```text
compare
```

---

## 5. Running the Program

Run all experiments using:

```bash
./compare
```

The program automatically executes all five experiments and generates the corresponding log files.

---

## 6. Experiments Performed

The following experiments are performed:

### Experiment 1 — Runtime vs Graph Size

Measures the runtime of both algorithms as the number of graph nodes increases.

### Experiment 2 — Runtime vs Graph Type

Compares performance across different graph structures.

### Experiment 3 — Runtime vs Density

Studies the effect of increasing edge density on algorithm performance.

### Experiment 4 — Runtime vs Component Distribution

Evaluates performance for different connected-component distributions.

### Experiment 5 — Speedup vs Threads

Measures parallel scalability by varying the number of threads.

Each configuration is executed multiple times and the results are averaged.

---

## 7. Log Files

Experiment logs are stored under:

```text
logs/<experiment_name>/runtime.json
```

Each log entry contains:

- Number of nodes
- Number of edges
- Graph type
- Experiment parameter
- Run number
- Hybrid runtime
- DSU runtime

The logs are organized as:

```text
logs/
├── exp1_size/
├── exp2_type/
├── exp3_density/
├── exp4_components/
└── exp5_threads/
```

---

## 8. Generating Graphs

Navigate to the visualization directory:

```bash
cd visualization
```

Run:

```bash
python stats.py
```

This generates the plots inside:

```text
visualization/results/
```

The generated results include:

- Runtime vs Graph Size
- Runtime vs Graph Type
- Runtime vs Density
- Runtime vs Component Distribution
- Speedup vs Number of Threads

---

## 9. Parallel Implementation

Parallelism is implemented using a custom `parallel_for` utility.

### Work Distribution

- Work is divided into chunks.
- Each thread processes its assigned chunk independently.
- Threads are created using `std::thread`.
- Synchronization is performed using `join()`.

### Hybrid Algorithm

The Hybrid implementation uses:

- Parallel BFS frontier processing
- Parallel Shiloach-Vishkin hooking
- Parallel pointer compression

### Parallel DSU

The DSU implementation uses:

- Parallel edge processing
- Atomic CAS for concurrent union operations
- Path compression for efficient component finding

---

## 10. Requirements

### C++

- C++20 compatible compiler
- `clang++`
- POSIX thread support

### Python

- Python 3
- Matplotlib

Install Matplotlib using:

```bash
pip install matplotlib
```

---

## 11. Output

The project generates:

```text
logs/
```

Raw experiment logs.

```text
visualization/results/
```

Generated performance plots.

```text
compare
```

Compiled executable.

---

## 12. Notes

- Graphs are generated internally; no external dataset is required.
- Log directories are created automatically during execution.
- Results are reproducible using fixed random seeds.
- Run the experiments before generating visualization plots.
- The project is designed to compare the performance and scalability of the two parallel connected-components algorithms.

---

## 13. Project Objective

The primary objective of this project is to implement and experimentally compare two parallel algorithms for connected-components computation and analyze their performance under different graph and parallel execution conditions.

---

## Author

**AI25MTECH14008**

**Parallel & Concurrent Programming**  
**IIT Hyderabad**
