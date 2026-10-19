#ifndef ENGINE_SOURCE_FRAME_CSIMPLEEMBEDDEDTEXTURE_H
#define ENGINE_SOURCE_FRAME_CSIMPLEEMBEDDEDTEXTURE_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"

class CSimpleFontString;
class CSimpleTexture;
struct GXUEMBEDDEDTEXTUREINFO;

class CSimpleEmbeddedTexture {
    public:
    /* 0000 */ CSimpleTexture* m_texture;
    /* 0004 */ TSLink<CSimpleEmbeddedTexture> m_link;

    CSimpleEmbeddedTexture();
    ~CSimpleEmbeddedTexture();
    void InitEmbeddedTexture(CSimpleFontString* string, const GXUEMBEDDEDTEXTUREINFO* info);
};

#endif
