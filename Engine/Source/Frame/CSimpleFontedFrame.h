#ifndef ENGINE_SOURCE_FRAME_CSIMPLEFONTEDFRAME_H
#define ENGINE_SOURCE_FRAME_CSIMPLEFONTEDFRAME_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Frame/CSimpleFont.h"

class CSimpleFontedFrameFont;

class CSimpleFontedFrame {
    public:
    virtual void FontUpdated(CSimpleFontedFrameFont* font, int32_t flags) = 0;
};

class CSimpleFontedFrameFont : public CSimpleFont {
    public:
    /* 006C */ CSimpleFontedFrame* m_frame;

    virtual void UpdateObjects();

    CSimpleFontedFrameFont(CSimpleFontedFrame* frame);
};

#endif
