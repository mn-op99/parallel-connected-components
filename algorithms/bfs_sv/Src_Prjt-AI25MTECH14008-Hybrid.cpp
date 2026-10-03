#include "Src_Prjt-AI25MTECH14008-Hybrid.hpp"
#include "../../parallel/ParallelFor.hpp"

#include <vector>
#include <cmath>
#include <algorithm>
#include <atomic>

using namespace std;

static vector<int> compute_degrees(int n, const EdgeList& edges) {
    vector<atomic<int>> deg(n);
    for (int i = 0; i < n; i++) deg[i].store(0, memory_order_relaxed);

    parallel_for(0, (int)edges.size(), [&](int i, int) {
        auto& [u, v] = edges[i];
        deg[u].fetch_add(1, memory_order_relaxed);
        deg[v].fetch_add(1, memory_order_relaxed);
    });

    vector<int> result(n);
    for (int i = 0; i < n; i++) result[i] = deg[i].load();
    return result;
}

static bool is_scale_free(const vector<int>& degree, double tau = 0.1) {
    vector<int> pos;
    pos.reserve(degree.size());
    for (int d : degree) if (d > 0) pos.push_back(d);
    if (pos.empty()) return false;

    int d_min = *min_element(pos.begin(), pos.end());
    if (d_min < 1) d_min = 1;

    // MLE for discrete power-law exponent (Clauset et al. eq. 3.6)
    double log_sum = 0.0;
    int cnt = 0;
    for (int d : pos) {
        if (d >= d_min) { log_sum += log((double)d / (d_min - 0.5)); cnt++; }
    }
    if (log_sum <= 0.0 || cnt == 0) return false;
    double alpha = 1.0 + cnt / log_sum;

    // KS statistic on empirical vs theoretical CCDF
    vector<int> sd = pos;
    sort(sd.begin(), sd.end());
    int sz = (int)sd.size();

    double ks = 0.0;
    for (int i = 0; i < sz; i++) {
        double d = sd[i];
        if (d < d_min) continue;
        double emp  = (double)(sz - i) / sz;
        double theo = pow(d / (double)d_min, 1.0 - alpha);
        theo = max(0.0, min(1.0, theo));
        ks = max(ks, fabs(emp - theo));
    }
    return ks < tau;
}

static int parallel_bfs_single(
    int seed,
    const vector<vector<int>>& adj,
    vector<char>& visited,
    vector<int>& component,
    int comp_id
) {
    extern int GLOBAL_NUM_THREADS;
    int threads = (GLOBAL_NUM_THREADS > 0) ? GLOBAL_NUM_THREADS : 4;

    visited[seed] = 1;
    component[seed] = comp_id;
    vector<int> frontier = {seed};
    int total = 1;

    while (!frontier.empty()) {
        vector<vector<int>> local_next(threads);

        parallel_for(0, (int)frontier.size(), [&](int i, int tid) {
            int u = frontier[i];
            for (int v : adj[u]) {
                if (!visited[v]) {           // benign read-write race
                    visited[v] = 1;          // benign write race
                    component[v] = comp_id;  // benign write race
                    local_next[tid].push_back(v);
                }
            }
        });

        vector<int> next;
        for (auto& buf : local_next)
            next.insert(next.end(), buf.begin(), buf.end());

        total += (int)next.size();
        frontier.swap(next);
    }
    return total;
}

static void parallel_sv(
    int n,
    const EdgeList& edges,
    vector<int>& parent,
    const vector<int>& component
) {
    parallel_for(0, n, [&](int i, int) {
        parent[i] = (component[i] == -1) ? i : -1;
    });

    // O(log n) iterations sufficient by pointer-doubling guarantee
    int MAX_ITER = max(2 * (int)ceil(log2((double)n + 2)) + 5, 30);

    for (int iter = 0; iter < MAX_ITER; iter++) {
        bool changed = false;

        // === HOOK PHASE ===
        // For each edge (u, v) where both endpoints are SV-managed:
        //   find their roots pu, pv; if different, hook larger to smaller.
        //   (Paper: "join partition p to pmin = min Ci(p)")
        parallel_for(0, (int)edges.size(), [&](int i, int) {
            auto [u, v] = edges[i];
            if (component[u] != -1 || component[v] != -1) return;
            if (parent[u] == -1 || parent[v] == -1) return;

            // Walk to root (inline find; compression done separately)
            int pu = u;
            while (parent[pu] != pu) pu = parent[pu];
            int pv = v;
            while (parent[pv] != pv) pv = parent[pv];

            if (pu == pv) return;

            // Hook: larger root points to smaller root
            // Benign write race: another thread may also write parent[pv]
            // or parent[pu]; any write is a valid SV hook operation.
            if (pu < pv) {
                if (parent[pv] != pu) { parent[pv] = pu; changed = true; }
            } else {
                if (parent[pu] != pv) { parent[pu] = pv; changed = true; }
            }
        });

        // === COMPRESS PHASE (Pointer Jumping / Shortcutting) ===
        // Paper: "shortcutting involves collapsing trees using pointer doubling"
        // Each node i -> grandparent of i, halving the tree depth.
        // Thread i has exclusive write to parent[i] (parallel_for partitions).
        // Read of parent[parent[i]] may observe a stale value from hook phase;
        // this is safe — it just leads to one fewer level of compression
        // which is corrected in the next iteration.
        parallel_for(0, n, [&](int i, int) {
            if (component[i] != -1 || parent[i] == -1 || parent[i] == i) return;
            int p = parent[i];
            int gp = (p != -1) ? parent[p] : p;
            if (gp != -1 && gp != p) parent[i] = gp;
        });

        if (!changed) break;
    }
}

HybridResult HybridCC::run(
    int n,
    const EdgeList& edges,
    const vector<vector<int>>& adj
) {
    vector<char> visited(n, 0);
    vector<int>  component(n, -1);
    int comp_id = 0;

    // ---- PHASE 0: DEGREE + SCALE-FREE PREDICTION ----
    // Paper Section 3.2: compute degree distribution, fit power-law,
    // evaluate KS statistic to decide whether to run BFS first.
    vector<int> degree = compute_degrees(n, edges);
    bool use_bfs = is_scale_free(degree);   // paper: KS < tau

    // ---- PHASE 1 (conditional): PARALLEL BFS ----
    // Only if scale-free topology detected. Run ONE BFS from the
    // highest-degree seed to capture the dominant large component.
    // Paper: "run BFS to label VI = largest component, then filter"
    if (use_bfs) {
        int seed = (int)(max_element(degree.begin(), degree.end()) - degree.begin());
        parallel_bfs_single(seed, adj, visited, component, comp_id);
        comp_id++;
    }

    // ---- PHASE 2: PARALLEL SV on remainder ----
    // Paper line 13: "Parallel-SV(G(V\VI, E\EI))"
    // SV operates only on nodes with component[i]==-1 (not yet labeled).
    vector<int> parent(n, -1);
    parallel_sv(n, edges, parent, component);

    // ---- PHASE 3: FINAL LABELING ----
    vector<int> root_to_comp(n, -1);
    for (int i = 0; i < n; i++) {
        if (component[i] != -1) continue;

        int r = i;
        // Walk to SV root (may be partially compressed)
        while (parent[r] != -1 && parent[r] != r) r = parent[r];

        if (r == -1 || parent[r] == -1) {
            // Isolated node: own component
            component[i] = comp_id++;
        } else {
            if (root_to_comp[r] == -1) root_to_comp[r] = comp_id++;
            component[i] = root_to_comp[r];
        }
    }

    return {component, comp_id};
}