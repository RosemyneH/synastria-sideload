#ifndef WOW_SOURCE_DB_REC_SPELLVISUALKITREC_H
#define WOW_SOURCE_DB_REC_SPELLVISUALKITREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SpellVisualKitRec {
    public:
    static const int32_t s_numColumns = 38;
    static const int32_t s_rowSize = 152;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_startAnimID;
    /* 0008 */ int32_t m_animID;
    /* 000C */ int32_t m_headEffect;
    /* 0010 */ int32_t m_chestEffect;
    /* 0014 */ int32_t m_baseEffect;
    /* 0018 */ int32_t m_leftHandEffect;
    /* 001C */ int32_t m_rightHandEffect;
    /* 0020 */ int32_t m_breathEffect;
    /* 0024 */ int32_t m_leftWeaponEffect;
    /* 0028 */ int32_t m_rightWeaponEffect;
    /* 002C */ int32_t m_specialEffect [3];
    /* 0038 */ int32_t m_worldEffect;
    /* 003C */ int32_t m_soundID;
    /* 0040 */ int32_t m_shakeID;
    /* 0044 */ int32_t m_charProc [4];
    /* 0054 */ float m_charParamZero [4];
    /* 0064 */ float m_charParamOne [4];
    /* 0074 */ float m_charParamTwo [4];
    /* 0084 */ float m_charParamThree [4];
    /* 0094 */ int32_t m_flags;

    SpellVisualKitRec();
    ~SpellVisualKitRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
