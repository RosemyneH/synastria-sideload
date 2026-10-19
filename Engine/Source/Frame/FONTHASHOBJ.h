#ifndef ENGINE_SOURCE_FRAME_FONTHASHOBJ_H
#define ENGINE_SOURCE_FRAME_FONTHASHOBJ_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Base/Handle.h"
#include "Engine/Source/Storm/H/stpl.h"

struct CGxFont;
struct FONTHASHOBJ;
struct HASHKEY_STR;

struct FONTHASHOBJ : public CHandleObject {
    /* 0008 */ TSHashObject<FONTHASHOBJ, HASHKEY_STR> m_hashObject;
    /* 0020 */ CGxFont* m_font;
};

#endif
