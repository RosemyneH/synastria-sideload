#include "SRP6_Random.h"
#include <cstring>
#include "Engine/Source/BlizzardCore/Source/Packages/Util/SHA1.h"

// VERIFY: 6d10b67a auto
// OFFSET: 0x009A8980
SRP6_Random::SRP6_Random(const void* seed, uint32_t seedLength) {
    SHA1_CONTEXT context;
    SHA1_Init(&context);
    SHA1_Update(&context, static_cast<const uint8_t*>(seed), seedLength);
    SHA1_Final(this->m_seed, &context);

    memcpy(this->m_state, this->m_seed, sizeof(this->m_state));
    memset(this->m_buffer, 0, sizeof(this->m_buffer));
    this->m_remaining = 0;
}

// VERIFY: 9c44517b auto
// OFFSET: 0x009A89F0
void SRP6_Random::GenerateRandomBytes(uint8_t* data, uint32_t length) {
    if (!length) {
        return;
    }

    while (length > this->m_remaining) {
        memcpy(data, &this->m_buffer[sizeof(this->m_buffer) - this->m_remaining], this->m_remaining);
        data += this->m_remaining;
        length -= this->m_remaining;

        SHA1_CONTEXT context;
        SHA1_Init(&context);
        SHA1_Update(&context, this->m_seed, sizeof(this->m_seed));
        SHA1_Update(&context, this->m_buffer, sizeof(this->m_buffer));
        SHA1_Update(&context, this->m_state, sizeof(this->m_state));
        SHA1_Final(this->m_buffer, &context);

        this->m_remaining = sizeof(this->m_buffer);
    }

    if (length) {
        memcpy(data, &this->m_buffer[sizeof(this->m_buffer) - this->m_remaining], length);
        this->m_remaining -= length;
    }
}
