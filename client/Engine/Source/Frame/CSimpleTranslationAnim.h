#ifndef ENGINE_SOURCE_FRAME_CSIMPLETRANSLATIONANIM_H
#define ENGINE_SOURCE_FRAME_CSIMPLETRANSLATIONANIM_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Frame/CSimpleAnim.h"
#include "Engine/Source/Tempest/C2Vector.h"

class CSimpleAnimGroup;
class CStatus;
class XMLNode;

class CSimpleTranslationAnim : public CSimpleAnim {
    public:
    static int32_t s_metatable;
    static int32_t s_objectType;
    static const char* s_objectTypeName;

    static int32_t GetObjectType();
    static void CreateScriptMetaTable();
    static void DestroyScriptMetaTable();
    static void RegisterScriptMethods(lua_State* L);

    /* 0098 */ C2Vector m_offset;

    virtual ~CSimpleTranslationAnim();
    virtual int32_t GetScriptMetaTable();
    virtual bool IsA(int32_t type);
    virtual bool IsA(const char* typeName);
    virtual const char* GetObjectTypeName();
    virtual void LoadXML(XMLNode* node, CStatus* status);
    virtual void Apply(float progress);
    virtual void ApplyInverse(float progress);

    CSimpleTranslationAnim(CSimpleAnimGroup* group);
    void SetOffset(const C2Vector& offset);
};

#endif
