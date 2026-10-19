#ifndef WOW_SOURCE_DB_REC_CREATUREDISPLAYINFOREC_H
#define WOW_SOURCE_DB_REC_CREATUREDISPLAYINFOREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class CreatureDisplayInfoRec {
    public:
    static const int32_t s_numColumns = 16;
    static const int32_t s_rowSize = 64;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_modelID;
    /* 0008 */ int32_t m_soundID;
    /* 000C */ int32_t m_extendedDisplayInfoID;
    /* 0010 */ float m_creatureModelScale;
    /* 0014 */ int32_t m_creatureModelAlpha;
    /* 0018 */ const char* m_textureVariation [3];
    /* 0024 */ const char* m_portraitTextureName;
    /* 0028 */ int32_t m_sizeClass;
    /* 002C */ int32_t m_bloodID;
    /* 0030 */ int32_t m_npcsoundID;
    /* 0034 */ int32_t m_particleColorID;
    /* 0038 */ int32_t m_creatureGeosetData;
    /* 003C */ int32_t m_objectEffectPackageID;

    CreatureDisplayInfoRec();
    ~CreatureDisplayInfoRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
