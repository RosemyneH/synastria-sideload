#ifndef WOW_SOURCE_OBJECT_GAMEOBJECTDEF_H
#define WOW_SOURCE_OBJECT_GAMEOBJECTDEF_H

#include <cstddef>
#include <cstdint>

class CGameObjectDef {
    public:
    struct TypeInfo {
        /* 0000 */ const char* m_name;
        /* 0004 */ int32_t m_numProps;
        /* 0008 */ const int32_t* m_props;
        /* 000C */ const void* m_propInfo;
    };

    static const TypeInfo s_typeInfo[36];

    static int32_t GetPropNum(int32_t typeId, int32_t prop);
};

#endif
