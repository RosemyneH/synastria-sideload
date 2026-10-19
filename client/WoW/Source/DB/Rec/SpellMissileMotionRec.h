#ifndef WOW_SOURCE_DB_REC_SPELLMISSILEMOTIONREC_H
#define WOW_SOURCE_DB_REC_SPELLMISSILEMOTIONREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SpellMissileMotionRec {
    public:
    static const int32_t s_numColumns = 5;
    static const int32_t s_rowSize = 20;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;
    /* 0008 */ const char* m_scriptBody;
    /* 000C */ int32_t m_flags;
    /* 0010 */ int32_t m_missileCount;

    SpellMissileMotionRec();
    ~SpellMissileMotionRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
