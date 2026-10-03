#pragma once
#include <fstream>
#include <filesystem>
#include <string>

class Logger {
public:

    // UNIFIED LOGGER (ALL EXPERIMENTS)
    static void log_runtime(
        const std::string& exp_name,   // exp1_size, exp2_type, etc.
        int nodes,
        int edges,
        const std::string& graph_type,
        double param,               
        int run_id,
        long long hybrid_time,
        long long dsu_time
    ) {
        // CREATE DIRECTORY
        std::string dir = "logs/" + exp_name;
        std::filesystem::create_directories(dir);

        std::string file = dir + "/runtime.json";

        std::ofstream out(file, std::ios::app);

        if (!out.is_open()) return;   // safety

        // WRITE JSON LINE
        out << "{"
            << "\"experiment\":\"" << exp_name << "\","
            << "\"nodes\":" << nodes << ","
            << "\"edges\":" << edges << ","
            << "\"graph_type\":\"" << graph_type << "\","
            << "\"param\":" << param << ","
            << "\"run\":" << run_id << ","
            << "\"hybrid_time\":" << hybrid_time << ","
            << "\"dsu_time\":" << dsu_time
            << "}\n";

        out.close();
    }
};