#ifndef ENGINE_SOURCE_STORM_SOURCE_RSA_H
#define ENGINE_SOURCE_STORM_SOURCE_RSA_H

#include <cstdint>
#include "BigNum.h"

namespace Crypt {
class RSA {
    public:
    /* 0000 */ BigNum m_modulus;
    /* 0004 */ BigNum m_exponent;

    void Prepare(const void* modulus, uint32_t mLength, const void* exponent, uint32_t eLength);
    void Process(uint8_t* data, uint32_t length);
};
}

#endif
