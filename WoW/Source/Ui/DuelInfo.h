#ifndef WOW_SOURCE_UI_DUELINFO_H
#define WOW_SOURCE_UI_DUELINFO_H

#include <cstdint>
#include "WoW/Source/Net/Types.h"
#include "WoW/Source/Object/WGUID.h"

struct lua_State;

class CGDuelInfo {
    public:
    static void ShutdownGame();
    static WOWGUID m_dueler;

    static void InitializeGame();
    static int32_t ProcessCountdown(const void* data, void* param);
    static void LeaveWorld();
    static void AcceptDuel();
    static void CancelDuel();
};

void DuelInfoRegisterScriptFunctions();
void DuelInfoUnregisterScriptFunctions();

class CDataStore;

int32_t Packet_SMSG_DUEL_REQUESTED(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg);
int32_t Packet_SMSG_DUEL_OUTOFBOUNDS(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg);
int32_t Packet_SMSG_DUEL_INBOUNDS(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg);
int32_t Packet_SMSG_DUEL_COUNTDOWN(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg);
int32_t Packet_SMSG_DUEL_COMPLETE(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg);
int32_t Packet_SMSG_DUEL_WINNER(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg);

#endif
