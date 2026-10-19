#ifndef ENGINE_SOURCE_TEMPEST_C3SEGMENT_H
#define ENGINE_SOURCE_TEMPEST_C3SEGMENT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C3Vector.h"

struct C3Segment {
    /* 0000 */ C3Vector b;
    /* 000C */ C3Vector t;

    C3Vector Lerp(float t) const;
};

#endif
