#include <cstdint>
#include "WoW/Source/ObjectAlloc/HeapUsage.h"
#include "Engine/Source/ConsoleBase/ConsoleVar.h"
#include "Engine/Source/ConsoleBase/IConsole.h"
#include "Engine/Source/Storm/H/storm.h"

// STATIC: 0x00B4AFBC
CVar* s_cvHeapAllocTracking;

// VERIFY: 030f8411 manual
// OFFSET: 0x004D2780
bool ChangeHeapAllocTracking(CVar* var, const char* oldValue, const char* value, void* arg) {
    if (SStrToInt(value)) {
        SMemSetDebugFlags(SMEM_DEBUG_HEAPTRACKING, SMEM_DEBUG_HEAPTRACKING);
        ConsoleWrite("Heap tracking switched ON", DEFAULT_COLOR);
    } else {
        SMemSetDebugFlags(0, SMEM_DEBUG_HEAPTRACKING);
        ConsoleWrite("Heap tracking switched OFF", DEFAULT_COLOR);
    }

    return true;
}
