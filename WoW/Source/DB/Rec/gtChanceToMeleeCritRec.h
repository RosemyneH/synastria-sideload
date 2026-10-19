#ifndef WOW_SOURCE_DB_REC_GTCHANCETOMELEECRITREC_H
#define WOW_SOURCE_DB_REC_GTCHANCETOMELEECRITREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class gtChanceToMeleeCritRec {
    public:
    static const int32_t s_numColumns = 1;
    static const int32_t s_rowSize = 4;
    static const bool s_generatedID = true;

    /* 0000 */ float m_data;
    /* 0004 */ int32_t m_generatedID;

    gtChanceToMeleeCritRec();
    ~gtChanceToMeleeCritRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
