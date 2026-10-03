#include "Src_Prjt-AI25MTECH14008-DSU.hpp"
#include "../../parallel/ParallelFor.hpp"

#include <vector>
#include <algorithm>
#include <random>
#include <atomic>
#include <limits>
#include <cmath>

using namespace std;

static inline int find_root(int x, const vector<int>& parent) {
    while (parent[x] != x) x = parent[x];
    return x;
}

// Mutable version for path splitting (used in compression phase only
// where write domains are exclusive per thread)
static inline int find_root_split(int x, vector<int>& parent) {
    while (parent[x] != x) {
        int gp = parent[parent[x]];
        parent[x] = gp;   // one-try split: point to grandparent
        x = gp;
    }
    return x;
}

static inline void link_by_rank(
    int pu, int pv,          // two distinct roots (pu < pv by convention)
    vector<int>& parent,
    vector<int>& rank
) {
    // Link smaller-rank to larger-rank (keeps depth O(log n))
    if (rank[pu] < rank[pv]) {
        parent[pu] = pv;     // pu's tree is shorter → link pu under pv
    } else if (rank[pu] > rank[pv]) {
        parent[pv] = pu;     // pv's tree is shorter → link pv under pu
    } else {
        parent[pv] = pu;     // equal rank: arbitrary choice, bump pu's rank
        rank[pu]++;
    }
}


static inline bool write_min(vector<atomic<int>>& arr, int slot, int val) {
    int cur = arr[slot].load(memory_order_relaxed);
    while (val < cur) {
        if (arr[slot].compare_exchange_weak(cur, val,
                memory_order_acq_rel, memory_order_relaxed)) {
            return true;
        }
        // cur is updated by CAS on failure → loop with fresh value
    }
    return (arr[slot].load(memory_order_acquire) == val);
}


// ──────────────────────────────────────────────────────────────
// PARALLEL DSU MAIN
// ──────────────────────────────────────────────────────────────
DSUResult ParallelDSU::run(int n, const EdgeList& input_edges) {

    // ── STEP 0: SHUFFLE EDGES ──────────────────────────────────
    // Paper Theorem 4.2: expected contentions for T threads =
    //   O(T² · log|V| · log|E|)   under RANDOM edge ordering.
    // Without shuffle, adversarial orderings give Ω(|E|) collisions.
    // Shuffle is O(|E|) and pays for itself immediately.
    vector<Edge> edges = input_edges;
    mt19937 rng(42);
    shuffle(edges.begin(), edges.end(), rng);

    int E = (int)edges.size();

    // ── STEP 1: INITIALISE FOREST ──────────────────────────────
    // Each node is its own root. rank[] bounds tree depth to O(log n).
    vector<int> parent(n);
    vector<int> rank(n, 0);

    parallel_for(0, n, [&](int i, int) { parent[i] = i; });

    // ── STEP 2: RESERVATION TABLE ──────────────────────────────
    // reservation[r] = index of edge that has claimed root r.
    // Initialised to INT_MAX ("unclaimed"). Reset between windows.
    const int UNCLAIMED = numeric_limits<int>::max();
    vector<atomic<int>> reservation(n);
    parallel_for(0, n, [&](int i, int) {
        reservation[i].store(UNCLAIMED, memory_order_relaxed);
    });

    // ── STEP 3: ADAPTIVE PREFIX SIZE ───────────────────────────
    // Paper Appendix B / Figure 5:
    //   "The adaptive algorithm changes the prefix size S between
    //    iterations. When no collisions occur, the algorithm doubles S.
    //    When there is a collision in the first half, S is halved."
    //
    // Initial S = cube root of |E| (Corollary 4.5 optimal starting
    // point for work efficiency when T = O(|E|^{1/3 - eps})).
    extern int GLOBAL_NUM_THREADS;
    int T = (GLOBAL_NUM_THREADS > 0) ? GLOBAL_NUM_THREADS : 4;

    // S must be >= T so every thread gets at least one edge per window
    int S = max(T, (int)cbrt((double)E));
    const int S_MIN = T;
    const int S_MAX = max(E, 1);      // cap at total edge count

    int cursor = 0;

    while (cursor < E) {

        int window_end = min(cursor + S, E);
        int wsize = window_end - cursor;

        vector<int> ru(wsize), rv(wsize);

        parallel_for(0, wsize, [&](int i, int) {
            auto [u, v] = edges[cursor + i];
            ru[i] = find_root(u, parent);   // read-only traverse
            rv[i] = find_root(v, parent);   // benign concurrent reads

            if (ru[i] == rv[i]) return;     // same component, no claim needed

            // Determine smaller root (by rank, then by index for determinism)
            int small_r;
            if (rank[ru[i]] < rank[rv[i]])       small_r = ru[i];
            else if (rank[ru[i]] > rank[rv[i]])   small_r = rv[i];
            else                                   small_r = min(ru[i], rv[i]);

            write_min(reservation, small_r, cursor + i);
        });

        int stop = window_end;   // default: no collision, process all

        for (int i = 0; i < wsize; i++) {
            int abs_i = cursor + i;
            if (ru[i] == rv[i]) continue;   // same component, no collision

            int small_r;
            if (rank[ru[i]] < rank[rv[i]])       small_r = ru[i];
            else if (rank[ru[i]] > rank[rv[i]])   small_r = rv[i];
            else                                   small_r = min(ru[i], rv[i]);

            int claimed_by = reservation[small_r].load(memory_order_acquire);
            if (claimed_by != abs_i) {
                // Collision detected at position i
                stop = cursor + i;
                break;
            }
        }

        int sz = stop - cursor;   // number of edges to actually process

        parallel_for(0, sz, [&](int i, int) {
            int abs_i = cursor + i;
            if (ru[i] == rv[i]) return;

            int small_r;
            if (rank[ru[i]] < rank[rv[i]])       small_r = ru[i];
            else if (rank[ru[i]] > rank[rv[i]])   small_r = rv[i];
            else                                   small_r = min(ru[i], rv[i]);

            // Only the winning thread links
            if (reservation[small_r].load(memory_order_acquire) != abs_i) return;

            // Re-find actual current roots (may have changed via earlier
            // links within this same window)
            int pu = find_root(ru[i], parent);
            int pv = find_root(rv[i], parent);
            if (pu == pv) return;

            // Rank-based link (paper Section 2: bounds depth to O(log n))
            link_by_rank(
                (rank[pu] <= rank[pv]) ? pu : pv,   // smaller-rank first arg
                (rank[pu] <= rank[pv]) ? pv : pu,
                parent, rank
            );
        });

        parallel_for(0, wsize, [&](int i, int) {
            if (ru[i] == rv[i]) return;

            int small_r;
            if (rank[ru[i]] < rank[rv[i]])       small_r = ru[i];
            else if (rank[ru[i]] > rank[rv[i]])   small_r = rv[i];
            else                                   small_r = min(ru[i], rv[i]);

            reservation[small_r].store(UNCLAIMED, memory_order_relaxed);
        });

        if (sz == wsize) {
            S = min(S * 2, S_MAX);          // no collision: grow window
        } else if (sz < wsize / 2) {
            S = max(S / 2, S_MIN);          // early collision: shrink window
        }
        
        cursor += max(sz, 1);
    }

    parallel_for(0, n, [&](int i, int) {
        find_root_split(i, parent);   // modifies parent[i] and ancestors
        // Note: find_root_split may write parent[x] for x != i.
        // This is the same benign concurrent-write situation as in
        // classic concurrent Union-Find: both writers push the node
        // closer to the root, never farther. Tree invariant preserved.
    });

    // Pass 2: direct root assignment (fully parallel, zero races)
    parallel_for(0, n, [&](int i, int) {
        // After pass 1, paths are shortened. Walk the (now short) path
        // to root and point i directly. find_root is now very cheap.
        parent[i] = find_root(i, parent);
    });

    vector<int> root_to_comp(n, -1);
    vector<int> component(n);
    int comp_id = 0;

    for (int i = 0; i < n; i++) {
        int r = parent[i];
        if (root_to_comp[r] == -1) root_to_comp[r] = comp_id++;
        component[i] = root_to_comp[r];
    }

    return {component, comp_id};
}