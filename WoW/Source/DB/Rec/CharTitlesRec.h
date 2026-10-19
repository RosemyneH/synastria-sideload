#ifndef WOW_SOURCE_DB_REC_CHARTITLESREC_H
#define WOW_SOURCE_DB_REC_CHARTITLESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class CharTitlesRec {
    public:
    static const int32_t s_numColumns = 37;
    static const int32_t s_rowSize = 148;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_conditionID;
    /* 0008 */ const char* m_name;
    /* 000C */ const char* m_name1;
    /* 0010 */ int32_t m_maskID;

    CharTitlesRec();
    ~CharTitlesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
