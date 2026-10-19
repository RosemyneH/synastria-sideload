#ifndef ENGINE_SOURCE_STORM_SOURCE_BIGNUM_H
#define ENGINE_SOURCE_STORM_SOURCE_BIGNUM_H

#include <cstdint>
#include <cstring>
#include "Engine/Source/Storm/H/storm.h"

class BigNum {
    public:
    /* 0000 */ BigData* m_data;

    BigNum();
    ~BigNum();
    void FromBinary(const void* data, uint32_t bytes);
    BigNum& PowMod(const BigNum& b, const BigNum& c, const BigNum& d);
    void* ToBinaryBuffer(void* data, uint32_t bytes) const;
};

// INLINED: [0x0077C9E9->0x0077C9F4, 0x0077CA05->0x0077CA0E, 0x00770E1F->0x00770E28, 0x00770E28->0x00770E31]
inline BigNum::BigNum() {
    SBigNew(&this->m_data);
}

// INLINED: [0x0077CA48->0x0077CA51, 0x0077CA51->0x0077CA5A, 0x00770ED7->0x00770EE9, 0x00770EE9->0x00770EF2]
inline BigNum::~BigNum() {
    SBigDel(this->m_data);
}

// INLINED: [0x0077C9B3->0x0077C9C6, 0x0077C9C6->0x0077C9D7, 0x0077C9F4->0x0077CA05]
inline void BigNum::FromBinary(const void* data, uint32_t bytes) {
    SBigFromBinary(this->m_data, data, bytes);
}

// INLINED: 0x0077CA0E->0x0077CA22
inline BigNum& BigNum::PowMod(const BigNum& b, const BigNum& c, const BigNum& d) {
    SBigPowMod(this->m_data, b.m_data, c.m_data, d.m_data);
    return *this;
}

// INLINED: 0x0077CA22->0x0077CA48
inline void* BigNum::ToBinaryBuffer(void* data, uint32_t bytes) const {
    uint32_t written;
    SBigToBinaryBuffer(this->m_data, data, bytes, &written);

    if (written < bytes) {
        memset(static_cast<uint8_t*>(data) + written, 0, bytes - written);
    }

    return data;
}

#endif
