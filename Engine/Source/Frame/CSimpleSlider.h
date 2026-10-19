#ifndef ENGINE_SOURCE_FRAME_CSIMPLESLIDER_H
#define ENGINE_SOURCE_FRAME_CSIMPLESLIDER_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Base/CDataAllocator.h"
#include "Engine/Source/Frame/CSimpleFrame.h"
#include "Engine/Source/Frame/Types.h"
#include "Engine/Source/FrameScript/FrameScript_Object.h"

class CMouseEvent;
class CRect;
class CSimpleRegion;
class CSimpleTexture;
class CStatus;
class XMLNode;
struct lua_State;

class CSimpleSlider : public CSimpleFrame {
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

    /* 029C */ uint32_t m_changed : 1;
    /* 029C */ uint32_t m_rangeSet : 1;
    /* 029C */ uint32_t m_valueSet : 1;
    /* 029C */ uint32_t m_buttonDown : 1;
    /* 02A0 */ float m_baseValue;
    /* 02A4 */ float m_range;
    /* 02A8 */ float m_value;
    /* 02AC */ float m_valueStep;
    /* 02B0 */ CSimpleTexture* m_thumbTexture;
    /* 02B4 */ ORIENTATION m_orientation;
    /* 02B8 */ FrameScript_Object::ScriptIx m_onValueChanged;
    /* 02C0 */ FrameScript_Object::ScriptIx m_onMinMaxChanged;

    virtual ~CSimpleSlider();
    virtual int32_t GetScriptMetaTable();
    virtual FrameScript_Object::ScriptIx* GetScriptByName(const char* name, FrameScript_Object::ScriptData& data);
    virtual bool IsA(int32_t type);
    virtual bool IsA(const char* typeName);
    virtual const char* GetObjectTypeName();
    virtual void GetScriptTime(lua_State* L, double* time, uint32_t* count, int32_t recurse);
    virtual void UnregisterRegion(CSimpleRegion* region);
    virtual void OnLayerHide();
    virtual void OnLayerUpdate(float elapsedSec);
    virtual int32_t OnLayerTrackUpdate(const CMouseEvent& evt);
    virtual int32_t OnLayerMouseDown(const CMouseEvent& evt, const char* btn);
    virtual int32_t OnLayerMouseUp(const CMouseEvent& evt, const char* btn);
    virtual void LoadXML(XMLNode* node, CStatus* status);
    virtual void OnFrameSizeChanged(const CRect& rect);

    CSimpleSlider(CSimpleFrame* parent);
    void RunOnMinMaxChangedScript();
    void SetMinMaxValues(float min, float max);
    void SetOrientation(ORIENTATION orientation);
    int32_t SetThumbTexture(const char* fileName);
    void SetThumbTexture(CSimpleTexture* texture, uint32_t drawLayer);
    void SetValue(float value);
    void SetValueStep(float step);
    float SnapValueToStep(float value);
};

#endif
