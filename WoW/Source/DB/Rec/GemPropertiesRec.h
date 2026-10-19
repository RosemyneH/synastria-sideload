#ifndef WOW_SOURCE_DB_REC_GEMPROPERTIESREC_H
#define WOW_SOURCE_DB_REC_GEMPROPERTIESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class GemPropertiesRec {
    public:
    static const int32_t s_numColumns = 5;
    static const int32_t s_rowSize = 20;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_enchantID;
    /* 0008 */ int32_t m_maxcountInv;
    /* 000C */ int32_t m_maxcountItem;
    /* 0010 */ int32_t m_type;

    GemPropertiesRec();
    ~GemPropertiesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
