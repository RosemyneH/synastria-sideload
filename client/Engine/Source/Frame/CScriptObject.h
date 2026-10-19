#ifndef ENGINE_SOURCE_FRAME_CSCRIPTOBJECT_H
#define ENGINE_SOURCE_FRAME_CSCRIPTOBJECT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Base/RCString.h"
#include "Engine/Source/FrameScript/FrameScript_Object.h"

class CScriptObject : public FrameScript_Object {
    public:
    static int32_t s_objectType;
    static const char* s_objectTypeName;

    static int32_t GetObjectType();
    static CScriptObject* GetScriptObjectByName(const char* name, int32_t type);
    static void RegisterScriptMethods(lua_State* L);

    /* 0014 */ RCString m_name;

    virtual ~CScriptObject();
    virtual const char* GetName();
    virtual CScriptObject* GetScriptObjectParent() = 0;
    virtual bool IsA(int32_t type);
    virtual bool IsA(const char* typeName);
    virtual const char* GetObjectTypeName();

    CScriptObject();
    void CreateName(const char* source, char* dest, uint32_t destSize);
    void SetName(const char* name);
};

CScriptObject* FrameScript_GetObjectThis(lua_State* L, int32_t type);

#endif
