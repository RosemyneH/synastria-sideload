#include "Engine/Source/Base/Clamp.h"

// VERIFY: c8057e49 manual
// OFFSET: 0x005C59F0
int32_t Clamp(int32_t value, int32_t low, int32_t high) {
    if (static_cast<uint32_t>(value - low) > static_cast<uint32_t>(high - low)) {
        value = value - low < 0 ? low : high;
    }

    return value;
}
