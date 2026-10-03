#include "../generator/SyntheticGenerator.hpp"
#include "../graph/Graph.hpp"
#include "../algorithms/bfs_sv/Src_Prjt-AI25MTECH14008-Hybrid.hpp"
#include "../algorithms/dsu/Src_Prjt-AI25MTECH14008-DSU.hpp"
#include "../utils/Logger.hpp"
#include "config.hpp"

#include <iostream>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace std::chrono;

// GLOBAL THREAD CONTROL (for EXP5)
extern int GLOBAL_NUM_THREADS;

// GRAPH CONFIG HELPER
GraphConfig get_graph_config(string type, int scale) {
    GraphConfig cfg;

    if (type == "mixed_dense") {
        cfg.num_large_components = 4;
        cfg.large_size = scale;

        cfg.num_medium_components = 5;
        cfg.medium_size = scale / 5;

        cfg.num_sparse_components = 5;
        cfg.sparse_size = scale / 10;
    }
    else if (type == "mixed_sparse") {
        cfg.num_large_components = 2;
        cfg.large_size = scale;

        cfg.num_medium_components = 5;
        cfg.medium_size = scale / 4;

        cfg.num_sparse_components = 20;
        cfg.sparse_size = scale / 20;
    }
    else { // balanced
        cfg.num_large_components = 3;
        cfg.large_size = scale;

        cfg.num_medium_components = 10;
        cfg.medium_size = scale / 6;

        cfg.num_sparse_components = 10;
        cfg.sparse_size = scale / 12;
    }

    return cfg;
}

// RUN BOTH ALGORITHMS
pair<long long, long long> run_both(int n, const EdgeList& edges, Graph& G) {

    auto t1s = high_resolution_clock::now();
    HybridCC::run(n, edges, G.adj);
    auto t1e = high_resolution_clock::now();

    auto t2s = high_resolution_clock::now();
    ParallelDSU::run(n, edges);
    auto t2e = high_resolution_clock::now();

    long long t_hybrid = duration_cast<milliseconds>(t1e - t1s).count();
    long long t_dsu    = duration_cast<milliseconds>(t2e - t2s).count();

    return {t_hybrid, t_dsu};
}

// MAIN
int main() {

    ExperimentConfig exp;
    exp.validate();

    cout << "\n========================================\n";
    cout << "Running ALL Experiments\n";
    cout << "========================================\n";

    // EXP 1: Runtime vs Size
{
        string EXP = "exp1_size";
        cout << "\n===== EXP1: Runtime vs Size =====\n";

        for (auto scale : exp.node_scales) {
            for (auto type : exp.graph_types) {
                for (int run = 0; run < exp.runs_per_config; run++) {

                    cout << "[EXP1] size=" << scale
                         << " type=" << type
                         << " run=" << run << endl;

                    auto cfg = get_graph_config(type, scale);
                    cfg.seed = exp.base_seed + run;

                    auto edges = SyntheticGenerator::generate(cfg);

                    int n = 0;
                    for (auto& [u, v] : edges)
                        n = max(n, max(u, v));
                    n++;

                    Graph G(n);
                    G.build_from_edges(edges);

                    auto [t1, t2] = run_both(n, edges, G);

                    Logger::log_runtime(EXP, n, edges.size(), type, scale, run, t1, t2);
                }
            }
        }
    }

    // EXP 2: Graph Type
    {
        string EXP = "exp2_type";
        cout << "\n===== EXP2: Graph Type =====\n";

        int fixed_size = exp.fixed_size_for_type_exp;

        for (auto type : exp.graph_types) {
            for (int run = 0; run < exp.runs_per_config; run++) {

                cout << "[EXP2] type=" << type
                     << " run=" << run << endl;

                auto cfg = get_graph_config(type, fixed_size);
                cfg.seed = exp.base_seed + run;

                auto edges = SyntheticGenerator::generate(cfg);

                int n = 0;
                for (auto& [u, v] : edges)
                    n = max(n, max(u, v));
                n++;

                Graph G(n);
                G.build_from_edges(edges);

                auto [t1, t2] = run_both(n, edges, G);

                Logger::log_runtime(EXP, n, edges.size(), type, 0, run, t1, t2);
            }
        }
    }

    // EXP 3: Density
    {
        string EXP = "exp3_density";
        cout << "\n===== EXP3: Density =====\n";

        int fixed_size = exp.fixed_size_for_density_exp;

        for (auto d : exp.densities) {
            for (int run = 0; run < exp.runs_per_config; run++) {

                cout << "[EXP3] density=" << d
                     << " run=" << run << endl;

                auto cfg = get_graph_config("mixed_dense", fixed_size);
                cfg.dense_prob = d;
                cfg.seed = exp.base_seed + run;

                auto edges = SyntheticGenerator::generate(cfg);

                int n = 0;
                for (auto& [u, v] : edges)
                    n = max(n, max(u, v));
                n++;

                Graph G(n);
                G.build_from_edges(edges);

                auto [t1, t2] = run_both(n, edges, G);

                Logger::log_runtime(EXP, n, edges.size(), "density", d, run, t1, t2);
            }
        }
    }

    // EXP 4: Component Distribution
    {
        string EXP = "exp4_components";
        cout << "\n===== EXP4: Components =====\n";

        int fixed_size = exp.fixed_size_for_component_exp;

        for (auto mode : exp.component_modes) {
            for (int run = 0; run < exp.runs_per_config; run++) {

                cout << "[EXP4] mode=" << mode
                     << " run=" << run << endl;

                GraphConfig cfg;

                if (mode == "few_large") {
                    cfg.num_large_components = 2;
                    cfg.large_size = fixed_size;
                } 
                else if (mode == "many_small") {
                    cfg.num_large_components = 20;
                    cfg.large_size = fixed_size / 10;
                } 
                else {
                    cfg = get_graph_config("balanced", fixed_size);
                }

                cfg.seed = exp.base_seed + run;

                auto edges = SyntheticGenerator::generate(cfg);

                int n = 0;
                for (auto& [u, v] : edges)
                    n = max(n, max(u, v));
                n++;

                Graph G(n);
                G.build_from_edges(edges);

                auto [t1, t2] = run_both(n, edges, G);

                Logger::log_runtime(EXP, n, edges.size(), mode, 0, run, t1, t2);
            }
        }
    }

    // EXP 5: Thread Scaling
    {
        string EXP = "exp5_threads";
        cout << "\n===== EXP5: Threads =====\n";

        int fixed_size = exp.fixed_size_for_thread_exp;

        for (auto threads : exp.thread_counts) {

            GLOBAL_NUM_THREADS = threads;

            for (int run = 0; run < exp.runs_per_config; run++) {

                cout << "[EXP5] threads=" << threads
                     << " run=" << run << endl;

                auto cfg = get_graph_config("mixed_dense", fixed_size);
                cfg.seed = exp.base_seed + run;

                auto edges = SyntheticGenerator::generate(cfg);

                int n = 0;
                for (auto& [u, v] : edges)
                    n = max(n, max(u, v));
                n++;

                Graph G(n);
                G.build_from_edges(edges);

                auto [t1, t2] = run_both(n, edges, G);

                Logger::log_runtime(EXP, n, edges.size(), "mixed_dense", threads, run, t1, t2);
            }
        }
    }

    cout << "\n========================================\n";
    cout << "All Experiments Completed\n";
    cout << "========================================\n";

    return 0;
}