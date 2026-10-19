#ifndef ENGINE_SOURCE_TEMPEST_CAASPHERE_H
#define ENGINE_SOURCE_TEMPEST_CAASPHERE_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C3Vector.h"
#include "Engine/Source/Tempest/CAaBox.h"

class CAaSphere {
    public:
    static CAaSphere BoundingAccurate(const CAaSphere* spheres, uint32_t count);
    static CAaBox FindExtrema(const CAaSphere* spheres, uint32_t count);

    /* 0000 */ C3Vector c;
    /* 000C */ float r;

    CAaSphere();
    CAaBox Box() const;
};

#endif
