#include "Graph.hpp"

Graph::Graph(int n) {
    num_nodes = n;
    num_edges = 0;
    adj.resize(n);
}

void Graph::build_from_edges(const EdgeList& edges) {
    num_edges = edges.size();

    // CLEAR EXISTING GRAPH
    for (auto& vec : adj) {
        vec.clear();
    }

    // BUILD ADJACENCY LIST
    for (const auto& [u, v] : edges) {

        // safety check
        if (u < 0 || v < 0 || u >= num_nodes || v >= num_nodes)
            continue;

        // undirected graph
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
}

const std::vector<int>& Graph::neighbors(int u) const {
    return adj[u];
}