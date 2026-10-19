#ifndef ENGINE_SOURCE_BLIZZARDCORE_INCLUDE_BLIZZARD_UTIL_MD5_H
#define ENGINE_SOURCE_BLIZZARDCORE_INCLUDE_BLIZZARD_UTIL_MD5_H

#include <cstdint>

namespace Blizzard {
namespace Util {

class MD5 {
    public:
    /* 0000 */ uint32_t m_count[2];
    /* 0008 */ uint32_t m_state[4];
    /* 0018 */ uint8_t m_buffer[64];

    MD5();
    void Finish(uint8_t* digest);
    void Process(const uint8_t* block);
    void Process(const void* data, uint32_t size);
};

void HashString(uint8_t* digest, const char* str);

}
}

#endif
