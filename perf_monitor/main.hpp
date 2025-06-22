//
// Created by pmacc on 9/21/2024.
//

#pragma once

#include <hadesmem/process.hpp>
#include <hadesmem/patcher.hpp>

#include <Windows.h>

#include <cstdint>
#include <memory>
#include <atomic>

#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>

namespace perf_monitor {
    using ISceneEndT = int *(__fastcall *)(uintptr_t *unk);
    using EndSceneT = int (__fastcall *)(uintptr_t *unk);
    using GetTimeMsT = uint64_t (__stdcall *)();

    using OnWorldRenderT = void (__fastcall *)(uintptr_t *worldFrame);

    using GetClientConnectionT = uintptr_t *(__stdcall *)();
    using GetNetStatsT = void (__thiscall *)(uintptr_t *connection, float *param_1, float *param_2, uint32_t *param_3);

    using LoadScriptFunctionsT = void (__stdcall *)();
    using FrameScript_RegisterFunctionT = void (__fastcall *)(char *name, uintptr_t *func);
    using FrameScript_CreateEventsT = void (__fastcall *)(int param_1, uint32_t maxEventId);

    using LuaGetContextT = uintptr_t *(__fastcall *)(void);
    using LuaGetTableT = void (__fastcall *)(uintptr_t *luaState, int globalsIndex);
    using LuaCallT = void (__fastcall *)(const char *code, const char *unused);
    using LuaScriptT = uint32_t (__fastcall *)(uintptr_t *luaState);
    using GetGUIDFromNameT = std::uint64_t (__fastcall *)(const char *);
    using GetUnitFromNameT = uintptr_t (__fastcall *)(const char *);
    using lua_gettableT = void (__fastcall *)(uintptr_t *luaState, int globalsIndex);
    using lua_isstringT = bool (__fastcall *)(uintptr_t *, int);
    using lua_isnumberT = bool (__fastcall *)(uintptr_t *, int);
    using lua_tostringT = char *(__fastcall *)(uintptr_t *, int);
    using lua_tonumberT = double (__fastcall *)(uintptr_t *, int);
    using lua_pushnumberT = void (__fastcall *)(uintptr_t *, double);
    using lua_pushstringT = void (__fastcall *)(uintptr_t *, char *);
    using lua_pcallT = int (__fastcall *)(uintptr_t *, int nArgs, int nResults, int errFunction);
    using lua_pushnilT = void (__fastcall *)(uintptr_t *);
    using lua_errorT = void (__cdecl *)(uintptr_t *, const char *);
    using lua_settopT = void (__fastcall *)(uintptr_t *, int);


    using SpellVisualsInitializeT = void (__stdcall *)(void);

    using PlaySpellVisual = void (__stdcall *)(int **param_1, void *param_2, int param_3, void **param_4);

}