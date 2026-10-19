#ifndef ENGINE_SOURCE_GX_CGXVERTEXPT_H
#define ENGINE_SOURCE_GX_CGXVERTEXPT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C2Vector.h"
#include "Engine/Source/Tempest/C3Vector.h"

class CGxVertexPT {
    public:
    /* 0000 */ C3Vector p;
    /* 000C */ C2Vector tc;
};

#endif
