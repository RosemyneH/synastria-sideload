#ifndef ENGINE_SOURCE_BASE_SIMD_H
#define ENGINE_SOURCE_BASE_SIMD_H

#include <cstddef>
#include <cstdint>

namespace SIMD {
    void* AllocA16(uint32_t size, const char* fileName, int32_t lineNumber);
    void FreeA16(void* ptr);
}

#endif
