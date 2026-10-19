#ifndef ENGINE_SOURCE_GX_CGXVERTEXPC_H
#define ENGINE_SOURCE_GX_CGXVERTEXPC_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C3Vector.h"
#include "Engine/Source/Tempest/CImVector.h"

class CGxVertexPC {
    public:
    /* 0000 */ C3Vector p;
    /* 000C */ CImVector c;
};

#endif
