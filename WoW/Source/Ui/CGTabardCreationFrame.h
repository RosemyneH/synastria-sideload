#ifndef WOW_SOURCE_UI_CGTABARDCREATIONFRAME_H
#define WOW_SOURCE_UI_CGTABARDCREATIONFRAME_H

#include <cstdint>
#include "WoW/Source/Object/WGUID.h"

struct lua_State;

void TabardCreationRegisterScriptFunctions();
void TabardCreationUnregisterScriptFunctions();

class CGTabardCreationFrame {
    public:
    static void Close();
    static void Open(const WOWGUID& vendor);
    static void EnterWorld();
    static WOWGUID m_vendor;
};

uint32_t GuildGetTabardCost();

#endif
