#ifndef WOW_SOURCE_DB_REC_LOCKTYPEREC_H
#define WOW_SOURCE_DB_REC_LOCKTYPEREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class LockTypeRec {
    public:
    static const int32_t s_numColumns = 53;
    static const int32_t s_rowSize = 212;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;
    /* 0008 */ const char* m_resourceName;
    /* 000C */ const char* m_verb;
    /* 0010 */ const char* m_cursorName;

    LockTypeRec();
    ~LockTypeRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
