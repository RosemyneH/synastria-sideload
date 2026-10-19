#ifndef ENGINE_SOURCE_STORM_SOURCE_SHA1_H
#define ENGINE_SOURCE_STORM_SOURCE_SHA1_H

#include <cstdint>

class Sha1 {
    private:
    static void Pump(uint32_t* const hash, const uint8_t* const data);

    public:
    /* 0000 */ uint64_t m_size;
    /* 0008 */ uint32_t m_hash[5];
    /* 001C */ uint8_t m_data[64];

    void Initialize();
    void Append(const void* data, uint32_t size);
    void Finalize(uint8_t* const hash);
};

#endif
