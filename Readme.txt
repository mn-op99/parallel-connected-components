Author: AI25MTECH14008
Project: Parallel Connected Components Comupatation

---

1. DIRECTORY STRUCTURE

---

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
├── logs/                         # Generated after execution
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
└── readme.txt

---

2. PROJECT DESCRIPTION

---

This project implements and compares two parallel algorithms for
computing Connected Components (CC):

* Hybrid Algorithm (BFS + Shiloach-Vishkin)
* Parallel Disjoint Set Union (DSU)

The evaluation is done using synthetic graphs under different conditions.

---

3. COMPILATION INSTRUCTIONS

---

Open terminal in project directory and run:

clang++ -std=c++20 -O2 -pthread 
experiments/Src_Prjt-AI25MTECH14008-compare.cpp 
graph/Graph.cpp 
generator/SyntheticGenerator.cpp 
algorithms/bfs_sv/Src_Prjt-AI25MTECH14008-Hybrid.cpp 
algorithms/dsu/Src_Prjt-AI25MTECH14008-DSU.cpp 
-o compare

---

4. RUNNING THE PROGRAM

---

Run all experiments:

./compare

This will automatically execute all 5 experiments and generate logs.

---

5. EXPERIMENTS PERFORMED

---

1. Runtime vs Graph Size
2. Runtime vs Graph Type
3. Runtime vs Density
4. Runtime vs Component Distribution
5. Speedup vs Threads

Each configuration is executed multiple times and averaged.

---

6. LOG FILES

---

Logs are stored in:

logs/<experiment_name>/runtime.json

Each entry contains:

* nodes
* edges
* graph type
* parameter (size/density/threads)
* run number
* hybrid runtime
* dsu runtime

---

7. GENERATING GRAPHS

---

Navigate to visualization/ and run:

python stats.py

This will generate all plots in:

visualization/results/

---

8. OUTPUT FILES

---

* logs/            → Raw experiment logs
* results/         → Generated plots (.png)
* compare.exe      → Executable

---

9. PARALLEL IMPLEMENTATION DETAILS

---

Parallelism is implemented using a custom parallel_for:

* Work is divided into chunks
* Each thread processes a chunk independently
* Threads are created using std::thread
* Synchronization is done using join

Hybrid Algorithm:

* Parallel BFS (frontier processing)
* Parallel SV (hooking + compression)

DSU:

* Parallel edge processing
* Atomic CAS for union operations
* Path compression for efficiency

---

10. REQUIREMENTS

---

* C++20 compatible compiler (clang++)
* Python 3

Python libraries:

* matplotlib

Install using:
pip install matplotlib

---

11. NOTES

---

* Graphs are generated internally (no dataset required)
* Logs and directories are created automatically
* Results are reproducible using fixed seeds
* Run experiments before generating plots

---

END OF FILE
