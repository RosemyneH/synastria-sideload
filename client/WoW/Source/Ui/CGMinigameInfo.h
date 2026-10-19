#ifndef WOW_SOURCE_UI_CGMINIGAMEINFO_H
#define WOW_SOURCE_UI_CGMINIGAMEINFO_H

#include <cstdint>
#include "WoW/Source/Net/Types.h"

struct lua_State;
class CMinigame_C;

class CGMinigameInfo {
    public:
    static CMinigame_C* m_minigame;

    static void LeaveWorld();
    static void ShutdownGame();
    static void InitializeGame();
};

void MinigameInfoRegisterScriptFunctions();
void MinigameInfoUnregisterScriptFunctions();

class CDataStore;

int32_t Packet_SMSG_MINIGAME_SETUP(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg);
int32_t Packet_SMSG_MINIGAME_STATE(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg);

#endif
