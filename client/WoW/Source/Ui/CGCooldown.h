#ifndef WOW_SOURCE_UI_CGCOOLDOWN_H
#define WOW_SOURCE_UI_CGCOOLDOWN_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Base/CDataAllocator.h"
#include "Engine/Source/Frame/CSimpleFrame.h"
#include "Engine/Source/Tempest/C2Vector.h"
#include "Engine/Source/Tempest/C3Vector.h"
#include "Engine/Source/Tempest/CImVector.h"

class CRect;
class CRenderBatch;
class CStatus;
class CTexture;
class XMLNode;
struct lua_State;

class CGCooldown : public CSimpleFrame {
    public:
    static CDataAllocator s_cooldownHeap;
    static int32_t s_metatable;
    static int32_t s_objectType;
    static const char* s_objectTypeName;
    static const char* s_starTextureName;
    static const char* s_edgeTextureName;
    static const uint16_t s_starIndices[24];
    static const uint16_t s_flashIndices[6];
    static const uint16_t s_edgeIndices[6];
    static float s_flashScale[7];
    static float s_flashAlpha[7];
    static float s_flashRotation[7];

    static void operator delete(void* ptr);
    static CSimpleFrame* Create(CSimpleFrame* parent);
    static void CreateScriptMetaTable();
    static void DestroyScriptMetaTable();
    static int32_t GetObjectType();
    static void RegisterScriptMethods(lua_State* L);

    /* 029C */ uint32_t m_startTime;
    /* 02A0 */ uint32_t m_duration;
    /* 02A4 */ uint32_t m_elapsed;
    /* 02A8 */ uint32_t m_flashing;
    /* 02AC */ uint32_t m_reverse;
    /* 02B0 */ CTexture* m_starTexture;
    /* 02B4 */ C3Vector m_starVerts [10];
    /* 032C */ C2Vector m_starTexCoords [10];
    /* 037C */ CImVector m_starColor;
    /* 0380 */ C3Vector m_flashVerts [4];
    /* 03B0 */ C2Vector m_flashTexCoords [4];
    /* 03D0 */ CImVector m_flashColor;
    /* 03D4 */ CTexture* m_edgeTexture;
    /* 03D8 */ uint32_t m_drawEdge;
    /* 03DC */ C3Vector m_edgeVerts [4];
    /* 040C */ C2Vector m_edgeTexCoords [4];
    /* 042C */ CImVector m_edgeColor;

    ~CGCooldown() override;
    int32_t GetScriptMetaTable() override;
    bool IsA(int32_t type) override;
    bool IsA(const char* typeName) override;
    const char* GetObjectTypeName() override;
    void OnLayerUpdate(float elapsedSec) override;
    void OnFrameRender(CRenderBatch* batch, uint32_t layer) override;
    void UpdateAlpha() override;
    void LoadXML(XMLNode* node, CStatus* status) override;
    void OnFrameSizeChanged(const CRect& rect) override;

    CGCooldown(CSimpleFrame* parent);
    void AnimateOctants01(uint32_t octant, float t);
    void AnimateOctants23(uint32_t octant, float t);
    void AnimateOctants45(uint32_t octant, float t);
    void AnimateOctants67(uint32_t octant, float t);
    void AnimateVerts();
    void ResetVerts();
    void SetCooldown(uint32_t start, uint32_t duration);
    void UpdateColors();
};

#endif
