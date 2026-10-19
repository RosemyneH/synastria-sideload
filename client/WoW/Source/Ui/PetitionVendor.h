#ifndef WOW_SOURCE_UI_PETITIONVENDOR_H
#define WOW_SOURCE_UI_PETITIONVENDOR_H

#include <cstdint>
#include "WoW/Source/Object/WGUID.h"
#include "WoW/Source/Ui/GuildRegistrar.h"

struct lua_State;

void PetitionVendorRegisterScriptFunctions();
void PetitionVendorUnregisterScriptFunctions();
void PetitionVendorClose();
void PetitionVendorItemCallback(int32_t id, const WOWGUID& guid, void* param, bool success);

class CGPetitionVendorInfo {
    public:
    static WOWGUID m_vendor;
    static uint32_t m_numPetitionItems;
    static WOWGUID m_petitionItem;
    static uint8_t m_petitionItemSlot;
    static WOWGUID m_petitionItemContainer;
    static GuildPetitionInfo m_petitionItems[10];
    static uint32_t m_arenaTeamSize;
    static uint32_t m_arenaBackgroundColor;
    static uint32_t m_arenaEmblemStyle;
    static uint32_t m_arenaEmblemColor;
    static uint32_t m_arenaBorderStyle;
    static uint32_t m_arenaBorderColor;
    static int32_t ClickPetitionButton(WOWGUID item, WOWGUID container, uint8_t slot);
    static void ShowPetitionList(WOWGUID vendor, const GuildPetitionInfo* petitions, uint32_t count);
    static void BuyPetition(uint32_t index, const char* name);

    static void EnterWorld();
};

#endif
