#pragma once

#include "../../graph/EdgeList.hpp"
#include <vector>

// RESULT STRUCT
struct HybridResult {
    std::vector<int> component;
    int num_components;

    // optional stats (not used now)
    int bfs_nodes = 0;
    int sv_nodes = 0;
};

// HYBRID BFS + SV
class HybridCC {
public:
    static HybridResult run(
        int n,
        const EdgeList& edges,
        const std::vector<std::vector<int>>& adj
    );
};