#ifndef WOW_SOURCE_DB_REC_SKILLLINEREC_H
#define WOW_SOURCE_DB_REC_SKILLLINEREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SkillLineRec {
    public:
    static const int32_t s_numColumns = 56;
    static const int32_t s_rowSize = 224;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_categoryID;
    /* 0008 */ int32_t m_skillCostsID;
    /* 000C */ const char* m_displayName;
    /* 0010 */ const char* m_description;
    /* 0014 */ int32_t m_spellIconID;
    /* 0018 */ const char* m_alternateVerb;
    /* 001C */ int32_t m_canLink;

    SkillLineRec();
    ~SkillLineRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
