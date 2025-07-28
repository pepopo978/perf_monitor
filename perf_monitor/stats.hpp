#pragma once

#include <string>
#include <cstdint>
#include <map>
#include <vector>
#include <chrono>
#include <iomanip>
#include <iostream>
#include "logging.hpp"

namespace perf_monitor {
    // Forward declarations
    enum EVENT_ID : int;
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

    // Structure to track memory usage for addons
    struct MemoryStats {
        std::string name;
        long long totalMemoryIncrease = 0;  // Total memory increase in KB
        long long maxMemoryIncrease = 0;    // Largest single memory increase in KB
        size_t callCount = 0;               // Number of calls tracked
        double avgMemoryIncrease = 0.0;     // Average memory increase per call in KB

        MemoryStats() : name("Unknown") {}
        MemoryStats(std::string addonName) : name(std::move(addonName)) {}

        void update(int memoryDelta) {
            if (memoryDelta > 0) {  // Only track increases
                callCount++;
                totalMemoryIncrease += memoryDelta;
                if (memoryDelta > maxMemoryIncrease) {
                    maxMemoryIncrease = memoryDelta;
                }
                avgMemoryIncrease = static_cast<double>(totalMemoryIncrease) / callCount;
            }
        }

        void clearStats() {
            totalMemoryIncrease = 0;
            maxMemoryIncrease = 0;
            callCount = 0;
            avgMemoryIncrease = 0.0;
        }

        void outputStats() {
            if (callCount > 0) {
                DEBUG_LOG(
                    std::fixed << std::setprecision(1)
                               << "[" << std::left << std::setw(45) << name << "] "
                               << "Calls: " << std::right << std::setw(6) << callCount
                               << ", Total Mem: " << std::right << std::setw(8) << totalMemoryIncrease << " KB"
                               << ", Avg: " << std::right << std::setw(6) << avgMemoryIncrease << " KB"
                               << ", Max: " << std::right << std::setw(6) << maxMemoryIncrease << " KB"
                );
            }
        }
    };

    // Global statistics objects - extern declarations
    extern FunctionStats gRenderWorldStats;
    extern FunctionStats gOnWorldRenderStats;
    extern FunctionStats gOnWorldUpdateStats;
    extern FunctionStats gSpellVisualsRenderStats;
    extern FunctionStats gSpellVisualsTickStats;
    extern FunctionStats gUnitUpdateStats;
    extern FunctionStats gFrameOnLayerUpdateStats;
    extern FunctionStats gCWorldRenderStats;
    extern FunctionStats gCWorldUpdateStats;
    extern FunctionStats gCWorldSceneRenderStats;
    extern FunctionStats gCWorldUnknownRenderStats;
    extern FunctionStats gTimeBetweenRenderStats;
    extern FunctionStats gFrameOnScriptEventStats;
    extern FunctionStats gCSimpleFrameOnFrameRender1Stats;
    extern FunctionStats gCSimpleFrameOnFrameRender2Stats;
    extern FunctionStats gCSimpleModelOnFrameRenderStats;
    extern FunctionStats gCSimpleTopOnLayerUpdateStats;
    extern FunctionStats gCSimpleTopOnLayerRenderStats;
    extern FunctionStats gObjectUpdateHandlerStats;
    extern FunctionStats gPlaySpellVisualStats;
    extern FunctionStats gUnknownOnRender1Stats;
    extern FunctionStats gUnknownOnRender2Stats;
    extern FunctionStats gUnknownOnRender3Stats;
    extern FunctionStats gCM2SceneAdvanceTimeStats;
    extern FunctionStats gCM2SceneAnimateStats;
    extern FunctionStats gCM2SceneDrawStats;
    extern FunctionStats gPaintScreenStats;
    extern FunctionStats gDrawBatchProjStats;
    extern FunctionStats gDrawBatchStats;
    extern FunctionStats gDrawBatchDoodadStats;
    extern FunctionStats gDrawRibbonStats;
    extern FunctionStats gDrawParticleStats;
    extern FunctionStats gDrawCallbackStats;
    extern FunctionStats gCM2SceneRenderDrawStats;
    extern FunctionStats gTotalEventsStats;
    extern FunctionStats gLuaCCollectgarbageStats;
    extern FunctionStats gCM2ModelAnimateMTStats;
    extern FunctionStats gObjectFreeStats;

    // Global maps and containers (not already declared in events.hpp)
    extern std::map<std::string, std::vector<EventStats>> gAddonEventStats;
    extern std::map<uint32_t, FunctionStats> gSpellVisualStatsById;
    extern std::map<std::string, MemoryStats> gAddonOnEventMemoryStats;
    extern std::map<std::string, MemoryStats> gAddonOnUpdateMemoryStats;

    // OutputStats function declaration
    void OutputStats(uint64_t startTime, uint64_t endTime);
}