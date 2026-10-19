#ifndef WOW_SOURCE_DB_REC_SPELLRADIUSREC_H
#define WOW_SOURCE_DB_REC_SPELLRADIUSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SpellRadiusRec {
    public:
    static const int32_t s_numColumns = 4;
    static const int32_t s_rowSize = 16;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ float m_radius;
    /* 0008 */ float m_radiusPerLevel;
    /* 000C */ float m_radiusMax;

    SpellRadiusRec();
    ~SpellRadiusRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
