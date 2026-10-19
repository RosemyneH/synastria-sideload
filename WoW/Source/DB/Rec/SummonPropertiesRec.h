#ifndef WOW_SOURCE_DB_REC_SUMMONPROPERTIESREC_H
#define WOW_SOURCE_DB_REC_SUMMONPROPERTIESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SummonPropertiesRec {
    public:
    static const int32_t s_numColumns = 6;
    static const int32_t s_rowSize = 24;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_control;
    /* 0008 */ int32_t m_faction;
    /* 000C */ int32_t m_title;
    /* 0010 */ int32_t m_slot;
    /* 0014 */ int32_t m_flags;

    SummonPropertiesRec();
    ~SummonPropertiesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
