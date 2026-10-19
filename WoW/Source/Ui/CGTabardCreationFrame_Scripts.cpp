#include "CGTabardCreationFrame.h"
#include <cstdint>
#include "Engine/Source/FrameScript/FrameScript.h"
#include "Engine/Source/FrameScript/FrameScript_Object.h"

// VERIFY: 5a471781 manual
// OFFSET: 0x00598DD0
static int32_t Script_GetTabardCreationCost(lua_State* L) {
    lua_pushnumber(L, GuildGetTabardCost());
    return 1;
}

// VERIFY: fff59080 auto
// OFFSET: 0x00598F30
static int32_t Script_CloseTabardCreation(lua_State* L) {
    CGTabardCreationFrame::Close();
    return 0;
}

// STATIC: 0x00ACF56C
static FrameScript_Method s_ScriptFunctions[] = {
    { "CloseTabardCreation", &Script_CloseTabardCreation },
    { "GetTabardCreationCost", &Script_GetTabardCreationCost },
};

// VERIFY: c7843cca auto
// OFFSET: 0x00598E10
void TabardCreationRegisterScriptFunctions() {
    for (uint32_t i = 0; i < sizeof(s_ScriptFunctions) / sizeof(s_ScriptFunctions[0]); i++) {
        FrameScript_RegisterFunction(s_ScriptFunctions[i].m_name, s_ScriptFunctions[i].m_method);
    }
}

// VERIFY: 6a9614d6 auto
// OFFSET: 0x00598E40
void TabardCreationUnregisterScriptFunctions() {
    for (uint32_t i = 0; i < sizeof(s_ScriptFunctions) / sizeof(s_ScriptFunctions[0]); i++) {
        FrameScript_UnregisterFunction(s_ScriptFunctions[i].m_name);
    }
}
