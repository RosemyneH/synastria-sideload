#ifndef WOW_SOURCE_DB_REC_LOCKREC_H
#define WOW_SOURCE_DB_REC_LOCKREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class LockRec {
    public:
    static const int32_t s_numColumns = 33;
    static const int32_t s_rowSize = 132;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_type [8];
    /* 0024 */ int32_t m_index [8];
    /* 0044 */ int32_t m_skill [8];
    /* 0064 */ int32_t m_action [8];

    LockRec();
    ~LockRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
