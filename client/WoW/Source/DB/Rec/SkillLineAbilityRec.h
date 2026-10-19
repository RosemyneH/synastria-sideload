#ifndef WOW_SOURCE_DB_REC_SKILLLINEABILITYREC_H
#define WOW_SOURCE_DB_REC_SKILLLINEABILITYREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SkillLineAbilityRec {
    public:
    static const int32_t s_numColumns = 14;
    static const int32_t s_rowSize = 56;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_skillLine;
    /* 0008 */ int32_t m_spell;
    /* 000C */ int32_t m_raceMask;
    /* 0010 */ int32_t m_classMask;
    /* 0014 */ int32_t m_excludeRace;
    /* 0018 */ int32_t m_excludeClass;
    /* 001C */ int32_t m_minSkillLineRank;
    /* 0020 */ int32_t m_supercededBySpell;
    /* 0024 */ int32_t m_acquireMethod;
    /* 0028 */ int32_t m_trivialSkillLineRankHigh;
    /* 002C */ int32_t m_trivialSkillLineRankLow;
    /* 0030 */ int32_t m_characterPoints [2];

    SkillLineAbilityRec();
    ~SkillLineAbilityRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
