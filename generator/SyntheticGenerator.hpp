#pragma once

#include "../graph/EdgeList.hpp"

// GRAPH CONFIG
struct GraphConfig {

    // COMPONENT COUNTS
    int num_large_components  = 3;
    int num_medium_components = 10;
    int num_sparse_components = 20;

    // COMPONENT SIZES
    int large_size  = 2000;
    int medium_size = 300;
    int sparse_size = 100;

    // DENSITY CONTROLS
    double dense_prob = 0.05;             // controls density of large components
    double sparse_extra_edge_prob = 0.3;  // extra edges in sparse graphs

    // RANDOM SEED (reproducibility)
    unsigned int seed = 42;
};

// SYNTHETIC GENERATOR
class SyntheticGenerator {
public:

    // Generates graph with:
    // - dense components (small diameter)
    // - medium (scale-free-like)
    // - sparse (tree-like)
    static EdgeList generate(const GraphConfig& config);
};