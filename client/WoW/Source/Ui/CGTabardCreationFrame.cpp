#include "CGTabardCreationFrame.h"
#include "WoW/Source/Object/ObjectClient/Player_C.h"
#include "WoW/Source/ObjectMgrClient/ObjectMgrClient.h"
#include "WoW/Source/Ui/GameUI.h"
#include "Engine/Source/FrameScript/FrameScript.h"
#include "WoW/Source/Ui/Types.h"
#include "WoW/Source/Ui/ScriptEvents.h"

// STATIC: 0x00C0E518
WOWGUID CGTabardCreationFrame::m_vendor;

// VERIFY: 4176cf42 auto
// OFFSET: 0x00598DC0
void CGTabardCreationFrame::EnterWorld() {
    CGTabardCreationFrame::m_vendor = WOWGUID();
}

// VERIFY: 52c0130a manual
// OFFSET: 0x00598E60
void CGTabardCreationFrame::Open(const WOWGUID& vendor) {
    CGTabardCreationFrame::m_vendor = vendor;
    CGPlayer_C* player = ClntObjMgrGetActivePlayerObj();

    if (player) {
        player->TurnOnGuildTabardPurchase();
    }

    CGGameUI::SetInteractTarget(vendor);
    FrameScript_SignalEvent(SCRIPT_EVENT_OPEN_TABARD_FRAME, nullptr);
}

// VERIFY: e3271ac3 manual
// OFFSET: 0x00598EC0
void CGTabardCreationFrame::Close() {
    if (!CGTabardCreationFrame::m_vendor) {
        return;
    }

    CGPlayer_C* player = ClntObjMgrGetActivePlayerObj();

    if (player) {
        player->TurnOffGuildTabardPurchase();
    }

    CGGameUI::ClearInteractTarget(CGTabardCreationFrame::m_vendor, 0);
    CGTabardCreationFrame::m_vendor = WOWGUID();
    FrameScript_SignalEvent(SCRIPT_EVENT_CLOSE_TABARD_FRAME, nullptr);
}

// VERIFY: b533bac0 auto
// OFFSET: 0x007EAE60
uint32_t GuildGetTabardCost() {
    return 100000;
}
