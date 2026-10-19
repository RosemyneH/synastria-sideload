#ifndef ENGINE_SOURCE_FRAME_CSIMPLESCROLLFRAME_H
#define ENGINE_SOURCE_FRAME_CSIMPLESCROLLFRAME_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Base/CDataAllocator.h"
#include "Engine/Source/Frame/CSimpleFrame.h"
#include "Engine/Source/FrameScript/FrameScript_Object.h"
#include "Engine/Source/Tempest/C2Vector.h"

class CRect;
class CRenderBatch;
class CStatus;
class XMLNode;
struct lua_State;

class CSimpleScrollFrame : public CSimpleFrame {
    public:
    static CDataAllocator s_allocator;
    static int32_t s_metatable;
    static int32_t s_objectType;
    static const char* s_objectTypeName;

    static void operator delete(void* ptr);
    static void CreateScriptMetaTable();
    static void DestroyScriptMetaTable();
    static int32_t GetObjectType();
    static void RegisterScriptMethods(lua_State* L);
    static void RenderScrollChild(void* param);

    /* 029C */ uint32_t m_scrollChildUpdate;
    /* 02A0 */ CSimpleFrame* m_scrollChild;
    /* 02A4 */ C2Vector m_scrollRange;
    /* 02AC */ C2Vector m_scrollOffset;
    /* 02B4 */ FrameScript_Object::ScriptIx m_onHorizontalScroll;
    /* 02BC */ FrameScript_Object::ScriptIx m_onVerticalScroll;
    /* 02C4 */ FrameScript_Object::ScriptIx m_onScrollRangeChanged;

    virtual ~CSimpleScrollFrame();
    virtual int32_t GetScriptMetaTable();
    virtual FrameScript_Object::ScriptIx* GetScriptByName(const char* name, FrameScript_Object::ScriptData& data);
    virtual bool IsA(int32_t type);
    virtual bool IsA(const char* typeName);
    virtual const char* GetObjectTypeName();
    virtual void GetScriptTime(lua_State* L, double* time, uint32_t* count, int32_t recurse);
    virtual bool GetBoundsRect(CRect& rect);
    virtual void OnLayerUpdate(float elapsedSec);
    virtual void OnFrameRender(CRenderBatch* batch, uint32_t layer);
    virtual void OnFrameSizeChanged(float width, float height);
    virtual void UnparentFrame(CSimpleFrame* child);
    virtual void LoadXML(XMLNode* node, CStatus* status);

    CSimpleScrollFrame(CSimpleFrame* parent);
    void RunOnHorizontalScrollScript();
    void RunOnScrollRangeChangedScript();
    void RunOnVerticalScrollScript();
    void SetHorizontalScroll(float offset);
    void SetScrollChild(CSimpleFrame* child);
    void SetVerticalScroll(float offset);
    void UpdateScrollChild();
    void UpdateScrollChildRect(float width, float height);
};

#endif
