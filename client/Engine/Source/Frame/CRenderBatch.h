#ifndef ENGINE_SOURCE_FRAME_CRENDERBATCH_H
#define ENGINE_SOURCE_FRAME_CRENDERBATCH_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Frame/CSimpleBatchedMesh.h"
#include "Engine/Source/Gx/Types.h"
#include "Engine/Source/Storm/H/stpl.h"

class C2Vector;
class C3Vector;
class CGxShader;
struct CGxStringBatch;
class CImVector;
class CSimpleFontString;
class CSimpleTexture;
class CTexture;

struct RENDERCALLBACKNODE : public TSLinkedNode<RENDERCALLBACKNODE> {
    /* 0008 */ void (*m_callback)(void* param);
    /* 000C */ void* m_param;
};

class CRenderBatch {
    public:
    /* 0004 */ uint32_t m_count;
    /* 0008 */ TSGrowableArray<CSimpleBatchedMesh> m_texturelist;
    /* 0018 */ CGxStringBatch* m_stringbatch;
    /* 001C */ TSList<RENDERCALLBACKNODE, TSGetLink<RENDERCALLBACKNODE> > m_callbacks;
    /* 0028 */ TSLink<CRenderBatch> m_renderLink;

    virtual ~CRenderBatch();

    CRenderBatch();
    void Clear();
    void Finish();
    void Queue(CTexture* texture, EGxBlend alphaMode, int32_t posCount, const C3Vector* position, const C2Vector* texCoord, int32_t colorCount, const CImVector* color, int32_t idxCount, const uint16_t* indices, CGxShader* shader);
    void QueueCallback(void (*callback)(void*), void* param);
    void QueueFontString(CSimpleFontString* string);
    void QueueTexture(CSimpleTexture* texture);
};

#endif
