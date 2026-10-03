#pragma once
#include <vector>
#include <thread>
#include <algorithm>

// GLOBAL THREAD CONTROL
inline int GLOBAL_NUM_THREADS = std::thread::hardware_concurrency();

// PARALLEL FOR (WITH THREAD ID)
template <typename Func>
void parallel_for(int start, int end, Func fn) {

    int total = end - start;
    if (total <= 0) return;

    int threads = GLOBAL_NUM_THREADS;
    if (threads <= 0) threads = 4;

    int chunk = (total + threads - 1) / threads;

    std::vector<std::thread> workers;

    for (int t = 0; t < threads; t++) {
        int l = start + t * chunk;
        int r = std::min(l + chunk, end);

        if (l >= r) continue;

        workers.emplace_back([=, &fn]() {
            for (int i = l; i < r; i++) {
                fn(i, t);  
            }
        });
    }

    for (auto &th : workers) {
        th.join();
    }
}