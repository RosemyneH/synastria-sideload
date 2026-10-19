#ifndef ENGINE_SOURCE_COMSAT_COMSATBITPACKER_H
#define ENGINE_SOURCE_COMSAT_COMSATBITPACKER_H

#include <cstddef>
#include <cstdint>

namespace ComSat {

class BitPacker {
    public:
    /* 0004 */ uint8_t* m_buffer;
    /* 0008 */ uint32_t m_bitLength;
    /* 000C */ uint32_t m_byteOffset;
    /* 0010 */ uint32_t m_bitOffset;

    virtual ~BitPacker();

    BitPacker(uint8_t* buffer, uint32_t bitLength);
    uint32_t Finalize();
    bool PackBits(const uint8_t* data, uint32_t bits);
};

class BitUnpacker {
    public:
    /* 0004 */ const uint8_t* m_buffer;
    /* 0008 */ uint32_t m_bitLength;
    /* 000C */ uint32_t m_byteOffset;
    /* 0010 */ uint32_t m_bitOffset;

    virtual ~BitUnpacker();

    BitUnpacker(const uint8_t* buffer, uint32_t bitLength);
    bool UnpackBits(uint8_t* data, uint32_t bits);
};

}

#endif
