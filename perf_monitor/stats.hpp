#pragma once

#include <string>
#include <cstdint>

namespace perf_monitor {
    constexpr uint64_t STATS_OUTPUT_INTERVAL_MS = 30000; // 30 seconds

    // Struct to encapsulate function performance statistics
    struct FunctionStats {
        std::string name;           // Name of the function being monitored
        double totalTime = 0;    // Cumulative execution time in microseconds
        size_t callCount = 0;       // Number of calls
        double avgTime = 0.0;       // Running average time in microseconds
        double slowestTime = 0;  // Slowest execution time in microseconds
        double fastestTime = 999999999LL; // Fastest execution time in microseconds
        uint64_t lastStatsOutputTime = 0; // Last time stats were output
        uint64_t periodStartTime = 0;

        // Default constructor (required for map's operator[])
        FunctionStats() : name("Unknown") {}

        FunctionStats(std::string functionName) : name(std::move(functionName)) {}

        // Update stats with a new execution time
        void update(long long duration);

        // Output statistics for this function and then reset the stats
        void outputStats();
        
        // Output statistics with custom width
        void outputStats(int nameWidth);

        void clearStats();

        // Check if it's time to output stats and do so if needed
        bool checkAndOutputStats(uint64_t nowMs);
    };

    // Structure to track slowest events for each addon
    struct EventStats {
        int eventCode;
        double duration;
        size_t count;

        EventStats(int code, double dur) : eventCode(code), duration(dur), count(1) {}
        
        bool operator<(const EventStats& other) const {
            return duration > other.duration; // For max heap (slowest first)
        }
    };
}