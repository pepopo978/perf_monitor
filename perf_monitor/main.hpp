//
// Created by pmacc on 9/21/2024.
//

#pragma once

#include <hadesmem/process.hpp>
#include <hadesmem/patcher.hpp>
#include <detours.h>

#include <Windows.h>

#include <cstdint>
#include <memory>
#include <atomic>
#include <cstdarg>

#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include "events.hpp"

namespace perf_monitor {


    using ISceneEndT = int *(__fastcall *)(uintptr_t *unk);
    using EndSceneT = int (__fastcall *)(uintptr_t *unk);
    using GetTimeMsT = uint64_t (__stdcall *)();

    using StdcallT = void (__stdcall *)(void);
    using FastcallFrameT = void (__fastcall *)(uintptr_t *frame);
    using FrameBatchT = void (__fastcall *)(uintptr_t *frame, void *param_1, uint32_t param_2, int unk);

    using FrameOnLayerUpdateT = void (__fastcall *)(uintptr_t *frame, uint8_t unk, int unk2);

    using SignalEventT = void (__fastcall *)(int eventCode);
    using SignalEventParamT = void (__cdecl *)(int eventId, const char* formatString, ...);

    using FrameOnScriptEventT = void (__fastcall *)(int *param_1, int *param_2);
    using FrameOnScriptEventParamT = void (__cdecl *)(int *param_1, int *param_2, char *param_3, va_list args);
    using FrameScript_ExecuteT = void (__cdecl *)(int *param_1, int *param_2, char *param_3);

    using WorldUpdateT = void (__fastcall *)(float *param_1, float *param_2, float *param_3);

    using IEvtQueueDispatchT = void (__fastcall *)(uintptr_t *eventContext, EVENT_ID eventId, void *unk);

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

    inline bool IsValidAsciiString(const char* str, size_t maxLen = 256) {
        if (!str) return false;
        if (IsBadReadPtr(const_cast<void*>(reinterpret_cast<const void*>(str)), 1) != 0) return false;
        
        size_t len = 0;
        bool hasAlpha = false;
        
        for (size_t i = 0; i < maxLen; ++i) {
            if (IsBadReadPtr(const_cast<void*>(reinterpret_cast<const void*>(str + i)), 1) != 0) return false;
            if (str[i] == '\0') {
                len = i;
                break;
            }
            if (str[i] < 32 || str[i] > 126) return false;
            if ((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z')) {
                hasAlpha = true;
            }
        }
        
        return len >= 3 && hasAlpha;
    }

    inline void PrintValidStrings(uintptr_t* basePtr, size_t arraySize = 150, const char* logPrefix = "String") {
        if (!basePtr) return;
        
        for (size_t i = 0; i < arraySize; ++i) {
            if (basePtr[i] != 0 && IsBadReadPtr(reinterpret_cast<void*>(basePtr[i]), 1) == 0) {
                auto potentialString0 = reinterpret_cast<const char*>(basePtr[i]);
                if (IsValidAsciiString(potentialString0)) {
                    DEBUG_LOG(logPrefix << "[" << i << "]: " << potentialString0);
                }
                
                auto level1Ptr = reinterpret_cast<uintptr_t*>(basePtr[i]);
                
                for (size_t j = 0; j < 450; ++j) {
                    if (level1Ptr[j] != 0 && IsBadReadPtr(reinterpret_cast<void*>(level1Ptr[j]), 1) == 0) {
                        auto potentialString1 = reinterpret_cast<const char*>(level1Ptr[j]);
                        if (IsValidAsciiString(potentialString1)) {
                            DEBUG_LOG(logPrefix << "[" << i << "][" << j << "]: " << potentialString1);
                        }
                        
                        auto level2Ptr = reinterpret_cast<uintptr_t*>(level1Ptr[j]);
                        
                        for (size_t k = 0; k < 64; ++k) {
                            if (level2Ptr[k] != 0 && IsBadReadPtr(reinterpret_cast<void*>(level2Ptr[k]), 1) == 0) {
                                auto potentialString2 = reinterpret_cast<const char*>(level2Ptr[k]);
                                
                                if (IsValidAsciiString(potentialString2)) {
                                    DEBUG_LOG(logPrefix << "[" << i << "][" << j << "][" << k << "]: " << potentialString2);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}