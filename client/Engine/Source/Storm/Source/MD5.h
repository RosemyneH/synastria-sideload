#ifndef ENGINE_SOURCE_STORM_SOURCE_MD5_H
#define ENGINE_SOURCE_STORM_SOURCE_MD5_H

#include <cstdint>

struct MD5_CTX {
    /* 0000 */ uint32_t state[4];
    /* 0010 */ uint32_t count[2];
    /* 0018 */ uint8_t buffer[64];
};

class MD5 {
    public:
    /* 0000 */ uint32_t val[4];

    MD5(char* str);
    const MD5& operator=(const char* str);
    uint8_t* ToArray(uint8_t* array) const;
};

void MD5Final(uint8_t digest[16], MD5_CTX* context);
void MD5Init(MD5_CTX* context);
void MD5Update(MD5_CTX* context, const uint8_t* input, uint32_t inputLen);

#endif
