#ifndef WOW_SOURCE_DB_REC_NAMEGENREC_H
#define WOW_SOURCE_DB_REC_NAMEGENREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class NameGenRec {
    public:
    static const int32_t s_numColumns = 4;
    static const int32_t s_rowSize = 16;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;
    /* 0008 */ int32_t m_raceID;
    /* 000C */ int32_t m_sex;

    NameGenRec();
    ~NameGenRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
