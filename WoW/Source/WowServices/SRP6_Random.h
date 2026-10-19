#ifndef WOW_SOURCE_WOWSERVICES_SRP6_RANDOM_H
#define WOW_SOURCE_WOWSERVICES_SRP6_RANDOM_H

#include <cstddef>
#include <cstdint>

class SRP6_Random {
    public:
    /* 0000 */ uint8_t m_seed [20];
    /* 0014 */ uint8_t m_state [20];
    /* 0028 */ uint8_t m_buffer [20];
    /* 003C */ uint32_t m_remaining;

    SRP6_Random(const void* seed, uint32_t seedLength);
    void GenerateRandomBytes(uint8_t* data, uint32_t length);
};

#endif
