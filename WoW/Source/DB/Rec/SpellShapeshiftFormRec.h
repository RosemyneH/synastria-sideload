#ifndef WOW_SOURCE_DB_REC_SPELLSHAPESHIFTFORMREC_H
#define WOW_SOURCE_DB_REC_SPELLSHAPESHIFTFORMREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SpellShapeshiftFormRec {
    public:
    static const int32_t s_numColumns = 35;
    static const int32_t s_rowSize = 140;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_bonusActionBar;
    /* 0008 */ const char* m_name;
    /* 000C */ int32_t m_flags;
    /* 0010 */ int32_t m_creatureType;
    /* 0014 */ int32_t m_attackIconID;
    /* 0018 */ int32_t m_combatRoundTime;
    /* 001C */ int32_t m_creatureDisplayID [4];
    /* 002C */ int32_t m_presetSpellID [8];

    SpellShapeshiftFormRec();
    ~SpellShapeshiftFormRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
