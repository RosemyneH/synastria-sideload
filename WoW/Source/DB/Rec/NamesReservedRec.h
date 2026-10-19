#ifndef WOW_SOURCE_DB_REC_NAMESRESERVEDREC_H
#define WOW_SOURCE_DB_REC_NAMESRESERVEDREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class NamesReservedRec {
    public:
    static const int32_t s_numColumns = 3;
    static const int32_t s_rowSize = 12;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;
    /* 0008 */ int32_t m_language;

    NamesReservedRec();
    ~NamesReservedRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
