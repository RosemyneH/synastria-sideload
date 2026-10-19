#ifndef WOW_SOURCE_DB_REC_UISOUNDLOOKUPSREC_H
#define WOW_SOURCE_DB_REC_UISOUNDLOOKUPSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class UISoundLookupsRec {
    public:
    static const int32_t s_numColumns = 3;
    static const int32_t s_rowSize = 12;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_soundID;
    /* 0008 */ const char* m_soundName;

    UISoundLookupsRec();
    ~UISoundLookupsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
