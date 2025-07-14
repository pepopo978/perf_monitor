/*
    Copyright (c) 2017-2023, namreeb (legal@namreeb.org)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    The views and conclusions contained in the software and documentation are those
    of the authors and should not be interpreted as representing official policies,
    either expressed or implied, of the FreeBSD Project.
*/

#include "logging.hpp"
#include "offsets.hpp"
#include "main.hpp"
#include "stats.hpp"
#include "events.hpp"

#include <cstdint>
#include <memory>
#include <atomic>
#include <map>
#include <set>
#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <intrin.h>

BOOL WINAPI DllMain(HINSTANCE, uint32_t, void *);

namespace perf_monitor {

    // Dynamic detour storage system
    std::vector<std::unique_ptr<hadesmem::PatchDetourBase>> gDetours;

    // Keep SpellVisualsInitDetour as it's used specifically in DllMain
    std::unique_ptr<hadesmem::PatchDetour<SpellVisualsInitializeT >> gSpellVisualsInitDetour;

    // Create stats objects for each monitored function
    FunctionStats gRenderWorldStats("RenderWorld");
    FunctionStats gOnWorldRenderStats("OnWorldRender");
    FunctionStats gOnWorldUpdateStats("OnWorldUpdate");
    FunctionStats gSpellVisualsRenderStats("SpellVisualsRender");
    FunctionStats gSpellVisualsTickStats("SpellVisualsTick");
    FunctionStats gUnitUpdateStats("UnitUpdate");
    FunctionStats gFrameOnLayerUpdateStats("All OnUpdates");
    FunctionStats gCWorldRenderStats("CWorldRender");
    FunctionStats gCWorldUpdateStats("CWorldUpdate");
    FunctionStats gCWorldSceneRenderStats("CWorldSceneRender");
    FunctionStats gCWorldUnknownRenderStats("CWorldUnknownRender");
    FunctionStats gTimeBetweenRenderStats("TimeBetweenRender");
    FunctionStats gFrameOnScriptEventStats("All Event Handling");
    FunctionStats gCSimpleFrameOnFrameRender1Stats("CSimpleFrameOnFrameRender1");
    FunctionStats gCSimpleFrameOnFrameRender2Stats("CSimpleFrameOnFrameRender2");
    FunctionStats gCSimpleModelOnFrameRenderStats("CSimpleModelOnFrameRender");
    FunctionStats gCSimpleTopOnLayerUpdateStats("UIParent OnUpdate");
    FunctionStats gCSimpleTopOnLayerRenderStats("UIParent OnRender");


    // Track 5 slowest events for each addon
    std::map<std::string, std::vector<EventStats>> gAddonEventStats;

    // Track addon OnUpdate performance
    std::map<std::string, FunctionStats> gAddonOnUpdateStats;

    // Early alphabetical Ace addons that are likely to get associated with other addons events
    std::set<std::string> gAceAddonBlacklist = {
            "AI_VoiceOver",
            "AtlasLoot",
            "BigWigs"
    };

    // Total events stats
    FunctionStats gTotalEventsStats("All Events");

    // Track event counts
    std::map<EVENT_ID, size_t> gEventCounts;
    uint64_t gLastEventStatsTime = 0;
    std::chrono::high_resolution_clock::time_point gCWorldSceneRenderEndTime;

    uint32_t GetTime() {
        return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count()) - gStartTime;
    }

    uint64_t GetWowTimeMs() {
        auto const osGetAsyncTimeMs = reinterpret_cast<GetTimeMsT>(Offsets::OsGetAsyncTimeMs);
        return osGetAsyncTimeMs();
    }


    uintptr_t *GetLuaStatePtr() {
        typedef uintptr_t *(__fastcall *GETCONTEXT)(void);
        static auto p_GetContext = reinterpret_cast<GETCONTEXT>(0x7040D0);
        return p_GetContext();
    }

    std::string format_timestamp_ms(uint64_t ms_since_epoch) {
        auto const time_point = std::chrono::time_point<std::chrono::system_clock>(
                std::chrono::milliseconds(ms_since_epoch));
        auto const time_t_val = std::chrono::system_clock::to_time_t(time_point);
        std::tm tm_val;
        localtime_s(&tm_val, &time_t_val);

        std::stringstream ss;
        ss << std::put_time(&tm_val, "%H:%M:%S");
        return ss.str();
    }

    // Helper function to track slowest events for an addon
    std::string getAddonOrFrameName(uintptr_t *framescriptObj, uintptr_t *addonNamePtr) {
        std::string addonName;
        std::string frameNameStr;

        // Try to get addon name
        if (addonNamePtr != nullptr && addonNamePtr[1] != 0) {
            const char *namePtr = reinterpret_cast<const char *>(addonNamePtr[1]);
            if (namePtr != nullptr && IsBadReadPtr(namePtr, 1) == 0) {
                addonName = std::string(namePtr);
            }
        }

        // Try to get frame name
        if (framescriptObj != nullptr && IsBadReadPtr(framescriptObj, sizeof(uintptr_t) * 39) == 0 && framescriptObj[38] != 0) {
            auto frameName = reinterpret_cast<char *>(framescriptObj[38]);
            if (frameName != nullptr && IsValidAsciiString(frameName)) {
                frameNameStr = std::string(frameName);
                
                // Ignore frame names ending with .dll
                if (frameNameStr.length() >= 4 && frameNameStr.substr(frameNameStr.length() - 4) == ".dll") {
                    frameNameStr = "";
                } else if (frameNameStr.length() >= 2 && frameNameStr.substr(0, 2) == "pf") {
                    // Special handling for pfUI
                    frameNameStr = "pfUI";
                } else if (frameNameStr.length() >= 7 && frameNameStr.substr(0, 7) == "Cursive") {
                    frameNameStr = "Cursive";
                } else if (frameNameStr.length() >= 7 && frameNameStr.substr(0, 7) == "BigWigs") {
                    frameNameStr = "BigWigs";
                } else if ((frameNameStr.length() >= 4 && frameNameStr.substr(0, 4) == "MSBT") || (frameNameStr.length() >= 4 && frameNameStr.substr(0, 4) == "MCEH")) {
                    frameNameStr = "MSBT";
                } else if (frameNameStr.length() >= 13 && frameNameStr.substr(0, 9) == "Character" && frameNameStr.substr(frameNameStr.length() - 4) == "Slot") {
                    frameNameStr = "CharacterSlot";
                } else if (frameNameStr.length() >= 11 && frameNameStr.substr(0, 7) == "Inspect" && frameNameStr.substr(frameNameStr.length() - 4) == "Slot") {
                    frameNameStr = "InspectSlot";
                } else if (frameNameStr.length() >= 8 && frameNameStr.substr(0, 8) == "RABFrame") {
                    frameNameStr = "Rabuffs";
                } else {
                    // Strip numbers from frame name
                    frameNameStr.erase(std::remove_if(frameNameStr.begin(), frameNameStr.end(), ::isdigit), frameNameStr.end());
                }
            }
        }

        // Prioritize addon name if it exists and is not blacklisted
        if (!addonName.empty() && gAceAddonBlacklist.find(addonName) == gAceAddonBlacklist.end()) {
            return addonName;
        }

        // Fall back to frame name if addon name is not available
        if (!frameNameStr.empty()) {
            return frameNameStr;
        }

        return "";
    }

    void TrackEvent(const std::string &addonName, int eventCode, double duration) {
        auto &eventStats = gAddonEventStats[addonName];

        // Find existing event with same code
        auto it = std::find_if(eventStats.begin(), eventStats.end(),
                               [eventCode](const EventStats &event) {
                                   return event.eventCode == eventCode;
                               });

        if (it != eventStats.end()) {
            // Combine durations and increment count for existing event
            it->duration += duration;
            it->count++;
        } else {
            // Add new event code
            eventStats.emplace_back(eventCode, duration);
        }
    }

    void OutputStats(uint64_t startTime, uint64_t endTime) {
        auto callCount = gRenderWorldStats.callCount;

        // Store the totals before clearing the stats
        double totalCSimpleTopOnLayerUpdate = gCSimpleTopOnLayerUpdateStats.totalTime;
        double totalCSimpleTopOnLayerRender = gCSimpleTopOnLayerRenderStats.totalTime;

        double totalOnWorldRender = gOnWorldRenderStats.totalTime;
        double totalOnWorldUpdate = gOnWorldUpdateStats.totalTime;
        double totalSpellVisualsRender = gSpellVisualsRenderStats.totalTime;
        double totalSpellVisualsTick = gSpellVisualsTickStats.totalTime;
        double totalUnitUpdate = gUnitUpdateStats.totalTime;
        double totalFrameOnLayerUpdate = gFrameOnLayerUpdateStats.totalTime;
        double totalCWorldRender = gCWorldRenderStats.totalTime;
        double totalCWorldUpdate = gCWorldUpdateStats.totalTime;
        double totalCWorldSceneRender = gCWorldSceneRenderStats.totalTime;
        double totalCWorldUnknownRender = gCWorldUnknownRenderStats.totalTime;
        double totalTimeBetweenRender = gTimeBetweenRenderStats.totalTime;

        double totalFrameOnScriptEvent = gFrameOnScriptEventStats.totalTime;
        double totalCSimpleFrameOnFrameRender1 = gCSimpleFrameOnFrameRender1Stats.totalTime;
        double totalCSimpleFrameOnFrameRender2 = gCSimpleFrameOnFrameRender2Stats.totalTime;
        double totalCSimpleModelOnFrameRender = gCSimpleModelOnFrameRenderStats.totalTime;

        // Get the total time for evt_Paint and evt_Idle
        double totalEvtPaint = gEventStats[EVENT_ID_PAINT].totalTime;
        double totalEvtIdle = gEventStats[EVENT_ID_IDLE].totalTime;

        // Calculate percentages of frame time using cumulative times
        double onWorldRenderPercent = (totalCSimpleTopOnLayerRender > 0) ?
                                      (totalOnWorldRender / totalCSimpleTopOnLayerRender) * 100.0 : 0.0;
        double onWorldUpdatePercent = (totalCSimpleTopOnLayerRender > 0) ?
                                      (totalOnWorldUpdate / totalCSimpleTopOnLayerRender) * 100.0 : 0.0;
        double spellVisualsRenderPercent = (totalCSimpleTopOnLayerRender > 0) ?
                                           (totalSpellVisualsRender / totalCSimpleTopOnLayerRender) * 100.0
                                                                              : 0.0;
        double spellVisualsTickPercent = (totalCSimpleTopOnLayerRender > 0) ?
                                         (totalSpellVisualsTick / totalCSimpleTopOnLayerRender) * 100.0
                                                                            : 0.0;
        double unitUpdatePercent = (totalCSimpleTopOnLayerRender > 0) ?
                                   (totalUnitUpdate / totalCSimpleTopOnLayerRender) * 100.0 : 0.0;
        double frameOnLayerUpdatePercent = (totalCSimpleTopOnLayerRender > 0) ?
                                           (totalFrameOnLayerUpdate / totalCSimpleTopOnLayerRender) * 100.0
                                                                              : 0.0;
        double cWorldRenderPercent = (totalCSimpleTopOnLayerRender > 0) ?
                                     (totalCWorldRender / totalCSimpleTopOnLayerRender) * 100.0 : 0.0;
        double cWorldUpdatePercent = (totalCSimpleTopOnLayerRender > 0) ?
                                     (totalCWorldUpdate / totalCSimpleTopOnLayerRender) * 100.0 : 0.0;
        double cWorldSceneRenderPercent = (totalCSimpleTopOnLayerRender > 0) ?
                                          (totalCWorldSceneRender / totalCSimpleTopOnLayerRender) * 100.0
                                                                             : 0.0;
        double cWorldUnknownRenderPercent = (totalCSimpleTopOnLayerRender > 0) ?
                                            (totalCWorldUnknownRender / totalCSimpleTopOnLayerRender) *
                                            100.0 : 0.0;
        double timeBetweenRenderPercent = (totalCSimpleTopOnLayerRender > 0) ?
                                          (totalTimeBetweenRender / totalCSimpleTopOnLayerRender) * 100.0
                                                                             : 0.0;
        double frameOnScriptEventPercent = (totalCSimpleTopOnLayerRender > 0) ?
                                           (totalFrameOnScriptEvent / totalCSimpleTopOnLayerRender) * 100.0
                                                                              : 0.0;

        // Convert microseconds to milliseconds for display
        double totalOnWorldRenderMs = totalOnWorldRender / 1000.0;
        double totalOnWorldUpdateMs = totalOnWorldUpdate / 1000.0;
        double totalCWorldRenderMs = totalCWorldRender / 1000.0;
        double totalCWorldUpdateMs = totalCWorldUpdate / 1000.0;
        double totalCWorldSceneRenderMs = totalCWorldSceneRender / 1000.0;
        double totalCWorldUnknownRenderMs = totalCWorldUnknownRender / 1000.0;
        double totalTimeBetweenRenderMs = totalTimeBetweenRender / 1000.0;
        double totalSpellVisualsRenderMs = totalSpellVisualsRender / 1000.0;
        double totalSpellVisualsTickMs = totalSpellVisualsTick / 1000.0;
        double totalUnitUpdateMs = totalUnitUpdate / 1000.0;
        double totalFrameOnLayerUpdateMs = totalFrameOnLayerUpdate / 1000.0;
        double totalFrameOnScriptEventMs = totalFrameOnScriptEvent / 1000.0;
        double totalEvtPaintMs = totalEvtPaint / 1000.0;
        double totalEvtIdleMs = totalEvtIdle / 1000.0;
        double totalCSimpleTopOnLayerUpdateMs = totalCSimpleTopOnLayerUpdate / 1000.0;
        double totalCSimpleTopOnLayerRenderMs = totalCSimpleTopOnLayerRender / 1000.0;
        double totalCSimpleFrameOnFrameRender1Ms = totalCSimpleFrameOnFrameRender1 / 1000.0;
        double totalCSimpleFrameOnFrameRender2Ms = totalCSimpleFrameOnFrameRender2 / 1000.0;
        double totalCSimpleModelOnFrameRenderMs = totalCSimpleModelOnFrameRender / 1000.0;

        // --- SUMMARY ---
        DEBUG_LOG(
                "--------------------------------------------------------------------------------------------------------------------------------------");
        DEBUG_LOG("--- STATS from " << format_timestamp_ms(startTime) << " to " << format_timestamp_ms(endTime)
                                    << " ---");
        DEBUG_LOG(
                std::fixed << std::setprecision(2)
                           << "[Total] Render: " << std::right << std::setw(8) << totalCSimpleTopOnLayerRenderMs
                           << " ms.  Avg fps: "
                           << std::right << std::setw(6)
                           << (callCount > 0 ? callCount / (STATS_OUTPUT_INTERVAL_MS / 1000.0) : 0.0));

        {
            std::vector<std::pair<double, std::string>> allStats;
            std::stringstream ss;

            // Render stats
            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "OnWorldRender:"
               << std::right << std::setw(6) << onWorldRenderPercent << "% ("
               << std::right << std::setw(8) << totalOnWorldRenderMs << " ms)";
            allStats.emplace_back(onWorldRenderPercent, ss.str());

//            ss.str("");
//            ss << std::fixed << std::setprecision(2) << "CWorldRender: " << cWorldRenderPercent << "% ("
//               << totalCWorldRenderMs << " ms)";
//            allStats.emplace_back(cWorldRenderPercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "CWorldSceneRender:"
               << std::right << std::setw(6) << cWorldSceneRenderPercent << "% ("
               << std::right << std::setw(8) << totalCWorldSceneRenderMs << " ms)";
            allStats.emplace_back(cWorldSceneRenderPercent, ss.str());

//            ss.str("");
//            ss << std::fixed << std::setprecision(2) << "TimeBetweenRender: " << timeBetweenRenderPercent << "% ("
//               << totalTimeBetweenRenderMs << " ms)";
//            allStats.emplace_back(timeBetweenRenderPercent, ss.str());
//
//            ss.str("");
//            ss << std::fixed << std::setprecision(2) << "CWorldUnknownRender: " << cWorldUnknownRenderPercent << "% ("
//               << totalCWorldUnknownRenderMs << " ms)";
//            allStats.emplace_back(cWorldUnknownRenderPercent, ss.str());

//            ss.str("");
//            ss << std::fixed << std::setprecision(2) << "SpellVisualsTick: " << spellVisualsTickPercent << "% ("
//               << totalSpellVisualsTickMs << " ms)";
//            allStats.emplace_back(spellVisualsTickPercent, ss.str());
//
//            ss.str("");
//            ss << std::fixed << std::setprecision(2) << "SpellVisualsRender: " << spellVisualsRenderPercent << "% ("
//               << totalSpellVisualsRenderMs << " ms)";
//            allStats.emplace_back(spellVisualsRenderPercent, ss.str());

            // Update stats
            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "OnWorldUpdate:"
               << std::right << std::setw(6) << onWorldUpdatePercent << "% ("
               << std::right << std::setw(8) << totalOnWorldUpdateMs << " ms)";
            allStats.emplace_back(onWorldUpdatePercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "CWorldUpdate:"
               << std::right << std::setw(6) << cWorldUpdatePercent << "% ("
               << std::right << std::setw(8) << totalCWorldUpdateMs << " ms)";
            allStats.emplace_back(cWorldUpdatePercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "UnitUpdate:"
               << std::right << std::setw(6) << unitUpdatePercent << "% ("
               << std::right << std::setw(8) << totalUnitUpdateMs << " ms)";
            allStats.emplace_back(unitUpdatePercent, ss.str());

            // Frame stats
            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "All OnUpdates:"
               << std::right << std::setw(6) << frameOnLayerUpdatePercent << "% ("
               << std::right << std::setw(8) << totalFrameOnLayerUpdateMs << " ms)";
            allStats.emplace_back(frameOnLayerUpdatePercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "All Events:"
               << std::right << std::setw(6) << frameOnScriptEventPercent << "% ("
               << std::right << std::setw(8) << totalFrameOnScriptEventMs << " ms)";
            allStats.emplace_back(frameOnScriptEventPercent, ss.str());

            double cSimpleTopOnLayerUpdatePercent = (totalEvtPaintMs > 0 ? (totalCSimpleTopOnLayerUpdateMs /
                                                                            totalEvtPaintMs * 100.0) : 0.0);
            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "UIParent OnUpdate:"
               << std::right << std::setw(6) << cSimpleTopOnLayerUpdatePercent << "% ("
               << std::right << std::setw(8) << totalCSimpleTopOnLayerUpdateMs << " ms)";
            allStats.emplace_back(cSimpleTopOnLayerUpdatePercent, ss.str());

//            double cSimpleFrameOnFrameRender1Percent = (totalCSimpleTopOnLayerRender > 0) ? (totalCSimpleFrameOnFrameRender1 /
//                                                                                 totalCSimpleTopOnLayerRender * 100.0) : 0.0;
//            ss.str("");
//            ss << std::fixed << std::setprecision(2) << "CSimpleFrameOnFrameRender1: "
//               << cSimpleFrameOnFrameRender1Percent
//               << "% (" << totalCSimpleFrameOnFrameRender1Ms << " ms)";
//            allStats.emplace_back(cSimpleFrameOnFrameRender1Percent, ss.str());
//
//            double cSimpleFrameOnFrameRender2Percent = (totalCSimpleTopOnLayerRender > 0) ? (totalCSimpleFrameOnFrameRender2 /
//                                                                                 totalCSimpleTopOnLayerRender * 100.0) : 0.0;
//            ss.str("");
//            ss << std::fixed << std::setprecision(2) << "CSimpleFrameOnFrameRender2: "
//               << cSimpleFrameOnFrameRender2Percent
//               << "% (" << totalCSimpleFrameOnFrameRender2Ms << " ms)";
//            allStats.emplace_back(cSimpleFrameOnFrameRender2Percent, ss.str());
//
//            double cSimpleModelOnFrameRenderPercent = (totalCSimpleTopOnLayerRender > 0) ? (totalCSimpleModelOnFrameRender /
//                                                                                totalCSimpleTopOnLayerRender * 100.0) : 0.0;
//            ss.str("");
//            ss << std::fixed << std::setprecision(2) << "CSimpleModelOnFrameRender: "
//               << cSimpleModelOnFrameRenderPercent
//               << "% (" << totalCSimpleModelOnFrameRenderMs << " ms)";
//            allStats.emplace_back(cSimpleModelOnFrameRenderPercent, ss.str());
//
//            ss.str("");
//            ss << std::fixed << std::setprecision(2) << "OnScriptEventParam: " << frameOnScriptEventParamPercent
//               << "% (" << totalFrameOnScriptParamEventMs << " ms)";
//            allStats.emplace_back(frameOnScriptEventParamPercent, ss.str());

            std::sort(allStats.rbegin(), allStats.rend());

            for (const auto &stat: allStats) {
                DEBUG_LOG(stat.second);
            }
        }

        // --- DETAILED STATS ---
        DEBUG_LOG("--- DETAILED STATS ---");
        gCSimpleTopOnLayerRenderStats.outputStats();
        gCSimpleTopOnLayerUpdateStats.outputStats();
//        gRenderWorldStats.outputStats();
        gOnWorldRenderStats.outputStats();
        gOnWorldUpdateStats.outputStats();
//        gCWorldRenderStats.outputStats();
        gCWorldSceneRenderStats.outputStats();
        gCWorldUpdateStats.outputStats();
//        gCWorldUnknownRenderStats.outputStats();
//        gTimeBetweenRenderStats.outputStats();
//        gCSimpleFrameOnFrameRender1Stats.outputStats();
//        gCSimpleFrameOnFrameRender2Stats.outputStats();
//        gCSimpleModelOnFrameRenderStats.outputStats();
//        gSpellVisualsRenderStats.outputStats();
//        gSpellVisualsTickStats.outputStats();
        gUnitUpdateStats.outputStats();

        gFrameOnScriptEventStats.outputStats();
        gFrameOnLayerUpdateStats.outputStats();

        NEWLINE_LOG();

        // --- ADDON ONUPDATE PERFORMANCE ---
        if (!gAddonOnUpdateStats.empty()) {
            DEBUG_LOG("--- ADDON/FRAME ONUPDATE PERFORMANCE (min .1ms total)---");

            // Sort addons by total time
            std::vector<std::pair<double, std::string>> addonOnUpdateStats;
            for (auto it = gAddonOnUpdateStats.begin();
                 it != gAddonOnUpdateStats.end(); ++it) {
                if (it->second.callCount > 0 && it->second.totalTime >= 100.0) {
                    addonOnUpdateStats.push_back(std::make_pair(it->second.totalTime, it->first));
                }
            }
            std::sort(addonOnUpdateStats.rbegin(), addonOnUpdateStats.rend());

            for (auto it = addonOnUpdateStats.begin();
                 it != addonOnUpdateStats.end(); ++it) {
                gAddonOnUpdateStats[it->second].outputStats();
            }
        }

        // --- ADDON EVENT STATS ---
        if (!gAddonScriptEventStats.empty()) {
            DEBUG_LOG("--- ADDON/FRAME EVENTS PERFORMANCE (min .1ms total)---");

            // Sort addons by total time
            std::vector<std::pair<double, std::string>> addonStats;
            for (auto it = gAddonScriptEventStats.begin();
                 it != gAddonScriptEventStats.end(); ++it) {
                if (it->second.callCount > 0 && it->second.totalTime >= 100.0) {
                    addonStats.push_back(std::make_pair(it->second.totalTime, it->first));
                }
            }
            std::sort(addonStats.rbegin(), addonStats.rend());

            for (auto it = addonStats.begin();
                 it != addonStats.end(); ++it) {
                gAddonScriptEventStats[it->second].outputStats();
            }
        }

        // --- ADDON SLOWEST EVENTS REPORT ---
        if (!gAddonEventStats.empty()) {
            DEBUG_LOG("--- ADDON/FRAME SLOWEST EVENTS REPORT (min .1ms combined duration) ---");

            for (const auto &addonPair: gAddonEventStats) {
                const std::string &addonName = addonPair.first;
                std::vector<EventStats> slowEvents = addonPair.second; // Copy for sorting

                if (!slowEvents.empty()) {
                    // Calculate total duration for this addon
                    double totalAddonDuration = 0.0;
                    for (const auto &event : slowEvents) {
                        totalAddonDuration += event.duration;
                    }

                    // Skip if total duration is less than 0.1ms (100 microseconds)
                    if (totalAddonDuration < 100.0) {
                        continue;
                    }

                    // Sort events by duration (descending) only when displaying
                    std::sort(slowEvents.begin(), slowEvents.end());

                    // Only show top 10 slowest events
                    size_t eventsToShow = slowEvents.size() < 10 ? slowEvents.size() : 10;
                    DEBUG_LOG("[" << addonName << "] Top " << eventsToShow << " slowest events (out of "
                                  << slowEvents.size() << " total):");

                    for (size_t i = 0; i < eventsToShow; ++i) {
                        const EventStats &event = slowEvents[i];
                        DEBUG_LOG("  " << std::right << std::setw(2) << (i + 1) << ".  "
                                       << std::left << std::setw(50) << GetEventName(event.eventCode)
                                       << " Total Duration: " << std::right << std::setw(8) << std::fixed
                                       << std::setprecision(3)
                                       << event.duration / 1000.0 << " ms"
                                       << "      Count: " << std::right << std::setw(6) << event.count);
                    }
                }
            }
        }

        // --- EVENT CODE DURATION STATISTICS (TOP 10) ---
        if (!gEventCodeStats.empty()) {
            DEBUG_LOG("--- TOTAL EVENT DURATION STATISTICS (SHOULD INCLUDE MISSING ADDONS) ---");

            // Sort event codes by total time
            std::vector<std::pair<double, int>> eventCodeStats;
            for (auto it = gEventCodeStats.begin(); it != gEventCodeStats.end(); ++it) {
                if (it->second.callCount > 0) {
                    eventCodeStats.push_back(std::make_pair(it->second.totalTime, it->first));
                }
            }
            std::sort(eventCodeStats.rbegin(), eventCodeStats.rend());

            // Show only top 10 events
            size_t eventsToShow = eventCodeStats.size() < 10 ? eventCodeStats.size() : 10;
            for (size_t i = 0; i < eventsToShow; ++i) {
                gEventCodeStats[eventCodeStats[i].second].outputStats(45);
            }
        }

        // Clear all stats
        gRenderWorldStats.clearStats();
        gOnWorldRenderStats.clearStats();
        gOnWorldUpdateStats.clearStats();
        gCWorldRenderStats.clearStats();
        gCWorldUpdateStats.clearStats();
        gCWorldSceneRenderStats.clearStats();
        gCWorldUnknownRenderStats.clearStats();
        gTimeBetweenRenderStats.clearStats();
        gFrameOnScriptEventStats.clearStats();
        gCSimpleFrameOnFrameRender1Stats.clearStats();
        gCSimpleFrameOnFrameRender2Stats.clearStats();
        gCSimpleModelOnFrameRenderStats.clearStats();
        gCSimpleTopOnLayerUpdateStats.clearStats();
        gCSimpleTopOnLayerRenderStats.clearStats();
        gSpellVisualsRenderStats.clearStats();
        gSpellVisualsTickStats.clearStats();
        gUnitUpdateStats.clearStats();
        gFrameOnLayerUpdateStats.clearStats();

        // Clear addon stats
        for (auto it = gAddonScriptEventStats.begin();
             it != gAddonScriptEventStats.end(); ++it) {
            it->second.clearStats();
        }

        // Clear addon OnUpdate stats
        for (auto it = gAddonOnUpdateStats.begin();
             it != gAddonOnUpdateStats.end(); ++it) {
            it->second.clearStats();
        }

        // Clear addon event stats
        gAddonEventStats.clear();

        // Clear event code stats
        for (auto it = gEventCodeStats.begin(); it != gEventCodeStats.end(); ++it) {
            it->second.clearStats();
        }
        gEventCodeStartTimes.clear();

        DEBUG_LOG(
                "--------------------------------------------------------------------------------------------------------------------------------------");

        NEWLINE_LOG();
    }

    // IEvtQueueDispatch hook to track events
    void
    IEvtQueueDispatchHook(hadesmem::PatchDetourBase *detour, uintptr_t *eventContext, EVENT_ID eventId, void *unk) {
        auto const IEvtQueueDispatch = detour->GetTrampolineT<IEvtQueueDispatchT>();

        // Check if the event is valid
        if (eventId < EVENT_ID_CAPTURECHANGED || eventId >= EVENTIDS) {
            // Just call original function for invalid events
            IEvtQueueDispatch(eventContext, eventId, unk);
            return;
        }

        // Track event count
        gEventCounts[eventId]++;

        // Time the event dispatch
        auto start = std::chrono::high_resolution_clock::now();

        // Call original function
        IEvtQueueDispatch(eventContext, eventId, unk);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        uint64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(end.time_since_epoch()).count();

        // Update event stats
        gEventStats[eventId].update(duration);
        gTotalEventsStats.update(duration);

        // Check if it's time to output event stats (every minute)
        if (gLastEventStatsTime == 0 || nowMs - gLastEventStatsTime >= STATS_OUTPUT_INTERVAL_MS) {
            gLastEventStatsTime = nowMs;
        }
    }

    // RenderWorld hook - this is the main hook that will output stats
    void RenderWorldHook(hadesmem::PatchDetourBase *detour, uintptr_t *worldFrame) {
        auto const RenderWorld = detour->GetTrampolineT<FastcallFrameT>();
        auto start = std::chrono::high_resolution_clock::now();
        RenderWorld(worldFrame);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        uint64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(end.time_since_epoch()).count();

        // Update frame stats
        gRenderWorldStats.update(duration);

        // Only output stats in the RenderWorldHook (once per minute)
        if (gRenderWorldStats.checkAndOutputStats(nowMs)) {
            OutputStats(gRenderWorldStats.periodStartTime, nowMs);
            gRenderWorldStats.periodStartTime = nowMs;
        }
    }

    void OnWorldRenderHook(hadesmem::PatchDetourBase *detour, uintptr_t *worldFrame) {
        auto const OnWorldRender = detour->GetTrampolineT<FastcallFrameT>();
        auto start = std::chrono::high_resolution_clock::now();
        OnWorldRender(worldFrame);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update frame stats
        gOnWorldRenderStats.update(duration);
    }

    void OnWorldUpdateHook(hadesmem::PatchDetourBase *detour, uintptr_t *worldFrame) {
        auto const OnWorldUpdate = detour->GetTrampolineT<FastcallFrameT>();
        auto start = std::chrono::high_resolution_clock::now();
        OnWorldUpdate(worldFrame);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update frame stats
        gOnWorldUpdateStats.update(duration);
    }

    // CWorldRender hook
    void CWorldRenderHook(hadesmem::PatchDetourBase *detour) {
        auto const CWorldRender = detour->GetTrampolineT<StdcallT>();
        auto start = std::chrono::high_resolution_clock::now();

        if (gCWorldSceneRenderEndTime.time_since_epoch().count() != 0) {
            auto timeBetween = std::chrono::duration_cast<std::chrono::microseconds>(
                    start - gCWorldSceneRenderEndTime).count();
            gTimeBetweenRenderStats.update(timeBetween);
        }

        CWorldRender();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gCWorldRenderStats.update(duration);
    }

    void CWorldSceneRenderHook(hadesmem::PatchDetourBase *detour) {
        auto const CWorldSceneRender = detour->GetTrampolineT<StdcallT>();
        auto start = std::chrono::high_resolution_clock::now();
        CWorldSceneRender();
        auto end = std::chrono::high_resolution_clock::now();

        gCWorldSceneRenderEndTime = end;

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gCWorldSceneRenderStats.update(duration);
    }

    // CWorldUnknownRender hook
    void CWorldUnknownRenderHook(hadesmem::PatchDetourBase *detour) {
        auto const CWorldUnknownRender = detour->GetTrampolineT<StdcallT>();
        auto start = std::chrono::high_resolution_clock::now();
        CWorldUnknownRender();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gCWorldUnknownRenderStats.update(duration);
    }

    // CWorldUpdate hook
    void CWorldUpdateHook(hadesmem::PatchDetourBase *detour, float *param_1, float *param_2, float *param_3) {
        auto const CWorldUpdate = detour->GetTrampolineT<WorldUpdateT>();
        auto start = std::chrono::high_resolution_clock::now();
        CWorldUpdate(param_1, param_2, param_3);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gCWorldUpdateStats.update(duration);
    }

    // SpellVisualsRender hook
    void SpellVisualsRenderHook(hadesmem::PatchDetourBase *detour) {
        auto const SpellVisualsRender = detour->GetTrampolineT<StdcallT>();
        auto start = std::chrono::high_resolution_clock::now();
        SpellVisualsRender();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gSpellVisualsRenderStats.update(duration);
    }

    // SpellVisualsTick hook
    void SpellVisualsTickHook(hadesmem::PatchDetourBase *detour) {
        auto const SpellVisualsTick = detour->GetTrampolineT<StdcallT>();
        auto start = std::chrono::high_resolution_clock::now();
        SpellVisualsTick();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gSpellVisualsTickStats.update(duration);
    }

    // UnitUpdate hook
    void UnitUpdateHook(hadesmem::PatchDetourBase *detour, uintptr_t *worldFrame) {
        auto const UnitUpdate = detour->GetTrampolineT<FastcallFrameT>();
        auto start = std::chrono::high_resolution_clock::now();
        UnitUpdate(worldFrame);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gUnitUpdateStats.update(duration);
    }

    // Add these new hook functions
    void
    CSimpleFrameOnFrameRender1Hook(hadesmem::PatchDetourBase *detour, uintptr_t *frame, void *param_1, uint32_t param_2,
                                   int unk) {
        auto const CSimpleFrameOnFrameRender1 = detour->GetTrampolineT<FrameBatchT>();
        auto start = std::chrono::high_resolution_clock::now();
        CSimpleFrameOnFrameRender1(frame, param_1, param_2, unk);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gCSimpleFrameOnFrameRender1Stats.update(duration);
    }

    void
    CSimpleModelOnFrameRenderHook(hadesmem::PatchDetourBase *detour, uintptr_t *frame, void *param_1, uint32_t param_2,
                                  int unk) {
        auto const CSimpleModelOnFrameRender = detour->GetTrampolineT<FrameBatchT>();
        auto start = std::chrono::high_resolution_clock::now();
        CSimpleModelOnFrameRender(frame, param_1, param_2, unk);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gCSimpleModelOnFrameRenderStats.update(duration);
    }

    void CSimpleFrameOnFrameRender2Hook(hadesmem::PatchDetourBase *detour, uintptr_t *frame) {
        auto const CSimpleFrameOnFrameRender2 = detour->GetTrampolineT<FastcallFrameT>();
        auto start = std::chrono::high_resolution_clock::now();
        CSimpleFrameOnFrameRender2(frame);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gCSimpleFrameOnFrameRender2Stats.update(duration);
    }

    void CSimpleTopOnLayerUpdateHook(hadesmem::PatchDetourBase *detour, uintptr_t *frame, uint8_t unk, int unk2) {
        auto const CSimpleTopOnLayerUpdate = detour->GetTrampolineT<FrameOnLayerUpdateT>();
        auto start = std::chrono::high_resolution_clock::now();
        CSimpleTopOnLayerUpdate(frame, unk, unk2);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gCSimpleTopOnLayerUpdateStats.update(duration);
    }

    void CSimpleTopOnLayerRenderHook(hadesmem::PatchDetourBase *detour, uintptr_t *frame) {
        auto const CSimpleTopOnLayerRender = detour->GetTrampolineT<FastcallFrameT>();
        auto start = std::chrono::high_resolution_clock::now();
        CSimpleTopOnLayerRender(frame);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gCSimpleTopOnLayerRenderStats.update(duration);
    }

    // FrameOnLayerUpdate hook
    void FrameOnLayerUpdateHook(hadesmem::PatchDetourBase *detour, uintptr_t *frame, uint8_t unk, int unk2) {
        auto const FrameOnLayerUpdate = detour->GetTrampolineT<FrameOnLayerUpdateT>();
        // check if onUpdate is set
        auto const onUpdate = reinterpret_cast<uintptr_t *>(frame + 0x128);
        if (!onUpdate || *onUpdate == 0) {
            FrameOnLayerUpdate(frame, unk, unk2);
        } else {
            // try to get addon name
            auto const addonNamePtr = reinterpret_cast<uintptr_t *>(frame + 74);
            auto addonName = getAddonOrFrameName(frame, addonNamePtr);

            if (addonName.empty()) {
                FrameOnLayerUpdate(frame, unk, unk2);
            } else {
                auto start = std::chrono::high_resolution_clock::now();
                FrameOnLayerUpdate(frame, unk, unk2);
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

                // Update overall stats
                gFrameOnLayerUpdateStats.update(duration);

                // Update addon-specific stats
                if (gAddonOnUpdateStats.find(addonName) == gAddonOnUpdateStats.end()) {
                    gAddonOnUpdateStats[addonName] = FunctionStats(addonName + " OnUpdate");
                }
                gAddonOnUpdateStats[addonName].update(duration);
            }
        }
    }


    void FrameOnScriptEventHook(hadesmem::PatchDetourBase *detour, int *param_1, int *param_2) {
        auto const FrameScriptObjectOnScriptEvent = detour->GetTrampolineT<FrameOnScriptEventT>();
        auto lastEventCode = gLastEventCode;

        auto start = std::chrono::high_resolution_clock::now();
        FrameScriptObjectOnScriptEvent(param_1, param_2);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update overall stats
        gFrameOnScriptEventStats.update(duration);

        if (param_2 != nullptr && param_2[1] != 0 && gLastEventCode != 0) {
            std::string addonName = getAddonOrFrameName(reinterpret_cast<uintptr_t *>(param_1),
                                                        reinterpret_cast<uintptr_t *>(param_2));
            if (!addonName.empty()) {

                // Update addon-specific stats
                if (gAddonScriptEventStats.find(addonName) == gAddonScriptEventStats.end()) {
                    gAddonScriptEventStats[addonName] = FunctionStats(addonName + " All Events");
                }
                gAddonScriptEventStats[addonName].update(duration);

                TrackEvent(addonName, lastEventCode, static_cast<double>(duration));
            }
        }
    }

    // FrameOnScriptEventParam hook
    void
    FrameOnScriptEventParamHook(hadesmem::PatchDetourBase *detour, int *framescriptObj, int *param_2, char *param_3,
                                va_list args) {
        auto const FrameOnScriptEventParam = detour->GetTrampolineT<FrameOnScriptEventParamT>();

        auto lastEventCode = gLastEventCode;
        if (lastEventCode == -1) {
            // this was called by FrameScript_Execute and was most likely an OnUpdate, ignore it for stat tracking.
            FrameOnScriptEventParam(framescriptObj, param_2, param_3, args);
            return;
        }

        auto start = std::chrono::high_resolution_clock::now();
        FrameOnScriptEventParam(framescriptObj, param_2, param_3, args);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update overall stats
        gFrameOnScriptEventStats.update(duration);

        if (framescriptObj != nullptr && param_2 != nullptr && gLastEventCode != 0) {
            auto addonName = getAddonOrFrameName(reinterpret_cast<uintptr_t *>(framescriptObj),
                                                 reinterpret_cast<uintptr_t *>(param_2));
            if (!addonName.empty()) {
                // Update addon-specific stats
                if (gAddonScriptEventStats.find(addonName) == gAddonScriptEventStats.end()) {
                    gAddonScriptEventStats[addonName] = FunctionStats(addonName + " All Events");
                }
                gAddonScriptEventStats[addonName].update(duration);

                TrackEvent(addonName, lastEventCode, static_cast<double>(duration));
            }
        }
    }

    // SignalEvent hook
    void SignalEventHook(hadesmem::PatchDetourBase *detour, int eventCode) {
        auto const SignalEvent = detour->GetTrampolineT<SignalEventT>();

        gLastEventCode = eventCode;

        SignalEvent(eventCode);

        gLastEventCode = -1; // Reset after processing
    }


    // Original FrameScript_Execute function pointer
    FrameScript_ExecuteT pOriginalFrameScript_Execute = nullptr;

    __declspec(naked) void __cdecl FrameScript_ExecuteHook(int *param_1, int *param_2, char *param_3) {
        __asm {
            // Wipe gLastEventCode
                mov eax, -1
                mov gLastEventCode, eax

            // Jump to original function (tail call)
                jmp pOriginalFrameScript_Execute
        }
    }

    // Original SignalEventParam function pointer
    SignalEventParamT pOriginalSignalEventParam = nullptr;
    // Storage for original return addresses (two needed due to recursive calls)
    void *originalSignalEventParamReturnAddress1 = nullptr;
    void *originalSignalEventParamReturnAddress2 = nullptr;
    int signalEventParamNestingLevel = 0;

    // Helper function to log debug info safely
    void SignalEventParamEnd(int eventCode) {
        // Find and remove the start time for this event
        auto startTimeIt = gEventCodeStartTimes.find(eventCode);
        if (startTimeIt != gEventCodeStartTimes.end()) {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTimeIt->second);

            // Update statistics for this event code
            auto it = gEventCodeStats.find(eventCode);
            if (it == gEventCodeStats.end()) {
                // Create new entry with proper name
                std::string eventName = GetEventName(eventCode);
                gEventCodeStats[eventCode] = FunctionStats(eventName);
            }
            gEventCodeStats[eventCode].update(duration.count());

            // Remove the start time entry
            gEventCodeStartTimes.erase(startTimeIt);
        }

        // Reset the global event code after processing
        gLastEventCode = -1;
    }

    void SignalEventParamStart(int eventCode, char *formatString) {
        gLastEventCode = eventCode;

        // Ignore event code 392 (EXECUTE_CHAT_LINE)
        if (eventCode == 392) {
            return;
        }

        // Record start time for this event code
        gEventCodeStartTimes[eventCode] = std::chrono::high_resolution_clock::now();
    }

    // called after the original function returns
    __declspec(naked) void PostSignalEventParamHook() {
        __asm {
            // Save registers
                pushad
                pushfd

            // Call SignalEventParamEnd with the event code from the stack
            // The original function was called with: SignalEventParam(eventCode, formatString, ...)
            // Stack layout after pushad/pushfd: [eventCode][formatString][...][pushad 8*4][pushfd]
            // esp+36=eventCode
                push[esp+36]            // eventCode parameter
                call SignalEventParamEnd
                add esp, 4               // Clean up parameter

            // Restore registers
                popfd
                popad

            // Decrement nesting level and use LIFO approach
                dec signalEventParamNestingLevel
                mov eax, signalEventParamNestingLevel
                test eax, eax
                jz use_address1          // Level 0, use first address

            // Level 1, use second address
                mov eax, originalSignalEventParamReturnAddress2
                mov originalSignalEventParamReturnAddress2, 0  // Clear the slot
                jmp eax

                use_address1:
                mov eax, originalSignalEventParamReturnAddress1
                mov originalSignalEventParamReturnAddress1, 0  // Clear the slot
                jmp eax
        }
    }

    // Trampoline for SignalEventParamHook using return address replacement
    __declspec(naked) void __cdecl SignalEventParamHook(int eventCode, const char *formatString, ...) {
        __asm {
            // Save all registers to avoid corruption
                pushad
                pushfd

            // Call our SignalEventParam function with parameters
            // Stack: [ret][eventCode][formatString][...][pushad 8*4][pushfd]
            // esp+36=ret, esp+40=eventCode, esp+44=formatString
                push[esp+44]            // formatString parameter
                push[esp+44]            // eventCode parameter (offset stays same after first push)
                call SignalEventParamStart
                add esp, 8               // Clean up parameters

            // Restore flags and registers
                popfd
                popad

            // Store original return address from stack using nesting level
                mov eax, signalEventParamNestingLevel
                test eax, eax
                jnz use_slot2            // Already nested, use second slot

            // Level 0, use first slot
                push[esp]               // Copy return address to top of stack
                pop originalSignalEventParamReturnAddress1 // Pop it into our variable
                jmp set_hook_return

                use_slot2:
            // Level 1, use second slot  
                push[esp]               // Copy return address to top of stack
                pop originalSignalEventParamReturnAddress2 // Pop it into our variable

                set_hook_return:
            // Increment nesting level
                inc signalEventParamNestingLevel

            // Replace return address on stack with our epilogue
                mov eax, offset PostSignalEventParamHook
                mov[esp], eax

            // Jump to original function (tail call)
                jmp pOriginalSignalEventParam
        }
    }

    void loadConfig() {
        gStartTime = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count());

        // remove/rename previous logs
        remove("perf_monitor.log.3");
        rename("perf_monitor.log.2", "perf_monitor.log.3");
        rename("perf_monitor.log.1", "perf_monitor.log.2");
        rename("perf_monitor.log", "perf_monitor.log.1");

        // open new log file
        debugLogFile.open("perf_monitor.log");

        DEBUG_LOG("Loading perf_monitor");

        // Initialize event stats
        initializeEventStats();

        // Initialize last event stats time
        gLastEventStatsTime = 0;
    }

    // Template function to simplify hook initialization with dynamic storage
    template<typename FuncT, typename HookT>
    void initializeHook(const hadesmem::Process &process, Offsets offset, HookT hookFunc) {
        auto const originalFunc = hadesmem::detail::AliasCast<FuncT>(offset);
        auto detour = std::make_unique<hadesmem::PatchDetour<FuncT>>(process, originalFunc, hookFunc);
        detour->Apply();
        gDetours.push_back(std::move(detour));
    }

    void initHooks() {
        const hadesmem::Process process(::GetCurrentProcessId());

        // Hook RenderWorld
        initializeHook<FastcallFrameT>(process, Offsets::RenderWorld, &RenderWorldHook);

        // Hook OnWorldRender
        initializeHook<FastcallFrameT>(process, Offsets::OnWorldRender, &OnWorldRenderHook);

        // Hook OnWorldUpdate
        initializeHook<FastcallFrameT>(process, Offsets::OnWorldUpdate, &OnWorldUpdateHook);

        // Hook CWorldRender
        initializeHook<StdcallT>(process, Offsets::CWorldRender, &CWorldRenderHook);

        // Hook CWorldUpdate
        initializeHook<WorldUpdateT>(process, Offsets::CWorldUpdate, &CWorldUpdateHook);

        // Hook SpellVisualsRender
        initializeHook<StdcallT>(process, Offsets::SpellVisualsRender, &SpellVisualsRenderHook);

        // Hook SpellVisualsTick
        initializeHook<StdcallT>(process, Offsets::SpellVisualsTick, &SpellVisualsTickHook);

        // Hook UnitUpdate
        initializeHook<FastcallFrameT>(process, Offsets::CGWorldFrameUnitUpdate, &UnitUpdateHook);

        // Hook FrameOnLayerUpdate
        initializeHook<FrameOnLayerUpdateT>(process, Offsets::CSimpleFrameOnLayerUpdate, &FrameOnLayerUpdateHook);

        // Hook CWorldSceneRender
        initializeHook<StdcallT>(process, Offsets::CWorldSceneRender, &CWorldSceneRenderHook);

        // Hook CWorldUnknownRender
        initializeHook<StdcallT>(process, Offsets::CWorldUnknownRender, &CWorldUnknownRenderHook);

        // Hook CSimpleFrameOnFrameRender1
        initializeHook<FrameBatchT>(process, Offsets::CSimpleFrameOnFrameRender1, &CSimpleFrameOnFrameRender1Hook);

        // Hook CSimpleFrameOnFrameRender2
        initializeHook<FastcallFrameT>(process, Offsets::CSimpleFrameOnFrameRender2, &CSimpleFrameOnFrameRender2Hook);

        // Hook CSimpleModelOnFrameRender
        initializeHook<FrameBatchT>(process, Offsets::CSimpleModelOnFrameRender, &CSimpleModelOnFrameRenderHook);

        // Hook FrameScriptObjectOnScriptEvent
        initializeHook<FrameOnScriptEventT>(process, Offsets::FrameScriptObjectOnScriptEvent, &FrameOnScriptEventHook);

        // Hook CSimpleTopOnLayerUpdate
        initializeHook<FrameOnLayerUpdateT>(process, Offsets::CSimpleTopOnLayerUpdate, &CSimpleTopOnLayerUpdateHook);

        // Hook CSimpleTopOnLayerRender
        initializeHook<FastcallFrameT>(process, Offsets::CSimpleTopOnLayerRender, &CSimpleTopOnLayerRenderHook);

        // Hook FrameOnScriptEventParam
        initializeHook<FrameOnScriptEventParamT>(process, Offsets::FrameScriptObjectOnScriptEventParam,
                                                 &FrameOnScriptEventParamHook);

        // Hook IEvtQueueDispatch
        initializeHook<IEvtQueueDispatchT>(process, Offsets::IEvtQueueDispatch, &IEvtQueueDispatchHook);

        // Hook SignalEvent
        initializeHook<SignalEventT>(process, Offsets::SignalEvent, &SignalEventHook);

        // Hook SignalEventParam using Microsoft Detours
        pOriginalSignalEventParam = reinterpret_cast<SignalEventParamT>(Offsets::SignalEventParam);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID &) pOriginalSignalEventParam, SignalEventParamHook);
        DetourTransactionCommit();

        // Hook FrameScript_Execute using Microsoft Detours
        pOriginalFrameScript_Execute = reinterpret_cast<FrameScript_ExecuteT>(Offsets::FrameScript_Execute);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID &) pOriginalFrameScript_Execute, FrameScript_ExecuteHook);
        DetourTransactionCommit();
    }

    void SpellVisualsInitializeHook(hadesmem::PatchDetourBase *detour) {
        auto const spellVisualsInitialize = detour->GetTrampolineT<SpellVisualsInitializeT>();
        spellVisualsInitialize();
        loadConfig();
        initHooks();
    }

    std::once_flag loadFlag;

    void load() {
        std::call_once(loadFlag, []() {
                           // hook spell visuals initialize
                           const hadesmem::Process process(::GetCurrentProcessId());

                           auto const spellVisualsInitOrig = hadesmem::detail::AliasCast<SpellVisualsInitializeT>(
                                   Offsets::SpellVisualsInitialize);
                           gSpellVisualsInitDetour = std::make_unique<hadesmem::PatchDetour<SpellVisualsInitializeT>>(process,
                                                                                                                      spellVisualsInitOrig,
                                                                                                                      &SpellVisualsInitializeHook);
                           gSpellVisualsInitDetour->Apply();
                       }
        );
    }
}

extern "C" __declspec(dllexport) uint32_t Load() {
    perf_monitor::load();
    return EXIT_SUCCESS;
}
