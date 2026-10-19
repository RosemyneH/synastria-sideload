#ifndef WOW_SOURCE_UI_CGBANKINFO_H
#define WOW_SOURCE_UI_CGBANKINFO_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/Object_C.h"

struct lua_State;

class CGBankInfo {
    public:
    static void SetBanker(const WOWGUID& banker);
    static void EnterWorld();
    static void LeaveWorld();
    static WOWGUID m_banker;
};

void BankRegisterScriptFunctions();
void BankUnregisterScriptFunctions();

#endif
