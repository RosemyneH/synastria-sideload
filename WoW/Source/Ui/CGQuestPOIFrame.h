#ifndef WOW_SOURCE_UI_CGQUESTPOIFRAME_H
#define WOW_SOURCE_UI_CGQUESTPOIFRAME_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Base/CDataAllocator.h"
#include "Engine/Source/Frame/CSimpleFrame.h"
#include "Engine/Source/Storm/H/stpl.h"
#include "Engine/Source/Tempest/C2Vector.h"
#include "Engine/Source/Tempest/C3Vector.h"
#include "Engine/Source/Tempest/CRect.h"

class CRenderBatch;
class CStatus;
class CTexture;
class XMLNode;
struct QuestPOIBlob;
struct QuestPOIInfo;
struct QuestPOIPoint;
struct lua_State;

class CGQuestPOIFrame : public CSimpleFrame {
    public:
    struct QuestPOIRenderData {
        /* 0000 */ QuestPOIBlob* m_blob;
        /* 0004 */ TSGrowableArray<C3Vector> m_fillVerts;
        /* 0014 */ TSGrowableArray<C2Vector> m_fillTexCoords;
        /* 0024 */ TSGrowableArray<uint16_t> m_fillIndices;
        /* 0034 */ TSGrowableArray<C3Vector> m_borderVerts;
        /* 0044 */ TSGrowableArray<C2Vector> m_borderTexCoords;
        /* 0054 */ TSGrowableArray<uint16_t> m_borderIndices;
        /* 0064 */ CRect m_bounds;
        /* 0074 */ int32_t m_objectives[4];
        /* 0084 */ uint32_t m_numObjectives;
        /* 0088 */ uint8_t m_valid;

        ~QuestPOIRenderData();
    };

    struct QuestPOIRenderGroup {
        /* 0000 */ QuestPOIInfo* m_info;
        /* 0004 */ uint8_t m_dirty;
        /* 0008 */ TSGrowableArray<QuestPOIRenderData> m_renderData;
    };

    static CDataAllocator s_questPOIHeap;
    static int32_t s_metatable;
    static int32_t s_objectType;
    static const char* s_objectTypeName;

    static void operator delete(void* ptr);
    static CSimpleFrame* Create(CSimpleFrame* parent);
    static void CreateScriptMetaTable();
    static void DestroyScriptMetaTable();
    static int32_t GetObjectType();
    static void RegisterScriptMethods(lua_State* L);
    static void RenderCallback(void* param);

    /* 029C */ CTexture* m_fillTexture;
    /* 02A0 */ CTexture* m_borderTexture;
    /* 02A4 */ uint8_t m_fillAlpha;
    /* 02A5 */ uint8_t m_borderAlpha;
    /* 02A8 */ float m_borderScalar;
    /* 02AC */ uint8_t m_smoothing;
    /* 02AD */ uint8_t m_merging;
    /* 02B0 */ float m_mergeThreshold;
    /* 02B4 */ uint32_t m_numSplinePoints;
    /* 02B8 */ int32_t m_mouseOverQuestId;
    /* 02BC */ uint32_t m_mouseOverCount;
    /* 02C0 */ int32_t m_mouseOverObjectives[4];
    /* 02D0 */ QuestPOIRenderGroup m_groups [4];

    ~CGQuestPOIFrame() override;
    int32_t GetScriptMetaTable() override;
    bool IsA(int32_t type) override;
    bool IsA(const char* typeName) override;
    const char* GetObjectTypeName() override;
    void OnFrameRender(CRenderBatch* batch, uint32_t layer) override;
    void LoadXML(XMLNode* node, CStatus* status) override;

    CGQuestPOIFrame(CSimpleFrame* parent);
    void Render();
    void UpdateMouseOverPOI(float x, float y);
    void MergeBlobs(uint32_t index);
    void BuildBlobOutline(QuestPOIBlob* blob, TSGrowableArray<QuestPOIPoint>* points);
    void BuildBlobRenderData(QuestPOIRenderData* data, QuestPOIBlob* blob);
    void RemoveQuest(int32_t questId);
    bool BuildRenderData(uint32_t index);
    void AddQuest(QuestPOIInfo* info);
    void SetBorderTexture(const char* fileName);
    void SetFillTexture(const char* fileName);
};

#endif
