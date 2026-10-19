#ifndef WOW_SOURCE_DB_REC_OVERRIDESPELLDATAREC_H
#define WOW_SOURCE_DB_REC_OVERRIDESPELLDATAREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class OverrideSpellDataRec {
    public:
    static const int32_t s_numColumns = 12;
    static const int32_t s_rowSize = 48;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_spells [10];
    /* 002C */ int32_t m_flags;

    OverrideSpellDataRec();
    ~OverrideSpellDataRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
