#include <cstddef>
#include <cstdint>
#include "Engine/Source/Frame/CSimpleTitleRegion.h"
#include "Engine/Source/Frame/CScriptRegion.h"
#include "Engine/Source/FrameScript/FrameScript.h"
#include "Engine/Source/FrameScript/FrameScript_Object.h"
#include "Engine/Source/lua-5.1/src/lauxlib.h"
#include "Engine/Source/lua-5.1/src/lua.h"

// VERIFY: 4bf7234b auto
// OFFSET: 0x0049E500
void CSimpleTitleRegion::RegisterScriptMethods(lua_State* L) {
    CScriptRegion::RegisterScriptMethods(L);
}

// VERIFY: d444bd47 auto
// OFFSET: 0x0049E590
void CSimpleTitleRegion::CreateScriptMetaTable() {
    CSimpleTitleRegion::s_metatable = FrameScript_Object::CreateScriptMetaTable(FrameScript_GetContext(), &CSimpleTitleRegion::RegisterScriptMethods);
}

// VERIFY: 85597986 auto
// OFFSET: 0x0049E510
void CSimpleTitleRegion::DestroyScriptMetaTable() {
    luaL_unref(FrameScript_GetContext(), LUA_REGISTRYINDEX, CSimpleTitleRegion::s_metatable);
    CSimpleTitleRegion::s_metatable = LUA_NOREF;
}
