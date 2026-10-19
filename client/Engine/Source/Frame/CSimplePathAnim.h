#ifndef ENGINE_SOURCE_FRAME_CSIMPLEPATHANIM_H
#define ENGINE_SOURCE_FRAME_CSIMPLEPATHANIM_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Frame/CSimpleAnim.h"
#include "Engine/Source/Frame/CSimpleControlPoint.h"
#include "Engine/Source/Storm/H/stpl.h"

class CSimpleAnimGroup;
class CStatus;
class XMLNode;

class CSimplePathAnim : public CSimpleAnim {
    public:
    static int32_t s_metatable;
    static int32_t s_objectType;
    static const char* s_objectTypeName;

    static int32_t GetObjectType();
    static void CreateScriptMetaTable();
    static void DestroyScriptMetaTable();
    static void RegisterScriptMethods(lua_State* L);

    /* 0098 */ TSExplicitList<CSimpleControlPoint, offsetof(CSimpleControlPoint, m_link)> m_controlPoints;
    /* 00A4 */ TSGrowableArray<CSimpleControlPoint*> m_sortedPoints;
    /* 00B4 */ int8_t m_curve;

    virtual ~CSimplePathAnim();
    virtual int32_t GetScriptMetaTable();
    virtual bool IsA(int32_t type);
    virtual bool IsA(const char* typeName);
    virtual const char* GetObjectTypeName();
    virtual void LoadXML(XMLNode* node, CStatus* status);
    virtual void PostLoadXML(XMLNode* node, CStatus* status);
    virtual void Apply(float progress);
    virtual void ApplyInverse(float progress);

    CSimplePathAnim(CSimpleAnimGroup* group);
    void BuildSpline();
    int32_t FindControlPoint(int32_t order);
    CSimpleControlPoint* GetControlPoint(uint32_t index);
    int32_t GetMaxControlPointOrder();
    void LoadXML_ControlPoint(XMLNode* node, CStatus* status);
    void NotifyControlPointAdded(CSimpleControlPoint* point);
    void NotifyControlPointRemoved(CSimpleControlPoint* point);
    void NotifyOrderChanged(CSimpleControlPoint* point);
    int32_t RegisterSplineControlPoint(CSimpleControlPoint* point);
    void UpdateControlPointPositions();
};

#endif
