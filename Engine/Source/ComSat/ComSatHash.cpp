#include "Engine/Source/ComSat/ComSatHash.h"

// VERIFY: a913c4f0 auto
// OFFSET: 0x00887330
uint32_t ComSat::Hash32(const void* data, uint32_t length, uint32_t seed) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint32_t hash = seed;
    for (; length; length--) {
        hash = (hash ^ *bytes) * 0x1000193;
        bytes++;
    }
    return hash;
}
