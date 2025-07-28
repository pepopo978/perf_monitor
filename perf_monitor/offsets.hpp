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

#pragma once

#include <cstdint>

enum class Offsets : std::uint32_t {
    OsGetAsyncTimeMs = 0X0042B790,

    GetClientConnection = 0X005AB490,
    GetNetStats = 0X00537F20,

    IEvtQueueDispatch = 0X004245B0,

    SpellVisualsInitialize = 0X006EC0E0,

    RenderWorld = 0X00482D70,

    OnWorldUpdate = 0X00482EA0,
    CWorldUpdate = 0x0066FD50,
    CGWorldFrameUnitUpdate = 0x00482CA0,

    OnWorldRender = 0x00483460,
    CWorldSceneRender = 0x00681070,
    CWorldRender = 0x006701E0,
    SpellVisualsRender = 0X006ECB20,
    SpellVisualsTick = 0X006ECA20,
    PlaySpellVisual = 0X0060EDF0,
    CWorldUnknownRender = 0X0069A3E0,
    UnknownOnRender1 = 0x0069a3e0,
    UnknownOnRender2 = 0x006813d0,
    UnknownOnRender3 = 0x006816d0,

    ObjectUpdateHandler = 0x004651a0,

    PaintScreen = 0x00764330,
    CSimpleTopOnLayerUpdate = 0X00765650,
    CSimpleTopOnLayerRender = 0X007657D0,

    CSimpleFrameOnFrameRender1 = 0X0076B3A0,
    CSimpleFrameOnFrameRender2 = 0X0076B3F0,
    CSimpleModelOnFrameRender = 0X0076D160,

    FrameScript_Execute = 0X007026F0,

    CSimpleFrameOnLayerUpdate = 0x0076B2C0,

    FrameScriptObjectOnScriptEvent = 0X00704D50,
    FrameScriptObjectOnScriptEventParam = 0x00702710,

    SignalEvent = 0X00703E50,
    SignalEventParam = 0X00703F50,

    CM2SceneAdvanceTime = 0X007074B0,
    CM2SceneAnimate = 0X00707680,
    CM2SceneDraw = 0X00708900,

    WorldObjectRender = 0X006EB840,  // barely impacted performance

    RunningAddonName = 0X00CEEAC0,
    RunningAddonName2 = 0X00CEEAC4,

    DrawBatchProj = 0x0070cb30,
    DrawBatch = 0x0070cf70,
    DrawBatchDoodad = 0x0070d330,
    DrawRibbon = 0x0070d820,
    DrawParticle = 0x0070d8b0,
    DrawCallback = 0x0070d960,
    CM2SceneRenderDraw = 0x0070b360,

    lua_isnumber = 0X006F34D0,
    lua_tonumber = 0X006F3620,

    EventUnregisterEx = 0X0041FD90,
    MovementIdleMoveUnits = 0X00616800,

    GxRsSet = 0X00589E60,

    luaC_collectgarbage = 0X006F7340,
    lua_getcontext = 0x007040D0,
    lua_getgccount = 0x006f43f0,

    CM2ModelAnimateMT = 0x00714260,
    ObjectFree = 0X00463B00
};
