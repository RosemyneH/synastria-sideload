#ifndef ENGINE_SOURCE_GX_CGXVERTEXPCT_H
#define ENGINE_SOURCE_GX_CGXVERTEXPCT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C2Vector.h"
#include "Engine/Source/Tempest/C3Vector.h"
#include "Engine/Source/Tempest/CImVector.h"

class CGxVertexPCT {
    public:
    /* 0000 */ C3Vector p;
    /* 000C */ CImVector c;
    /* 0010 */ C2Vector tc;
};

#endif
