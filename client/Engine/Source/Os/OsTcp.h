#ifndef ENGINE_SOURCE_OS_OSTCP_H
#define ENGINE_SOURCE_OS_OSTCP_H

#include <cstdint>
#include "Engine/Source/Storm/H/Types.h"

union NETADDR {
    /* 0000 */ uint8_t data[16];
};

struct NETCONNADDR {
    /* 0000 */ NETADDR peerAddr;
    /* 0010 */ NETADDR selfAddr;
};

struct HNETCONN__ {
    /* 0000 */ int32_t unused;
};

typedef HNETCONN__* HNETCONN;

enum NETNOTE {
    NETNOTE_CONNECT = 0,
    NETNOTE_DATA = 1,
    NETNOTE_DISCONNECT = 2,
    NETNOTE_CANTCONNECT = 3,
    NETNOTE_FILEWRITE = 4,
    NETNOTE_FILEREAD = 5,
};

enum OS_SEND {
    OS_SEND_OK = 0,
    OS_SEND_SOCKET_CLOSED = 1,
    OS_SEND_TIMEOUT = 2,
    OS_SEND_ERROR = 3,
};

typedef int32_t (STORMAPI* NETEVENTPROC)(HNETCONN conn, const NETCONNADDR* connAddr, NETNOTE note, void* user, const void* data, uint32_t bytes, uint32_t* bytesProcessed);

uint32_t OsNetAddrGetAddress(const NETADDR* addr, uint16_t* port);
void OsNetAddrMake(uint32_t address, uint16_t port, NETADDR* addr);
const char* OsNetAddrToStr(const NETADDR* addr, char* buffer, uint32_t size);
void OsNetPump(uint32_t timeout);

#endif
