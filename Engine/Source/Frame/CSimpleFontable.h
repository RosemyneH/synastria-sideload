#ifndef ENGINE_SOURCE_FRAME_CSIMPLEFONTABLE_H
#define ENGINE_SOURCE_FRAME_CSIMPLEFONTABLE_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"

class CSimpleFont;
class CSimpleFontStringAttributes;

class CSimpleFontable {
    public:
    /* 0004 */ CSimpleFont* m_fontObject;
    /* 0008 */ uint32_t m_fontableFlags;
    /* 000C */ TSLink<CSimpleFontable> m_fontableLink;

    virtual ~CSimpleFontable();
    virtual void FontObjectUpdated(CSimpleFontStringAttributes& attributes) = 0;

    CSimpleFontable();
    void SetFontObject(CSimpleFont* font);
};

#endif
