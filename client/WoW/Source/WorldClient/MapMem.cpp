#include "MapMem.h"
#include "WoW/Source/WorldClient/MapArea.h"

// VERIFY: bffb685d manual
// OFFSET: 0x007BFE40
void* CMapArea::AllocAsyncLoadBuffer(int32_t fileSize) {
    return SMemAlloc(fileSize, __FILE__, __LINE__, 0);
}

// VERIFY: 6c2669fd manual
// OFFSET: 0x007BFE60
void CMapArea::FreeAsyncLoadBuffer(void* buffer, int32_t size) {
    SMemFree(buffer, __FILE__, __LINE__, 0);
}
