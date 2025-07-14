#include "stats.hpp"
#include "logging.hpp"
#include <iomanip>

namespace perf_monitor {
    
    void FunctionStats::update(long long duration) {
        double d = static_cast<double>(duration);
        // Update stats
        callCount++;
        totalTime += d;
        avgTime = static_cast<double>(totalTime) / callCount;

        // Update fastest/slowest times
        if (d > slowestTime) slowestTime = d;
        if (d < fastestTime) fastestTime = d;
    }

    void FunctionStats::outputStats() {
        outputStats(45);
    }
    
    void FunctionStats::outputStats(int nameWidth) {
        DEBUG_LOG(
                std::fixed << std::setprecision(3)
                           << "[" << std::left << std::setw(nameWidth) << name << "] "
                           << "Calls: " << std::right << std::setw(8) << callCount
                           << ", Total: " << std::right << std::setw(8) << totalTime / 1000.0 << " ms"
                           << ", Avg: " << std::right << std::setw(6) << avgTime / 1000.0 << " ms"
                           << ", Slowest: " << std::right << std::setw(7) << slowestTime / 1000.0 << " ms"
                           << ", Fastest: " << std::right << std::setw(6)
                           << (fastestTime == 999999999LL ? 0 : fastestTime / 1000.0) << " ms"
        );
    }

    void FunctionStats::clearStats() {
        // Reset all stats after output
        totalTime = 0;
        callCount = 0;
        avgTime = 0.0;
        slowestTime = 0;
        fastestTime = 999999999LL;
    }

    bool FunctionStats::checkAndOutputStats(uint64_t nowMs) {
        if (periodStartTime == 0) {
            periodStartTime = nowMs;
            return false;
        }
        if (nowMs - periodStartTime >= STATS_OUTPUT_INTERVAL_MS) {
            return true;
        }
        return false;
    }
}