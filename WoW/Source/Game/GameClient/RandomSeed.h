#ifndef WOW_SOURCE_GAME_GAMECLIENT_RANDOMSEED_H
#define WOW_SOURCE_GAME_GAMECLIENT_RANDOMSEED_H

#include <cstdint>

struct _RandomSeed {
    /* 0000 */ uint32_t m_mem[256];
    /* 0400 */ uint32_t m_rsl[256];
    /* 0800 */ uint32_t m_a;
    /* 0804 */ uint32_t m_b;
    /* 0808 */ int32_t m_count;
};

void InitializeRandomSeed(_RandomSeed* seed, uint32_t value);
uint32_t GetRandomValue(uint32_t max, _RandomSeed* seed);

#endif
