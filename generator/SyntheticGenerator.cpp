#include "SyntheticGenerator.hpp"
#include <random>

using namespace std;

// Dense component (controlled)
static EdgeList generate_dense(int n, int offset, double p, mt19937& rng) {
    EdgeList edges;

    uniform_real_distribution<> dist(0.0, 1.0);

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (dist(rng) < p) {
                edges.emplace_back(i + offset, j + offset);
            }
        }
    }

    return edges;
}

// Medium (preferential attachment)
static EdgeList generate_medium(int n, int offset, mt19937& rng) {
    EdgeList edges;

    vector<int> degree(n, 1);

    for (int i = 1; i < n; i++) {
        int total_degree = 0;
        for (int j = 0; j < i; j++) total_degree += degree[j];

        int r = rng() % total_degree;
        int sum = 0;

        int target = 0;
        for (int j = 0; j < i; j++) {
            sum += degree[j];
            if (sum > r) {
                target = j;
                break;
            }
        }

        edges.emplace_back(i + offset, target + offset);
        degree[i]++;
        degree[target]++;
    }

    return edges;
}

// Sparse (branching tree)
static EdgeList generate_sparse(
    int n,
    int offset,
    double extra_prob,
    mt19937& rng
) {
    EdgeList edges;

    uniform_real_distribution<> prob_dist(0.0, 1.0);

    for (int i = 1; i < n; i++) {
        int parent = rng() % i;
        edges.emplace_back(i + offset, parent + offset);

        // optional extra edge
        if (i > 5 && prob_dist(rng) < extra_prob) {
            int extra = rng() % i;
            edges.emplace_back(i + offset, extra + offset);
        }
    }

    return edges;
}

// MAIN GENERATOR
EdgeList SyntheticGenerator::generate(const GraphConfig& config) {
    EdgeList edges;

    mt19937 rng(config.seed);

    int offset = 0;

    // LARGE dense components
    for (int i = 0; i < config.num_large_components; i++) {
        auto comp = generate_dense(
            config.large_size,
            offset,
            config.dense_prob,
            rng
        );
        edges.insert(edges.end(), comp.begin(), comp.end());
        offset += config.large_size;
    }

    // MEDIUM components
    for (int i = 0; i < config.num_medium_components; i++) {
        auto comp = generate_medium(
            config.medium_size,
            offset,
            rng
        );
        edges.insert(edges.end(), comp.begin(), comp.end());
        offset += config.medium_size;
    }

    // SPARSE components
    for (int i = 0; i < config.num_sparse_components; i++) {
        auto comp = generate_sparse(
            config.sparse_size,
            offset,
            config.sparse_extra_edge_prob,
            rng
        );
        edges.insert(edges.end(), comp.begin(), comp.end());
        offset += config.sparse_size;
    }

    return edges;
}