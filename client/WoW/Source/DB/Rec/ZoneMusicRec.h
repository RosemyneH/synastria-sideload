#ifndef WOW_SOURCE_DB_REC_ZONEMUSICREC_H
#define WOW_SOURCE_DB_REC_ZONEMUSICREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class ZoneMusicRec {
    public:
    static const int32_t s_numColumns = 8;
    static const int32_t s_rowSize = 32;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_setName;
    /* 0008 */ int32_t m_silenceIntervalMin [2];
    /* 0010 */ int32_t m_silenceIntervalMax [2];
    /* 0018 */ int32_t m_sounds [2];

    ZoneMusicRec();
    ~ZoneMusicRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
