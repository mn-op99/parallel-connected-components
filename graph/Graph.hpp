#pragma once

#include "EdgeList.hpp"
#include <vector>

class Graph {
public:
    int num_nodes;
    int num_edges;

    std::vector<std::vector<int>> adj;

    explicit Graph(int n);

    // Build graph from edge list
    void build_from_edges(const EdgeList& edges);

    // Get neighbors of a node
    const std::vector<int>& neighbors(int u) const;
};