#ifndef ENGINE_SOURCE_FRAME_CSIMPLEROTATIONANIM_H
#define ENGINE_SOURCE_FRAME_CSIMPLEROTATIONANIM_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Frame/CSimpleAnim.h"
#include "Engine/Source/Frame/Types.h"
#include "Engine/Source/Tempest/C2Vector.h"

class CSimpleAnimGroup;
class CStatus;
class XMLNode;

class CSimpleRotationAnim : public CSimpleAnim {
    public:
    static int32_t s_metatable;
    static int32_t s_objectType;
    static const char* s_objectTypeName;

    static int32_t GetObjectType();
    static void CreateScriptMetaTable();
    static void DestroyScriptMetaTable();
    static void RegisterScriptMethods(lua_State* L);

    /* 0098 */ FRAMEPOINT m_origin;
    /* 009C */ C2Vector m_originOffset;
    /* 00A4 */ float m_radians;

    virtual ~CSimpleRotationAnim();
    virtual int32_t GetScriptMetaTable();
    virtual bool IsA(int32_t type);
    virtual bool IsA(const char* typeName);
    virtual const char* GetObjectTypeName();
    virtual void LoadXML(XMLNode* node, CStatus* status);
    virtual void Apply(float progress);
    virtual void ApplyInverse(float progress);

    CSimpleRotationAnim(CSimpleAnimGroup* group);
};

#endif
