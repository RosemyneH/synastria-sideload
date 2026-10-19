#ifndef ENGINE_SOURCE_SERVICES_TEXTUREINT_H
#define ENGINE_SOURCE_SERVICES_TEXTUREINT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/BLPFile/Types.h"
#include "Engine/Source/Storm/H/stpl.h"

class CMipBitsCache {
    public:
    /* 0000 */ MipBits* m_bits;
    /* 0004 */ TSLink<CMipBitsCache> m_link;

    CMipBitsCache();
    ~CMipBitsCache();
};

#endif
