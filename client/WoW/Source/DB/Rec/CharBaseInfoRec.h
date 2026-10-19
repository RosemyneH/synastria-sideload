#ifndef WOW_SOURCE_DB_REC_CHARBASEINFOREC_H
#define WOW_SOURCE_DB_REC_CHARBASEINFOREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class CharBaseInfoRec {
    public:
    static const int32_t s_numColumns = 2;
    static const int32_t s_rowSize = 2;
    static const bool s_generatedID = true;

    /* 0000 */ int8_t m_raceID;
    /* 0001 */ int8_t m_classID;
    /* 0004 */ int32_t m_generatedID;

    CharBaseInfoRec();
    ~CharBaseInfoRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
