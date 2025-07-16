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
    UnknownOnRender1 = 0x00707680,

    ObjectUpdateHandler = 0x004651a0,

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

    RunningAddonName = 0X00CEEAC0,
};
