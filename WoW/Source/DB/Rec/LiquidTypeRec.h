#ifndef WOW_SOURCE_DB_REC_LIQUIDTYPEREC_H
#define WOW_SOURCE_DB_REC_LIQUIDTYPEREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class LiquidTypeRec {
    public:
    static const int32_t s_numColumns = 45;
    static const int32_t s_rowSize = 180;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;
    /* 0008 */ int32_t m_flags;
    /* 000C */ int32_t m_soundBank;
    /* 0010 */ int32_t m_soundID;
    /* 0014 */ int32_t m_spellID;
    /* 0018 */ float m_maxDarkenDepth;
    /* 001C */ float m_fogDarkenIntensity;
    /* 0020 */ float m_ambDarkenIntensity;
    /* 0024 */ float m_dirDarkenIntensity;
    /* 0028 */ int32_t m_lightID;
    /* 002C */ float m_particleScale;
    /* 0030 */ int32_t m_particleMovement;
    /* 0034 */ int32_t m_particleTexSlots;
    /* 0038 */ int32_t m_materialID;
    /* 003C */ const char* m_texture [6];
    /* 0054 */ int32_t m_color [2];
    /* 005C */ float m_float [18];
    /* 00A4 */ int32_t m_int [4];

    LiquidTypeRec();
    ~LiquidTypeRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
