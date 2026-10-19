#ifndef WOW_SOURCE_DB_REC_SPELLITEMENCHANTMENTREC_H
#define WOW_SOURCE_DB_REC_SPELLITEMENCHANTMENTREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SpellItemEnchantmentRec {
    public:
    static const int32_t s_numColumns = 38;
    static const int32_t s_rowSize = 152;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_charges;
    /* 0008 */ int32_t m_effect [3];
    /* 0014 */ int32_t m_effectPointsMin [3];
    /* 0020 */ int32_t m_effectPointsMax [3];
    /* 002C */ int32_t m_effectArg [3];
    /* 0038 */ const char* m_name;
    /* 003C */ int32_t m_itemVisual;
    /* 0040 */ int32_t m_flags;
    /* 0044 */ int32_t m_srcItemID;
    /* 0048 */ int32_t m_conditionID;
    /* 004C */ int32_t m_requiredSkillID;
    /* 0050 */ int32_t m_requiredSkillRank;
    /* 0054 */ int32_t m_minLevel;

    SpellItemEnchantmentRec();
    ~SpellItemEnchantmentRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
