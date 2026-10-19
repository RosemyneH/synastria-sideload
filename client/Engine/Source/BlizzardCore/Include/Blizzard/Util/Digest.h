#ifndef ENGINE_SOURCE_BLIZZARDCORE_INCLUDE_BLIZZARD_UTIL_DIGEST_H
#define ENGINE_SOURCE_BLIZZARDCORE_INCLUDE_BLIZZARD_UTIL_DIGEST_H

#include <cstdint>
#include <cstring>

namespace Blizzard {
namespace Util {

template <int N>
struct Digest {
    /* 0000 */ uint8_t m_data[N];

    Digest& FromHexString(const char* hex);
    bool operator<(const Digest& other) const;
    bool operator==(const Digest& other) const;
    bool operator!=(const Digest& other) const;
};

extern const uint8_t s_hexDigits[256];

}
}

// VERIFY: 4cd92949 auto
// OFFSET: 0x00429580<16>
template <int N>
Blizzard::Util::Digest<N>& Blizzard::Util::Digest<N>::FromHexString(const char* hex) {
    if (hex && strlen(hex) == 2 * N) {
        for (int32_t i = 0; i < 2 * N; i += 2) {
            this->m_data[i >> 1] = (s_hexDigits[hex[i]] << 4) | s_hexDigits[hex[i + 1]];
        }
    }

    return *this;
}

// VERIFY: 182eff5b manual
// OFFSET: 0x00429540
template <int N>
bool Blizzard::Util::Digest<N>::operator<(const Digest& other) const {
    for (int32_t i = 0; i < N; i++) {
        if (this->m_data[i] < other.m_data[i]) {
            return true;
        }

        if (this->m_data[i] > other.m_data[i]) {
            return false;
        }
    }

    return false;
}

// VERIFY: 06cb648f manual
// OFFSET: 0x00668750
template <int N>
bool Blizzard::Util::Digest<N>::operator==(const Digest& other) const {
    return memcmp(this->m_data, other.m_data, N) == 0;
}

// VERIFY: 951b079f manual
// OFFSET: 0x004221A0
template <int N>
bool Blizzard::Util::Digest<N>::operator!=(const Digest& other) const {
    return memcmp(this->m_data, other.m_data, N) != 0;
}

#endif
