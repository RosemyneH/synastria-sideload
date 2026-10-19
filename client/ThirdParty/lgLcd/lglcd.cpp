#include "ThirdParty/lgLcd/lglcd.h"

DWORD WINAPI lgLcdInit(void) {
    return RPC_S_SERVER_UNAVAILABLE;
}

DWORD WINAPI lgLcdDeInit(void) {
    return ERROR_SUCCESS;
}

DWORD WINAPI lgLcdConnectExA(lgLcdConnectContextExA* ctx) {
    return ERROR_SERVICE_NOT_ACTIVE;
}

DWORD WINAPI lgLcdDisconnect(int connection) {
    return ERROR_SERVICE_NOT_ACTIVE;
}

DWORD WINAPI lgLcdOpenByType(lgLcdOpenByTypeContext* ctx) {
    return ERROR_SERVICE_NOT_ACTIVE;
}

DWORD WINAPI lgLcdClose(int device) {
    return ERROR_SERVICE_NOT_ACTIVE;
}

DWORD WINAPI lgLcdUpdateBitmap(int device, const lgLcdBitmapHeader* bitmap, DWORD priority) {
    return ERROR_SERVICE_NOT_ACTIVE;
}

DWORD WINAPI lgLcdSetAsLCDForegroundApp(int device, int foregroundYesNoFlag) {
    return ERROR_SERVICE_NOT_ACTIVE;
}
