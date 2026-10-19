#ifndef WOW_SOURCE_DB_REC_MAILTEMPLATEREC_H
#define WOW_SOURCE_DB_REC_MAILTEMPLATEREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class MailTemplateRec {
    public:
    static const int32_t s_numColumns = 35;
    static const int32_t s_rowSize = 140;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_subject;
    /* 0008 */ const char* m_body;

    MailTemplateRec();
    ~MailTemplateRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
