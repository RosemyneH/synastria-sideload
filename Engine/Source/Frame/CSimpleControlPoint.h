#ifndef ENGINE_SOURCE_FRAME_CSIMPLECONTROLPOINT_H
#define ENGINE_SOURCE_FRAME_CSIMPLECONTROLPOINT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Frame/CScriptObject.h"
#include "Engine/Source/Tempest/C2Vector.h"
#include "Engine/Source/Storm/H/stpl.h"

class CSimplePathAnim;
class CStatus;
class XMLNode;

class CSimpleControlPoint : public CScriptObject {
    public:
    static int32_t s_metatable;
    static int32_t s_objectType;
    static const char* s_objectTypeName;

    static int32_t GetObjectType();
    static void CreateScriptMetaTable();
    static void DestroyScriptMetaTable();
    static void RegisterScriptMethods(lua_State* L);

    /* 0020 */ TSLink<CSimpleControlPoint> m_link;
    /* 0028 */ CSimplePathAnim* m_parent;
    /* 002C */ C2Vector m_offset;
    /* 0034 */ float m_position;
    /* 0038 */ int8_t m_order;

    virtual ~CSimpleControlPoint();
    virtual int32_t GetScriptMetaTable();
    virtual bool IsA(int32_t type);
    virtual CScriptObject* GetScriptObjectParent();
    virtual bool IsA(const char* typeName);
    virtual const char* GetObjectTypeName();
    virtual void LoadXML(XMLNode* node, CStatus* status);

    CSimpleControlPoint(CSimplePathAnim* parent);
    bool ChangeParent(CSimplePathAnim* parent);
    void SetOffset(const C2Vector& offset);
    void SetOrder(int32_t order, bool notify, bool wrap);
};

#endif
