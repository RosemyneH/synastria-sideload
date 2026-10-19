#ifndef WOW_SOURCE_UI_GMTICKETINFO_H
#define WOW_SOURCE_UI_GMTICKETINFO_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Net/Types.h"

class CDataStore;
struct lua_State;

int32_t Packet_Group_5(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg);

struct StaticConstant {
    /* 0000 */ const char* m_name;
    /* 0004 */ int32_t m_value;
};

class CGGMTicketInfo {
    public:
    static void LeaveWorld();
    static void EnterWorld();
    static void InitializeGame();
    static void ShutdownGame();
    static int32_t m_ticketID;
    static int32_t m_responseID;
    static void CreateTicket(const char* text, int32_t needResponse, uint32_t moreHelp);
};

class CGGMReportLag {
    public:
    static void Submit(int32_t type);
};

void GMTicketInfoRegisterScriptFunctions();
void GMTicketInfoUnregisterScriptFunctions();

void SendGetTicketOpcode();
void SendGMTicketUpdateText(const char* text);
void SendGMTicketDeleteTicket();
void SendGMResponseResolve();

#endif
