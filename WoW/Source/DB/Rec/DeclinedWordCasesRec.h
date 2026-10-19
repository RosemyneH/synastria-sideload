#ifndef WOW_SOURCE_DB_REC_DECLINEDWORDCASESREC_H
#define WOW_SOURCE_DB_REC_DECLINEDWORDCASESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class DeclinedWordCasesRec {
    public:
    static const int32_t s_numColumns = 4;
    static const int32_t s_rowSize = 16;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_declinedWordID;
    /* 0008 */ int32_t m_caseIndex;
    /* 000C */ const char* m_declinedWord;

    DeclinedWordCasesRec();
    ~DeclinedWordCasesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
