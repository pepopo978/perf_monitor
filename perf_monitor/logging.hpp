//
// Created by pmacc on 9/29/2024.
//

#pragma once

#include <fstream>
#include <chrono>

namespace perf_monitor {
    extern std::ofstream debugLogFile;

    extern uint32_t gStartTime;
    extern uint32_t GetTime();

#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

// TODO uncomment once ready for release
//#ifdef _DEBUG
//std::ofstream debugLogFile("perf_monitor_debug.log");
//#define DEBUG_LOG(msg) debugLogFile << "[DEBUG]" << GetTime() << ": " << msg << std::endl
//#else
//#define DEBUG_LOG(msg) // No-op in release mode
//#endif

#define DEBUG_LOG(msg) debugLogFile << "[DEBUG]" << GetTime() << ": " << msg << std::endl
#define DEBUG_LOG2(msg) debugLogFile << msg

#endif  // DEBUG_LOG_H
}