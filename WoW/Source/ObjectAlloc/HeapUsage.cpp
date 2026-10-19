#include <cstddef>
#include <cstdint>
#include "WoW/Source/ObjectAlloc/HeapUsage.h"
#include "Engine/Source/ConsoleBase/ConsoleCommand.h"

// VERIFY: 28c3bf02 auto
// OFFSET: 0x004D27D0
void HeapUsageDestroy() {
    ConsoleCommandUnregister("HeapUsage");
}

