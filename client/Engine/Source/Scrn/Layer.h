#ifndef ENGINE_SOURCE_SCRN_LAYER_H
#define ENGINE_SOURCE_SCRN_LAYER_H

#include <cstdint>
#include "Engine/Source/Base/Handle.h"
#include "Engine/Source/Storm/H/Types.h"
#include "Engine/Source/Storm/H/stpl.h"

class CILayer : public CHandleObject {
    public:
    /* 0008 */ RECTF m_rect;
    /* 0018 */ RECTF m_visible;
    /* 0028 */ float m_zorder;
    /* 002C */ uint32_t m_flags;
    /* 0030 */ void* m_param;
    /* 0034 */ void (*m_paintFunc)(void* param, const RECTF* rect, const RECTF* visible, float elapsedSec);
    /* 0038 */ TSLink<CILayer> m_zorderlink;

    virtual ~CILayer();
};

void ILayerDestroy();
void ILayerInitialize();

#endif
