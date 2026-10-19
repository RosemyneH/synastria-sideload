#include <cstdint>
#include "Sha1.h"

// STATIC: 0x00ADEDA8
static uint8_t pads[64] = { 0x80 };

namespace Private {
// INLINED: [0x0077A610->0x0077A62C, 0x0077A648->0x0077A764, 0x0077A764->0x0077A85C, 0x0077A860->0x0077A96F, 0x0077A970->0x0077AA64]
inline uint32_t S(uint32_t x, int32_t n) {
    return (x << n) | (x >> (32 - n));
}

// INLINED: 0x0077A648->0x0077A764
inline void R1(uint32_t a, uint32_t& b, uint32_t c, uint32_t d, uint32_t& e, uint32_t& w) {
    e += S(a, 5) + ((b & c) | (~b & d)) + w + 0x5A827999;
    b = S(b, 30);
    w = 0;
}

// INLINED: 0x0077A764->0x0077A85C
inline void R2(uint32_t a, uint32_t& b, uint32_t c, uint32_t d, uint32_t& e, uint32_t& w) {
    e += S(a, 5) + (b ^ c ^ d) + w + 0x6ED9EBA1;
    b = S(b, 30);
    w = 0;
}

// INLINED: 0x0077A860->0x0077A96F
inline void R3(uint32_t a, uint32_t& b, uint32_t c, uint32_t d, uint32_t& e, uint32_t& w) {
    e += S(a, 5) + (((c | d) & b) | (c & d)) + w + 0x8F1BBCDC;
    b = S(b, 30);
    w = 0;
}

// INLINED: 0x0077A970->0x0077AA64
inline void R4(uint32_t a, uint32_t& b, uint32_t c, uint32_t d, uint32_t& e, uint32_t& w) {
    e += S(a, 5) + (b ^ c ^ d) + w + 0xCA62C1D6;
    b = S(b, 30);
    w = 0;
}

// INLINED: 0x0077A571->0x0077A603
inline void Load(uint32_t& a, const uint8_t* const b) {
    a = (((((b[0] << 8) | b[1]) << 8) | b[2]) << 8) | b[3];
}

// INLINED: 0x0077ABA9->0x0077ABF7
inline void Save(uint64_t a, uint8_t* const b) {
    for (int32_t i = 7; i >= 0; --i) {
        b[i] = static_cast<uint8_t>(a);
        a >>= 8;
    }
}

// INLINED: 0x0077AC31->0x0077AC47
inline void Save(uint32_t a, uint8_t* const b) {
    for (int32_t i = 3; i >= 0; --i) {
        b[i] = static_cast<uint8_t>(a);
        a >>= 8;
    }
}
}

// VERIFY: 5599a282 manual
// OFFSET: 0x0077A560
void Sha1::Pump(uint32_t* const hash, const uint8_t* const data) {
    uint32_t w[80];

    for (int32_t i = 0; i < 16; ++i) {
        Private::Load(w[i], data + i * 4);
    }

    for (int32_t i = 16; i < 80; ++i) {
        w[i] = Private::S(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    uint32_t a = hash[0];
    uint32_t b = hash[1];
    uint32_t c = hash[2];
    uint32_t d = hash[3];
    uint32_t e = hash[4];
    int32_t i = 0;

    for (; i < 20; i += 5) {
        Private::R1(a, b, c, d, e, w[i]);
        Private::R1(e, a, b, c, d, w[i + 1]);
        Private::R1(d, e, a, b, c, w[i + 2]);
        Private::R1(c, d, e, a, b, w[i + 3]);
        Private::R1(b, c, d, e, a, w[i + 4]);
    }

    for (; i < 40; i += 5) {
        Private::R2(a, b, c, d, e, w[i]);
        Private::R2(e, a, b, c, d, w[i + 1]);
        Private::R2(d, e, a, b, c, w[i + 2]);
        Private::R2(c, d, e, a, b, w[i + 3]);
        Private::R2(b, c, d, e, a, w[i + 4]);
    }

    for (; i < 60; i += 5) {
        Private::R3(a, b, c, d, e, w[i]);
        Private::R3(e, a, b, c, d, w[i + 1]);
        Private::R3(d, e, a, b, c, w[i + 2]);
        Private::R3(c, d, e, a, b, w[i + 3]);
        Private::R3(b, c, d, e, a, w[i + 4]);
    }

    for (; i < 80; i += 5) {
        Private::R4(a, b, c, d, e, w[i]);
        Private::R4(e, a, b, c, d, w[i + 1]);
        Private::R4(d, e, a, b, c, w[i + 2]);
        Private::R4(c, d, e, a, b, w[i + 3]);
        Private::R4(b, c, d, e, a, w[i + 4]);
    }

    hash[0] += a;
    hash[1] += b;
    hash[2] += c;
    hash[3] += d;
    hash[4] += e;
}

// VERIFY: 24473150 auto
// OFFSET: 0x0077AAA0
void Sha1::Initialize() {
    this->m_size = 0;
    this->m_hash[0] = 0x67452301;
    this->m_hash[1] = 0xEFCDAB89;
    this->m_hash[2] = 0x98BADCFE;
    this->m_hash[3] = 0x10325476;
    this->m_hash[4] = 0xC3D2E1F0;
}

// VERIFY: 5d03f823 manual
// OFFSET: 0x0077AAE0
void Sha1::Append(const void* _data, uint32_t size) {
    const uint8_t* data = static_cast<const uint8_t*>(_data);
    uint32_t index = (static_cast<uint32_t>(this->m_size) >> 3) & 0x3F;
    this->m_size += static_cast<uint64_t>(size) * 8;

    if (index) {
        size += index;
        data -= index;

        if (size >= 64) {
            for (; index < 64; ++index) {
                this->m_data[index] = data[index];
            }

            Sha1::Pump(this->m_hash, this->m_data);
            data += 64;
            size -= 64;
            index = 0;
        }
    }

    for (; size >= 64; size -= 64) {
        Sha1::Pump(this->m_hash, data);
        data += 64;
    }

    for (; index < size; ++index) {
        this->m_data[index] = data[index];
    }
}

// VERIFY: 86da01d7 manual
// OFFSET: 0x0077ABA0
void Sha1::Finalize(uint8_t* const hash) {
    uint8_t size[8];
    Private::Save(this->m_size, size);
    this->Append(pads, ((-9 - (static_cast<uint32_t>(this->m_size) >> 3)) & 0x3F) + 1);
    this->Append(size, 8);

    for (int32_t i = 0; i < 5; ++i) {
        Private::Save(this->m_hash[i], hash + i * 4);
    }
}
