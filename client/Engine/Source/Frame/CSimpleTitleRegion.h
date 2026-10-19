#ifndef ENGINE_SOURCE_FRAME_CSIMPLETITLEREGION_H
#define ENGINE_SOURCE_FRAME_CSIMPLETITLEREGION_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Frame/CScriptRegion.h"

class CSimpleTitleRegion : public CScriptRegion {
    public:
    static int32_t s_metatable;

    static void CreateScriptMetaTable();
    static void DestroyScriptMetaTable();
    static void RegisterScriptMethods(lua_State* L);

    virtual int32_t GetScriptMetaTable();
};

#endif
