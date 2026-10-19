#ifndef WOW_SOURCE_DB_REC_SKILLRACECLASSINFOREC_H
#define WOW_SOURCE_DB_REC_SKILLRACECLASSINFOREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SkillRaceClassInfoRec {
    public:
    static const int32_t s_numColumns = 8;
    static const int32_t s_rowSize = 32;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_skillID;
    /* 0008 */ int32_t m_raceMask;
    /* 000C */ int32_t m_classMask;
    /* 0010 */ int32_t m_flags;
    /* 0014 */ int32_t m_minLevel;
    /* 0018 */ int32_t m_skillTierID;
    /* 001C */ int32_t m_skillCostIndex;

    SkillRaceClassInfoRec();
    ~SkillRaceClassInfoRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
