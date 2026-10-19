#ifndef WOW_SOURCE_DB_REC_GAMEOBJECTDISPLAYINFOREC_H
#define WOW_SOURCE_DB_REC_GAMEOBJECTDISPLAYINFOREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class GameObjectDisplayInfoRec {
    public:
    static const int32_t s_numColumns = 19;
    static const int32_t s_rowSize = 76;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_modelName;
    /* 0008 */ int32_t m_sound [10];
    /* 0030 */ float m_geoBoxMin [3];
    /* 003C */ float m_geoBoxMax [3];
    /* 0048 */ int32_t m_objectEffectPackageID;

    GameObjectDisplayInfoRec();
    ~GameObjectDisplayInfoRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
