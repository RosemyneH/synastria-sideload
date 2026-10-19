#ifndef ENGINE_SOURCE_TEMPEST_CBOUNDINGBOX_H
#define ENGINE_SOURCE_TEMPEST_CBOUNDINGBOX_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/CRange.h"

class CBoundingBox {
    public:
    /* 0000 */ CRange m_x;
    /* 0008 */ CRange m_y;
    /* 0010 */ CRange m_z;
};

#endif
