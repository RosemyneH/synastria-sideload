#ifndef WOW_SOURCE_DB_REC_SPAMMESSAGESREC_H
#define WOW_SOURCE_DB_REC_SPAMMESSAGESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SpamMessagesRec {
    public:
    static const int32_t s_numColumns = 2;
    static const int32_t s_rowSize = 8;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_text;

    SpamMessagesRec();
    ~SpamMessagesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
