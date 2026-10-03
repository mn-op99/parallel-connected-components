#pragma once

#include "../../graph/EdgeList.hpp"
#include <vector>

// RESULT STRUCT
struct DSUResult {
    std::vector<int> component;   // component id per node
    int num_components = 0;       // total number of components
};

// PARALLEL DSU
class ParallelDSU {
public:

    // Runs parallel Union-Find (DSU)
    // Input:
    //   n      → number of nodes
    //   edges  → edge list
    //
    // Output:
    //   component[i] = component id of node i
    //   num_components = total connected components
    static DSUResult run(int n, const EdgeList& edges);
};