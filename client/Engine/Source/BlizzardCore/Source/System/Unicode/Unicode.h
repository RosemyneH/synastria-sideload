#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_UNICODE_UNICODE_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_UNICODE_UNICODE_H

#include <cstdint>
#include "Engine/Source/BlizzardCore/Source/System/Memory/TempBuffer.h"

namespace Blizzard {
namespace Unicode {

int32_t ConvertUTF16to8(char* dst, uint32_t dstmaxchars, const uint16_t* src, int32_t srcmaxchars, uint32_t* dstchars, uint32_t* srcchars);
int32_t ConvertUTF8to16(uint16_t* dst, uint32_t dstmaxchars, const char* src, int32_t srcmaxchars, uint32_t* dstchars, uint32_t* srcchars);

}
}

namespace Blizzard {
namespace StringConversion_CodepageToUTF8 {

class ToNative : public Blizzard::Memory::TempBuffer<char, 300> {
    public:
    ToNative(const char* str);
};

}
}

namespace System_Unicode {

extern const uint32_t s_offsetsFromUTF8[7];
extern const uint32_t s_firstByteMark[7];
extern const uint8_t s_bytesFromUTF8[256];

}

#endif
