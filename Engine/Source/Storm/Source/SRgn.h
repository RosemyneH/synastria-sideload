#ifndef ENGINE_SOURCE_STORM_SOURCE_SRGN_H
#define ENGINE_SOURCE_STORM_SOURCE_SRGN_H

#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"
#include "Engine/Source/Storm/H/storm.h"

struct HLOCKEDRGN__ {
    /* 0000 */ int32_t unused;
};

typedef HLOCKEDRGN__* HLOCKEDRGN;

struct _SOURCE {
    /* 0000 */ RECTF rect;
    /* 0010 */ void* param;
    /* 0014 */ int32_t sequence;
    /* 0018 */ uint32_t flags;
};

struct _FOUNDPARAM {
    /* 0000 */ void* param;
    /* 0004 */ int32_t sequence;
};

struct RGN : TSHashObject<RGN, HASHKEY_NONE> {
    /* 0018 */ TSGrowableArray<_SOURCE> source;
    /* 0028 */ TSGrowableArray<RECTF> combined;
    /* 0038 */ TSGrowableArray<_FOUNDPARAM> foundparams;
    /* 0048 */ RECTF foundparamsrect;
    /* 0058 */ int32_t sequence;
    /* 005C */ int32_t dirty;

    ~RGN();
};

#endif
