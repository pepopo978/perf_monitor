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

BOOL WINAPI DllMain(HINSTANCE, uint32_t, void *);

namespace perf_monitor {

    // Dynamic detour storage system
    std::vector<std::unique_ptr<hadesmem::PatchDetourBase>> gDetours;

    // Keep SpellVisualsInitDetour as it's used specifically in DllMain
    std::unique_ptr<hadesmem::PatchDetour<SpellVisualsInitializeT >> gSpellVisualsInitDetour;

    // Track 5 slowest events for each addon
    std::map<std::string, std::vector<EventStats>> gAddonEventStats;

    // Track addon OnUpdate performance
    std::map<std::string, FunctionStats> gAddonOnUpdateStats;


    // Track spell visual performance by spell ID
    std::map<uint32_t, FunctionStats> gSpellVisualStatsById;

    // Track addon memory usage for OnEvent and OnUpdate
    std::map<std::string, MemoryStats> gAddonOnEventMemoryStats;
    std::map<std::string, MemoryStats> gAddonOnUpdateMemoryStats;

    // Early alphabetical Ace addons that are likely to get associated with other addons events
    std::set<std::string> gAceAddonBlacklist = {
            "pfUI",
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

    // Track when we're drawing the world scene
    bool gIsDrawingWorldScene = false;

    // Scene draw loop timing variables
    std::chrono::high_resolution_clock::time_point gSceneDrawLoopStartTime;

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
        static auto p_GetContext = reinterpret_cast<GETCONTEXT>(Offsets::lua_getcontext);
        return p_GetContext();
    }

    // Helper function to get current Lua memory usage in KB
    int GetLuaMemoryKB() {
        auto const lua_getgccount = reinterpret_cast<lua_getgccountT>(Offsets::lua_getgccount);

        uintptr_t *luaState = GetLuaStatePtr();
        if (luaState != nullptr) {
            return lua_getgccount(luaState);
        }
        return 0;
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
        if (framescriptObj != nullptr && IsBadReadPtr(framescriptObj, sizeof(uintptr_t) * 39) == 0 &&
            framescriptObj[38] != 0) {
            auto frameName = reinterpret_cast<char *>(framescriptObj[38]);
            if (frameName != nullptr && IsValidAsciiString(frameName)) {
                frameNameStr = std::string(frameName);

                // comment if you want to see individual pfui frame performance
//                if (frameNameStr.length() >= 2 && frameNameStr.substr(0, 2) == "pf") {
//                    // Special handling for pfUI
//                    frameNameStr = "pfUI";
//                }

                // Ignore frame names ending with .dll
                if (frameNameStr.length() >= 4 && frameNameStr.substr(frameNameStr.length() - 4) == ".dll") {
                    frameNameStr = "";
                } else if (frameNameStr.length() >= 7 && frameNameStr.substr(0, 7) == "Cursive") {
                    frameNameStr = "Cursive";
                } else if (frameNameStr.length() >= 7 && frameNameStr.substr(0, 7) == "BigWigs") {
                    frameNameStr = "BigWigs";
                } else if ((frameNameStr.length() >= 4 && frameNameStr.substr(0, 4) == "MSBT") ||
                           (frameNameStr.length() >= 4 && frameNameStr.substr(0, 4) == "MCEH")) {
                    frameNameStr = "MSBT";
                } else if (frameNameStr.length() >= 13 && frameNameStr.substr(0, 9) == "Character" &&
                           frameNameStr.substr(frameNameStr.length() - 4) == "Slot") {
                    frameNameStr = "CharacterSlot";
                } else if (frameNameStr.length() >= 11 && frameNameStr.substr(0, 7) == "Inspect" &&
                           frameNameStr.substr(frameNameStr.length() - 4) == "Slot") {
                    frameNameStr = "InspectSlot";
                } else if (frameNameStr.length() >= 8 && frameNameStr.substr(0, 8) == "RABFrame") {
                    frameNameStr = "Rabuffs";
                } else {
                    // Strip numbers from frame name
                    frameNameStr.erase(std::remove_if(frameNameStr.begin(), frameNameStr.end(), ::isdigit),
                                       frameNameStr.end());
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
            uint64_t previousPeriodStart = gRenderWorldStats.periodStartTime;
            uint64_t previousPeriodEnd = nowMs;
            OutputStats(previousPeriodStart, previousPeriodEnd);
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

    typedef enum OBJECT_TYPE_ID {
        ID_OBJECT = 0,
        ID_ITEM = 1,
        ID_CONTAINER = 2,
        ID_UNIT = 3,
        ID_PLAYER = 4,
        ID_GAMEOBJECT = 5,
        ID_DYNAMICOBJECT = 6,
        ID_CORPSE = 7,
        ID_AIGROUP = 8,
        NUM_CLIENT_OBJECT_TYPES = 8,
        ID_AREATRIGGER = 9,
        NUM_OBJECT_TYPES = 10
    } OBJECT_TYPE_ID;

    // ObjectUpdateHandler hook
    int ObjectUpdateHandlerHook(hadesmem::PatchDetourBase *detour, uintptr_t *param_1, CDataStore *dataStore) {
        auto const ObjectUpdateHandler = detour->GetTrampolineT<PacketHandlerT>();
        auto start = std::chrono::high_resolution_clock::now();
        auto result = ObjectUpdateHandler(param_1, dataStore);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gObjectUpdateHandlerStats.update(duration);

        return result;
    }

    // PlaySpellVisual hook
    void PlaySpellVisualHook(hadesmem::PatchDetourBase *detour, uintptr_t *unit, uintptr_t *unk, uintptr_t *spellRec,
                             uintptr_t *visualKit, void *param_3, void *param_4) {
        auto const PlaySpellVisual = detour->GetTrampolineT<PlaySpellVisualT>();
        auto start = std::chrono::high_resolution_clock::now();

        auto spellId = spellRec ? spellRec[0] : 0;

        PlaySpellVisual(unit, unk, spellRec, visualKit, param_3, param_4);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update overall stats
        gPlaySpellVisualStats.update(duration);

        // Update spell-specific stats
        if (spellId != 0) {
            if (gSpellVisualStatsById.find(spellId) == gSpellVisualStatsById.end()) {
                std::string spellName = "Spell ID " + std::to_string(spellId);
                gSpellVisualStatsById[spellId] = FunctionStats(spellName);
            }
            gSpellVisualStatsById[spellId].update(duration);
        }
    }

    // UnknownOnRender1 hook
    void UnknownOnRender1Hook(hadesmem::PatchDetourBase *detour) {
        auto const UnknownOnRender1 = detour->GetTrampolineT<UnknownOnRender1T>();
        auto start = std::chrono::high_resolution_clock::now();
        UnknownOnRender1();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gUnknownOnRender1Stats.update(duration);
    }

    // UnknownOnRender2 hook
    void UnknownOnRender2Hook(hadesmem::PatchDetourBase *detour) {
        auto const UnknownOnRender2 = detour->GetTrampolineT<UnknownOnRender2T>();
        auto start = std::chrono::high_resolution_clock::now();
        UnknownOnRender2();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gUnknownOnRender2Stats.update(duration);
    }

    // UnknownOnRender3 hook
    void UnknownOnRender3Hook(hadesmem::PatchDetourBase *detour) {
        auto const UnknownOnRender3 = detour->GetTrampolineT<UnknownOnRender3T>();
        auto start = std::chrono::high_resolution_clock::now();
        UnknownOnRender3();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gUnknownOnRender3Stats.update(duration);
    }

    // CM2Scene::AdvanceTime hook - only track performance if this pointer equals Offsets::ActiveWorldScene
    void
    CM2SceneAdvanceTimeHook(hadesmem::PatchDetourBase *detour, uintptr_t *this_ptr, void *dummy_edx, uint32_t param_1) {
        auto const CM2SceneAdvanceTime = detour->GetTrampolineT<CM2SceneAdvanceTimeT>();

        auto worldScenePtr = *reinterpret_cast<uintptr_t **>(0x00c7b298);

        // Check if this pointer matches the specific address
        if (this_ptr == worldScenePtr) {
            auto start = std::chrono::high_resolution_clock::now();
            CM2SceneAdvanceTime(this_ptr, dummy_edx, param_1);
            auto end = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            // Update stats without outputting
            gCM2SceneAdvanceTimeStats.update(duration);
        } else {
            // Call original function without timing
            CM2SceneAdvanceTime(this_ptr, dummy_edx, param_1);
        }
    }

    // CM2Scene::Animate hook - only track performance if this pointer equals Offsets::ActiveWorldScene
    void CM2SceneAnimateHook(hadesmem::PatchDetourBase *detour, uintptr_t *this_ptr, void *dummy_edx, float *param_1) {
        auto const CM2SceneAnimate = detour->GetTrampolineT<CM2SceneAnimateT>();

        auto worldScenePtr = *reinterpret_cast<uintptr_t **>(0x00c7b298);

        // Check if this pointer matches the specific address
        if (this_ptr == worldScenePtr) {
            auto start = std::chrono::high_resolution_clock::now();
            CM2SceneAnimate(this_ptr, dummy_edx, param_1);
            auto end = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            // Update stats without outputting
            gCM2SceneAnimateStats.update(duration);
        } else {
            // Call original function without timing
            CM2SceneAnimate(this_ptr, dummy_edx, param_1);
        }
    }

    // CM2Scene::Draw hook - only track performance if this pointer equals Offsets::ActiveWorldScene
    void CM2SceneDrawHook(hadesmem::PatchDetourBase *detour, uintptr_t *this_ptr, void *dummy_edx, int param_1) {
        auto const CM2SceneDraw = detour->GetTrampolineT<CM2SceneDrawT>();

        auto worldScenePtr = *reinterpret_cast<uintptr_t **>(0x00c7b298);

        // Check if this pointer matches the specific address
        if (this_ptr == worldScenePtr) {
            gIsDrawingWorldScene = true;
            auto start = std::chrono::high_resolution_clock::now();
            CM2SceneDraw(this_ptr, dummy_edx, param_1);
            auto end = std::chrono::high_resolution_clock::now();
            gIsDrawingWorldScene = false;

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            // Update stats without outputting
            gCM2SceneDrawStats.update(duration);
        } else {
            // Call original function without timing
            CM2SceneDraw(this_ptr, dummy_edx, param_1);
        }
    }

    // DrawBatchProj hook
    void DrawBatchProjHook(hadesmem::PatchDetourBase *detour, uintptr_t *this_ptr, void *dummy_edx) {
        auto const DrawBatchProj = detour->GetTrampolineT<DrawBatchProjT>();
        auto start = std::chrono::high_resolution_clock::now();
        DrawBatchProj(this_ptr, dummy_edx);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        gDrawBatchProjStats.update(duration);
    }

    // DrawBatch hook
    void DrawBatchHook(hadesmem::PatchDetourBase *detour, uintptr_t *this_ptr, void *dummy_edx) {
        auto const DrawBatch = detour->GetTrampolineT<DrawBatchT>();
        auto start = std::chrono::high_resolution_clock::now();
        DrawBatch(this_ptr, dummy_edx);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        gDrawBatchStats.update(duration);
    }

    // DrawBatchDoodad hook
    void DrawBatchDoodadHook(hadesmem::PatchDetourBase *detour, uintptr_t *this_ptr, void *dummy_edx, int param_1,
                             int param_2) {
        auto const DrawBatchDoodad = detour->GetTrampolineT<DrawBatchDoodadT>();
        auto start = std::chrono::high_resolution_clock::now();
        DrawBatchDoodad(this_ptr, dummy_edx, param_1, param_2);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        gDrawBatchDoodadStats.update(duration);
    }

    // DrawRibbon hook
    void DrawRibbonHook(hadesmem::PatchDetourBase *detour, uintptr_t *this_ptr, void *dummy_edx) {
        auto const DrawRibbon = detour->GetTrampolineT<DrawRibbonT>();
        auto start = std::chrono::high_resolution_clock::now();
        DrawRibbon(this_ptr, dummy_edx);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        gDrawRibbonStats.update(duration);
    }

    // DrawParticle hook
    void DrawParticleHook(hadesmem::PatchDetourBase *detour, uintptr_t *this_ptr, void *dummy_edx) {
        auto const DrawParticle = detour->GetTrampolineT<DrawParticleT>();
        auto start = std::chrono::high_resolution_clock::now();
        DrawParticle(this_ptr, dummy_edx);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        gDrawParticleStats.update(duration);
    }

    // DrawCallback hook
    void DrawCallbackHook(hadesmem::PatchDetourBase *detour, uintptr_t *this_ptr, void *dummy_edx) {
        auto const DrawCallback = detour->GetTrampolineT<DrawCallbackT>();
        auto start = std::chrono::high_resolution_clock::now();
        DrawCallback(this_ptr, dummy_edx);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        gDrawCallbackStats.update(duration);
    }

    // CM2SceneRender::Draw hook - only track stats when drawing world scene
    void
    CM2SceneRenderDrawHook(hadesmem::PatchDetourBase *detour, uintptr_t *this_ptr, void *dummy_edx, uint32_t param_1,
                           int param_2, int param_3, uint32_t param_4) {
        auto const CM2SceneRenderDraw = detour->GetTrampolineT<CM2SceneRenderDrawT>();

        // Only track performance when drawing world scene
        if (gIsDrawingWorldScene) {
            auto start = std::chrono::high_resolution_clock::now();
            CM2SceneRenderDraw(this_ptr, dummy_edx, param_1, param_2, param_3, param_4);
            auto end = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            gCM2SceneRenderDrawStats.update(duration);
        } else {
            // Call original function without timing when not drawing world scene
            CM2SceneRenderDraw(this_ptr, dummy_edx, param_1, param_2, param_3, param_4);
        }
    }

    // luaC_collectgarbage hook
    void luaC_collectgarbageHook(hadesmem::PatchDetourBase *detour, int param_1) {
        auto const luaC_collectgarbage = detour->GetTrampolineT<luaC_collectgarbageT>();
        auto start = std::chrono::high_resolution_clock::now();
        luaC_collectgarbage(param_1);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        gLuaCCollectgarbageStats.update(duration);
    }


    void CM2ModelAnimateMTHook(hadesmem::PatchDetourBase *detour, uintptr_t *this_ptr, void *dummy_edx, float *param_1,
                               float *param_2, float *param_3, float *param_4) {

        auto const CM2ModelAnimateMT = detour->GetTrampolineT<CM2ModelAnimateMTT>();
        auto start = std::chrono::high_resolution_clock::now();
        CM2ModelAnimateMT(this_ptr, dummy_edx, param_1, param_2, param_3, param_4);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        gCM2ModelAnimateMTStats.update(duration);
    }

    void ObjectFreeHook(hadesmem::PatchDetourBase *detour, int param_1, uint32_t param_2) {
        auto const ObjectFree = detour->GetTrampolineT<ObjectFreeT>();
        auto start = std::chrono::high_resolution_clock::now();
        ObjectFree(param_1, param_2);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        gObjectFreeStats.update(duration);
    }


    // PaintScreen hook
    void PaintScreenHook(hadesmem::PatchDetourBase *detour, uint32_t param_1, uint32_t param_2) {
        auto const PaintScreen = detour->GetTrampolineT<PaintScreenT>();
        auto start = std::chrono::high_resolution_clock::now();
        PaintScreen(param_1, param_2);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update stats without outputting
        gPaintScreenStats.update(duration);
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
                // Get memory before OnUpdate
                int memoryBefore = GetLuaMemoryKB();

                auto start = std::chrono::high_resolution_clock::now();
                FrameOnLayerUpdate(frame, unk, unk2);
                auto end = std::chrono::high_resolution_clock::now();

                // Get memory after OnUpdate
                int memoryAfter = GetLuaMemoryKB();
                int memoryDelta = memoryAfter - memoryBefore;

                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

                // Update overall stats
                gFrameOnLayerUpdateStats.update(duration);

                // Update addon-specific stats
                if (gAddonOnUpdateStats.find(addonName) == gAddonOnUpdateStats.end()) {
                    gAddonOnUpdateStats[addonName] = FunctionStats(addonName + " OnUpdate");
                }
                gAddonOnUpdateStats[addonName].update(duration);

                // Update memory stats for OnUpdate
                if (gAddonOnUpdateMemoryStats.find(addonName) == gAddonOnUpdateMemoryStats.end()) {
                    gAddonOnUpdateMemoryStats[addonName] = MemoryStats(addonName + " OnUpdate Memory");
                }
                gAddonOnUpdateMemoryStats[addonName].update(memoryDelta);
            }
        }
    }


    void FrameOnScriptEventHook(hadesmem::PatchDetourBase *detour, int *param_1, int *param_2) {
        auto const FrameScriptObjectOnScriptEvent = detour->GetTrampolineT<FrameOnScriptEventT>();
        auto lastEventCode = gLastEventCode;

        // Get memory before event
        int memoryBefore = GetLuaMemoryKB();

        auto start = std::chrono::high_resolution_clock::now();
        FrameScriptObjectOnScriptEvent(param_1, param_2);
        auto end = std::chrono::high_resolution_clock::now();

        // Get memory after event
        int memoryAfter = GetLuaMemoryKB();
        int memoryDelta = memoryAfter - memoryBefore;

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

                // Update memory stats for OnEvent
                if (gAddonOnEventMemoryStats.find(addonName) == gAddonOnEventMemoryStats.end()) {
                    gAddonOnEventMemoryStats[addonName] = MemoryStats(addonName + " OnEvent Memory");
                }

                gAddonOnEventMemoryStats[addonName].update(memoryDelta);
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

        // Get memory before event
        int memoryBefore = GetLuaMemoryKB();

        auto start = std::chrono::high_resolution_clock::now();
        FrameOnScriptEventParam(framescriptObj, param_2, param_3, args);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Get memory after event
        int memoryAfter = GetLuaMemoryKB();
        int memoryDelta = memoryAfter - memoryBefore;

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

                // Update memory stats for OnEvent
                if (gAddonOnEventMemoryStats.find(addonName) == gAddonOnEventMemoryStats.end()) {
                    gAddonOnEventMemoryStats[addonName] = MemoryStats(addonName + " OnEvent Memory");
                }

                gAddonOnEventMemoryStats[addonName].update(memoryDelta);
            }
        }
    }

    // Helper function to log debug info safely
    void SignalEventEnd(int eventCode) {
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

    // SignalEvent hook
    void SignalEventHook(hadesmem::PatchDetourBase *detour, int eventCode) {
        auto const SignalEvent = detour->GetTrampolineT<SignalEventT>();

        gLastEventCode = eventCode;
        gEventCodeStartTimes[eventCode] = std::chrono::high_resolution_clock::now();

        SignalEvent(eventCode);

        SignalEventEnd(eventCode);
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
    // Dynamic stack for return addresses to handle arbitrary nesting levels
    void *signalEventParamReturnAddressStack[32]; // Fixed size array for inline assembly compatibility
    int signalEventParamStackTop = 0;
    void *tempReturnAddress = nullptr; // Temporary storage for return address during register restoration

    // Helper functions for managing the return address stack
    void PushReturnAddress(void *address) {
        if (signalEventParamStackTop < 32) {
            signalEventParamReturnAddressStack[signalEventParamStackTop++] = address;
        }
    }

    void *PopReturnAddress() {
        if (signalEventParamStackTop <= 0) {
            DEBUG_LOG("ERROR: Trying to pop from empty return address stack!");
            return nullptr;
        }
        void *address = signalEventParamReturnAddressStack[--signalEventParamStackTop];
        return address;
    }

    void SignalEventParamStart(int eventCode, char *formatString) {
        gLastEventCode = eventCode;

        // Record start time for this event code
        gEventCodeStartTimes[eventCode] = std::chrono::high_resolution_clock::now();
    }

    // called after the original function returns
    __declspec(naked) void PostSignalEventParamHook() {
        __asm {
            // Save registers
                pushad
                pushfd

            // Call SignalEventEnd with the event code from the stack
            // The original function was called with: SignalEventParam(eventCode, formatString, ...)
            // Stack layout after pushad/pushfd: [eventCode][formatString][...][pushad 8*4][pushfd]
            // esp+36=eventCode
                push[esp+36]            // eventCode parameter
                call SignalEventEnd
                add esp, 4               // Clean up parameter

            // Call PopReturnAddress to get the return address from our stack
                call PopReturnAddress
                test eax, eax           // Check if we got a valid address
                jz error_exit           // If null, something went wrong

            // Store return address in global variable
                mov tempReturnAddress, eax

            // Restore registers
                popfd
                popad

            // Jump to the original return address from global variable
                jmp tempReturnAddress

                error_exit:
            // Restore registers even on error
                popfd
                popad
                ret                     // Just return normally if stack is corrupted
        }
    }

    // Trampoline for SignalEventParamHook using return address replacement
    __declspec(naked) void __cdecl SignalEventParamHook(int eventCode, const char *formatString, ...) {
        __asm {
            // Save all registers to avoid corruption
                pushad
                pushfd

            // Call our SignalEventParamStart function with parameters
            // Stack: [ret][eventCode][formatString][...][pushad 8*4][pushfd]
            // esp+36=ret, esp+40=eventCode, esp+44=formatString
                push[esp+44]            // formatString parameter
                push[esp+44]            // eventCode parameter (offset stays same after first push)
                call SignalEventParamStart
                add esp, 8               // Clean up parameters

            // Restore flags and registers first
                popfd
                popad

            // Store original return address from stack by pushing it to our dynamic stack
            // Call PushReturnAddress with the return address
                push[esp]               // Push the return address from stack top
                call PushReturnAddress
                add esp, 4               // Clean up parameter

            // Replace return address on stack with PostSignalEventParamHook
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

        // Hook PlaySpellVisual - DISABLED wasn't super useful.  doesn't capture the true performance cost of some spells
//        initializeHook<PlaySpellVisualT>(process, Offsets::PlaySpellVisual, &PlaySpellVisualHook);

        // Hook UnitUpdate
        initializeHook<FastcallFrameT>(process, Offsets::CGWorldFrameUnitUpdate, &UnitUpdateHook);

        // Hook UnknownOnRender1
        initializeHook<UnknownOnRender1T>(process, Offsets::UnknownOnRender1, &UnknownOnRender1Hook);

        // Hook UnknownOnRender2
        initializeHook<UnknownOnRender2T>(process, Offsets::UnknownOnRender2, &UnknownOnRender2Hook);

        // Hook UnknownOnRender3
        initializeHook<UnknownOnRender3T>(process, Offsets::UnknownOnRender3, &UnknownOnRender3Hook);

        // Hook CM2Scene::AdvanceTime
        initializeHook<CM2SceneAdvanceTimeT>(process, Offsets::CM2SceneAdvanceTime, &CM2SceneAdvanceTimeHook);

        // Hook CM2Scene::Animate
        initializeHook<CM2SceneAnimateT>(process, Offsets::CM2SceneAnimate, &CM2SceneAnimateHook);

        // Hook CM2Scene::Draw
        initializeHook<CM2SceneDrawT>(process, Offsets::CM2SceneDraw, &CM2SceneDrawHook);

        // Hook CM2SceneRender functions
        initializeHook<DrawBatchProjT>(process, Offsets::DrawBatchProj, &DrawBatchProjHook);
        initializeHook<DrawBatchT>(process, Offsets::DrawBatch, &DrawBatchHook);
        initializeHook<DrawBatchDoodadT>(process, Offsets::DrawBatchDoodad, &DrawBatchDoodadHook);
        initializeHook<DrawRibbonT>(process, Offsets::DrawRibbon, &DrawRibbonHook);
        initializeHook<DrawParticleT>(process, Offsets::DrawParticle, &DrawParticleHook);
        initializeHook<DrawCallbackT>(process, Offsets::DrawCallback, &DrawCallbackHook);
        initializeHook<CM2SceneRenderDrawT>(process, Offsets::CM2SceneRenderDraw, &CM2SceneRenderDrawHook);
        // Hook CM2ModelAnimateMT
        initializeHook<CM2ModelAnimateMTT>(process, Offsets::CM2ModelAnimateMT, &CM2ModelAnimateMTHook);

        // Hook ObjectFree
        initializeHook<ObjectFreeT>(process, Offsets::ObjectFree, &ObjectFreeHook);

        // Hook ObjectUpdateHandler
        initializeHook<PacketHandlerT>(process, Offsets::ObjectUpdateHandler, &ObjectUpdateHandlerHook);

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

        // Hook PaintScreen
        initializeHook<PaintScreenT>(process, Offsets::PaintScreen, &PaintScreenHook);

        // Hook FrameOnScriptEventParam
        initializeHook<FrameOnScriptEventParamT>(process, Offsets::FrameScriptObjectOnScriptEventParam,
                                                 &FrameOnScriptEventParamHook);

        // Hook IEvtQueueDispatch
        initializeHook<IEvtQueueDispatchT>(process, Offsets::IEvtQueueDispatch, &IEvtQueueDispatchHook);

        // Hook SignalEvent
        initializeHook<SignalEventT>(process, Offsets::SignalEvent, &SignalEventHook);

        // Hook luaC_collectgarbage
        initializeHook<luaC_collectgarbageT>(process, Offsets::luaC_collectgarbage, &luaC_collectgarbageHook);

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
