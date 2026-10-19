#ifndef ENGINE_SOURCE_FRAME_CSIMPLEALPHAANIM_H
#define ENGINE_SOURCE_FRAME_CSIMPLEALPHAANIM_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Frame/CSimpleAnim.h"

class CSimpleAnimGroup;
class CStatus;
class XMLNode;

class CSimpleAlphaAnim : public CSimpleAnim {
    public:
    static int32_t s_metatable;
    static int32_t s_objectType;
    static const char* s_objectTypeName;

    static int32_t GetObjectType();
    static void CreateScriptMetaTable();
    static void DestroyScriptMetaTable();
    static void RegisterScriptMethods(lua_State* L);

    /* 0098 */ int16_t m_change;

    virtual ~CSimpleAlphaAnim();
    virtual int32_t GetScriptMetaTable();
    virtual bool IsA(int32_t type);
    virtual bool IsA(const char* typeName);
    virtual const char* GetObjectTypeName();
    virtual void LoadXML(XMLNode* node, CStatus* status);
    virtual void Apply(float progress);
    virtual void ApplyInverse(float progress);

    CSimpleAlphaAnim(CSimpleAnimGroup* group);
    void SetChange(float change);
};

#endif
