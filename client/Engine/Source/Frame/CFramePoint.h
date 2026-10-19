#ifndef ENGINE_SOURCE_FRAME_CFRAMEPOINT_H
#define ENGINE_SOURCE_FRAME_CFRAMEPOINT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Base/CDataAllocator.h"
#include "Engine/Source/Frame/Types.h"

class CLayoutFrame;
class CRect;

class CFramePoint {
    public:
    static float UNDEFINED;
    static CDataAllocator s_framePointHeap;

    static void* operator new(size_t bytes) noexcept;
    static void operator delete(void* ptr);

    /* 0000 */ float m_x;
    /* 0004 */ float m_y;
    /* 0008 */ CLayoutFrame* m_relative;
    /* 000C */ FRAMEPOINT m_framePoint : 8;
    /* 000C */ uint32_t m_flags : 24;

    CFramePoint(CLayoutFrame* relative, FRAMEPOINT framePoint, float x, float y);
    int32_t GetRelativeRect(CRect& rect);
    void MarkUnused();
    void SetRelative(CLayoutFrame* relative, FRAMEPOINT framePoint, float x, float y);
    float X(float scale);
    float Y(float scale);
};

#endif
