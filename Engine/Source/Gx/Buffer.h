#ifndef ENGINE_SOURCE_GX_BUFFER_H
#define ENGINE_SOURCE_GX_BUFFER_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Gx/CGxDevice/Types.h"
#include "Engine/Source/Gx/Types.h"

class VertexBufDesc {
    public:
    /* 0000 */ CGxVertexAttrib* attribs;
    /* 0004 */ uint32_t attribCount;
    /* 0008 */ uint32_t size;
    /* 000C */ uint32_t mask;
};

namespace Buffer {
    extern const VertexBufDesc s_vertexBufDesc[GxVertexBufferFormats_Last];
}

#endif
