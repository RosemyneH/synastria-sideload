#include <cstdint>
#include "RSA.h"

// VERIFY: ec6208ab auto
// OFFSET: 0x0077C9B0
void Crypt::RSA::Prepare(const void* modulus, uint32_t mLength, const void* exponent, uint32_t eLength) {
    this->m_modulus.FromBinary(modulus, mLength);
    this->m_exponent.FromBinary(exponent, eLength);
}

// VERIFY: ac3878cb auto
// OFFSET: 0x0077C9E0
void Crypt::RSA::Process(uint8_t* data, uint32_t length) {
    BigNum src;
    src.FromBinary(data, length);
    BigNum dst;
    dst.PowMod(src, this->m_exponent, this->m_modulus);
    dst.ToBinaryBuffer(data, length);
}
