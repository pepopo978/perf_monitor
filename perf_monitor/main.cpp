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

#include <cstdint>
#include <memory>
#include <atomic>

#include <chrono>
#include <iostream>
#include <deque>
#include <iomanip>

BOOL WINAPI DllMain(HINSTANCE, uint32_t, void *);

namespace perf_monitor {
    std::unique_ptr<hadesmem::PatchDetour<ISceneEndT>> gIEndSceneDetour;
    std::unique_ptr<hadesmem::PatchDetour<SpellVisualsInitializeT >> gSpellVisualsInitDetour;
    std::unique_ptr<hadesmem::PatchDetour<OnWorldRenderT>> gOnWorldRenderDetour;

    auto gAverageFrameTime = 0.0;
    auto gNumFrames = 0;
    auto gSlowestFrameTime = 0LL;
    auto gFastestFrameTime = 999999999LL; // use a large constant

    // For 1-minute stats
    std::deque<std::pair<uint64_t, long long>> gFrameTimes1Min; // (timestamp_ms, frame_time_us)
    long long gSlowestFrameTime1Min = 0;
    long long gFastestFrameTime1Min = 999999999LL; // use a large constant

    uint64_t gLastStatsOutputTime = 0;

    uint32_t GetTime() {
        return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count()) - gStartTime;
    }

    uint64_t GetWowTimeMs() {
        auto const osGetAsyncTimeMs = reinterpret_cast<GetTimeMsT>(Offsets::OsGetAsyncTimeMs);
        return osGetAsyncTimeMs();
    }

    uintptr_t* GetLuaStatePtr() {
        typedef uintptr_t* (__fastcall* GETCONTEXT)(void);
        static auto p_GetContext = reinterpret_cast<GETCONTEXT>(0x7040D0);
        return p_GetContext();
    }

    void OutputStats()
    {
        // Overall stats
        double avgFrameTime = gNumFrames > 0 ? gAverageFrameTime / gNumFrames : 0.0;
        // 1-min stats
        double avgFrameTime1Min = 0.0;
        if (!gFrameTimes1Min.empty()) {
            long long sum = 0;
            gSlowestFrameTime1Min = 0;
            gFastestFrameTime1Min = 999999999LL;
            for (const auto& p : gFrameTimes1Min) {
                sum += p.second;
                if (p.second > gSlowestFrameTime1Min) gSlowestFrameTime1Min = p.second;
                if (p.second < gFastestFrameTime1Min) gFastestFrameTime1Min = p.second;
            }
            avgFrameTime1Min = static_cast<double>(sum) / gFrameTimes1Min.size();
        }

        DEBUG_LOG(
            std::fixed << std::setprecision(2)
            << "[Overall] Frames: " << gNumFrames
            << ", Avg: " << avgFrameTime / 1000.0 << " ms"
            << ", Slowest: " << gSlowestFrameTime / 1000.0 << " ms"
            << ", Fastest: " << (gFastestFrameTime == 999999999LL ? 0 : gFastestFrameTime / 1000.0) << " ms"
        );
        DEBUG_LOG(
            std::fixed << std::setprecision(2)
            << "[Last 1 min] Frames: " << gFrameTimes1Min.size()
            << ", Avg: " << avgFrameTime1Min / 1000.0 << " ms"
            << ", Slowest: " << gSlowestFrameTime1Min / 1000.0 << " ms"
            << ", Fastest: " << (gFastestFrameTime1Min == 999999999LL ? 0 : gFastestFrameTime1Min / 1000.0) << " ms"
        );
    }

    void OnWorldRenderHook(hadesmem::PatchDetourBase *detour, uintptr_t *worldFrame) {
        auto const onWorldRender = detour->GetTrampolineT<OnWorldRenderT>();

        auto start = std::chrono::high_resolution_clock::now();
        onWorldRender(worldFrame);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Update overall stats
        gAverageFrameTime += duration;
        ++gNumFrames;
        if (duration > gSlowestFrameTime) gSlowestFrameTime = duration;
        if (duration < gFastestFrameTime) gFastestFrameTime = duration;

        // Update 1-min stats
        uint64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(end.time_since_epoch()).count();
        gFrameTimes1Min.emplace_back(nowMs, duration);

        // Remove frames older than 1 minute
        while (!gFrameTimes1Min.empty() && nowMs - gFrameTimes1Min.front().first > 60000) {
            gFrameTimes1Min.pop_front();
        }

        // Output stats every minute
        if (gLastStatsOutputTime == 0 || nowMs - gLastStatsOutputTime >= 60000) {
            OutputStats();
            gLastStatsOutputTime = nowMs;
        }

        // DEBUG_LOG("OnWorldRender took " << duration << " ms"); // Commented out per requirements
    }

    void loadConfig() {
        gStartTime = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count());

        // open new log file
        debugLogFile.open("perf_monitor.log");

        DEBUG_LOG("Loading perf_monitor");
    }

    void initHooks() {
        const hadesmem::Process process(::GetCurrentProcessId());

        auto const onWorldRenderOrig = hadesmem::detail::AliasCast<OnWorldRenderT>(Offsets::OnWorldRender);
        gOnWorldRenderDetour = std::make_unique<hadesmem::PatchDetour<OnWorldRenderT>>(
                process, onWorldRenderOrig, &OnWorldRenderHook);
        gOnWorldRenderDetour->Apply();
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
                           gSpellVisualsInitDetour = std::make_unique<hadesmem::PatchDetour<SpellVisualsInitializeT >>(process,
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
