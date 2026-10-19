#ifndef WOW_SOURCE_DB_REC_EMOTESTEXTREC_H
#define WOW_SOURCE_DB_REC_EMOTESTEXTREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class EmotesTextRec {
    public:
    static const int32_t s_numColumns = 19;
    static const int32_t s_rowSize = 76;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;
    /* 0008 */ int32_t m_emoteID;
    /* 000C */ int32_t m_emoteText [16];

    EmotesTextRec();
    ~EmotesTextRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
