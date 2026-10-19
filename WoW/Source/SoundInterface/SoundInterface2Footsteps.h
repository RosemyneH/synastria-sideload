#ifndef WOW_SOURCE_SOUNDINTERFACE_SOUNDINTERFACE2FOOTSTEPS_H
#define WOW_SOURCE_SOUNDINTERFACE_SOUNDINTERFACE2FOOTSTEPS_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"

struct FOOTSTEPSNDCACHE : public TSHashObject<FOOTSTEPSNDCACHE, HASHKEY_NONE> {
    /* 0018 */ TSGrowableArray<uint32_t> m_sounds;
    /* 0028 */ TSGrowableArray<uint32_t> m_splashSounds;

    FOOTSTEPSNDCACHE();
};

#endif
