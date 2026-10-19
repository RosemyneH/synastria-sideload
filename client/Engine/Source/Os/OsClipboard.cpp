#include "Engine/Source/Os/OsClipboard.h"
#include "Engine/Source/Gx/CGxDevice/CGxDevice.h"

// VERIFY: a242c6ef auto
// OFFSET: 0x00872280
char* OsClipboardGetString() {
    return IOsClipboardGetString(g_theGxDevicePtr->DeviceWindow());
}

// VERIFY: 0a0a9194 auto
// OFFSET: 0x008722A0
int32_t OsClipboardPutString(const char* string) {
    return IOsClipboardPutString(string, g_theGxDevicePtr->DeviceWindow());
}
