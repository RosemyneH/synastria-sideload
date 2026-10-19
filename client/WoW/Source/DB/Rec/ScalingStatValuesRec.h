#ifndef WOW_SOURCE_DB_REC_SCALINGSTATVALUESREC_H
#define WOW_SOURCE_DB_REC_SCALINGSTATVALUESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class ScalingStatValuesRec {
    public:
    static const int32_t s_numColumns = 24;
    static const int32_t s_rowSize = 96;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_charlevel;
    /* 0008 */ int32_t m_shoulderBudget;
    /* 000C */ int32_t m_trinketBudget;
    /* 0010 */ int32_t m_weaponBudget1h;
    /* 0014 */ int32_t m_rangedBudget;
    /* 0018 */ int32_t m_clothShoulderArmor;
    /* 001C */ int32_t m_leatherShoulderArmor;
    /* 0020 */ int32_t m_mailShoulderArmor;
    /* 0024 */ int32_t m_plateShoulderArmor;
    /* 0028 */ int32_t m_weaponDps1h;
    /* 002C */ int32_t m_weaponDps2h;
    /* 0030 */ int32_t m_spellcasterDps1h;
    /* 0034 */ int32_t m_spellcasterDps2h;
    /* 0038 */ int32_t m_rangedDps;
    /* 003C */ int32_t m_wandDps;
    /* 0040 */ int32_t m_spellPower;
    /* 0044 */ int32_t m_primaryBudget;
    /* 0048 */ int32_t m_tertiaryBudget;
    /* 004C */ int32_t m_clothCloakArmor;
    /* 0050 */ int32_t m_clothChestArmor;
    /* 0054 */ int32_t m_leatherChestArmor;
    /* 0058 */ int32_t m_mailChestArmor;
    /* 005C */ int32_t m_plateChestArmor;

    ScalingStatValuesRec();
    ~ScalingStatValuesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
