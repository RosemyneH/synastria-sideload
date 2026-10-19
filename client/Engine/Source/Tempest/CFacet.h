#ifndef ENGINE_SOURCE_TEMPEST_CFACET_H
#define ENGINE_SOURCE_TEMPEST_CFACET_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C3Vector.h"
#include "Engine/Source/Tempest/C4Plane.h"

class CFacet {
    public:
    /* 0000 */ C4Plane plane;
    /* 0010 */ C3Vector v [3];

    CFacet(float value = 0.0f);
    CFacet(const C3Vector& v0, const C3Vector& v1, const C3Vector& v2);
    void Set(float value);
};

#endif
