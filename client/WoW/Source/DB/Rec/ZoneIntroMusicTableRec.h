#ifndef WOW_SOURCE_DB_REC_ZONEINTROMUSICTABLEREC_H
#define WOW_SOURCE_DB_REC_ZONEINTROMUSICTABLEREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class ZoneIntroMusicTableRec {
    public:
    static const int32_t s_numColumns = 5;
    static const int32_t s_rowSize = 20;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;
    /* 0008 */ int32_t m_soundID;
    /* 000C */ int32_t m_priority;
    /* 0010 */ int32_t m_minDelayMinutes;

    ZoneIntroMusicTableRec();
    ~ZoneIntroMusicTableRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
