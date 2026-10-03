#pragma once
#include <vector>
#include <string>
#include <stdexcept>

using namespace std;

// EXPERIMENT CONFIGURATION
struct ExperimentConfig {

    // COMMON SETTINGS
    int runs_per_config = 3;              // repetitions per configuration
    std::string dataset = "synthetic";    // dataset type
    bool enable_logging = true;

    // EXP1: SIZE SCALING
    std::vector<int> node_scales = {
        2000,
        5000,
        10000
    };

    // EXP2: GRAPH TYPES
    std::vector<std::string> graph_types = {
        "mixed_dense",
        "mixed_sparse",
        "balanced"
    };

    int fixed_size_for_type_exp = 10000;

    // EXP3: DENSITY VARIATION
    std::vector<double> densities = {
        0.02,
        0.05,
        0.1
    };

    int fixed_size_for_density_exp = 10000;

    // EXP4: COMPONENT DISTRIBUTION
    std::vector<std::string> component_modes = {
        "few_large",
        "many_small",
        "balanced"
    };

    int fixed_size_for_component_exp = 10000;

    // EXP5: THREAD SCALING
    std::vector<int> thread_counts = {
        1, 2, 4, 8, 12
    };

    int fixed_size_for_thread_exp = 10000;

    // RANDOM SEED BASE (REPRODUCIBILITY)
    int base_seed = 42;

    // VALIDATION
    void validate() const {

        if (runs_per_config <= 0) {
            throw std::runtime_error("runs_per_config must be > 0");
        }

        if (node_scales.empty()) {
            throw std::runtime_error("node_scales cannot be empty");
        }

        if (graph_types.empty()) {
            throw std::runtime_error("graph_types cannot be empty");
        }

        if (densities.empty()) {
            throw std::runtime_error("densities cannot be empty");
        }

        if (component_modes.empty()) {
            throw std::runtime_error("component_modes cannot be empty");
        }

        if (thread_counts.empty()) {
            throw std::runtime_error("thread_counts cannot be empty");
        }
    }
};