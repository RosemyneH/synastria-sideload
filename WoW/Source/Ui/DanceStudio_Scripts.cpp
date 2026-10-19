#include "DanceStudio.h"
#include <cstdint>
#include "Engine/Source/FrameScript/FrameScript.h"
#include "Engine/Source/FrameScript/FrameScript_Object.h"
#include "Engine/Source/lua-5.1/src/lauxlib.h"
#include "Engine/Source/lua-5.1/src/lua.h"

// VERIFY: 3ffb6962 auto
// OFFSET: 0x005761F0
static int32_t Script_PlayDance(lua_State* L) {
    if (!lua_isstring(L, 1)) {
        luaL_error(L, "Usage: PlayDance(\"dance name\")");
    }

    const char* name = lua_tolstring(L, 1, nullptr);

    if (name) {
        PlayDance(name);
    }

    return 0;
}

// STATIC: 0x00ACE834
static FrameScript_Method s_ScriptFunctions[] = {
    { "PlayDance", &Script_PlayDance },
};

// VERIFY: 4f09f3b4 auto
// OFFSET: 0x00575820
void CGDanceStudio::RegisterScriptFunctions() {
    for (uint32_t i = 0; i < sizeof(s_ScriptFunctions) / sizeof(s_ScriptFunctions[0]); i++) {
        FrameScript_RegisterFunction(s_ScriptFunctions[i].m_name, s_ScriptFunctions[i].m_method);
    }
}

// VERIFY: 5268fde5 auto
// OFFSET: 0x00575840
void CGDanceStudio::UnregisterScriptFunctions() {
    for (uint32_t i = 0; i < sizeof(s_ScriptFunctions) / sizeof(s_ScriptFunctions[0]); i++) {
        FrameScript_UnregisterFunction(s_ScriptFunctions[i].m_name);
    }
}
