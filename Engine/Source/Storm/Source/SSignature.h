#ifndef ENGINE_SOURCE_STORM_SOURCE_SSIGNATURE_H
#define ENGINE_SOURCE_STORM_SOURCE_SSIGNATURE_H

#include <cstdint>
#include "Sha1.h"

struct SSignatureData {
    /* 0000 */ uint32_t modulusSize;
    /* 0004 */ uint32_t pubExponentSize;
    /* 0008 */ uint32_t magicBufferUsed;
    /* 000C */ uint32_t magicBufferSize;
    /* 0010 */ uint8_t* magicBuffer;
    /* 0018 */ Sha1 sha;
};

#endif
