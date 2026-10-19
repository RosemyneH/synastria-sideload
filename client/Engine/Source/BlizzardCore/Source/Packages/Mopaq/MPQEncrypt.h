#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_MOPAQ_MPQENCRYPT_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_MOPAQ_MPQENCRYPT_H

#include <cstdint>

namespace System_MPQEncrypt {

extern const uint32_t s_cryptTable[1280];
extern const uint8_t s_hashChars[256];

void Decrypt(uint32_t* data, uint32_t size, uint32_t key);
void Encrypt(uint32_t* data, uint32_t size, uint32_t key);
uint32_t Hash(const char* str, uint32_t type);

}

#endif
