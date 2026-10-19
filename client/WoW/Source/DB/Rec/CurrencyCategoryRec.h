#ifndef WOW_SOURCE_DB_REC_CURRENCYCATEGORYREC_H
#define WOW_SOURCE_DB_REC_CURRENCYCATEGORYREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class CurrencyCategoryRec {
    public:
    static const int32_t s_numColumns = 19;
    static const int32_t s_rowSize = 76;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_flags;
    /* 0008 */ const char* m_name;

    CurrencyCategoryRec();
    ~CurrencyCategoryRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
