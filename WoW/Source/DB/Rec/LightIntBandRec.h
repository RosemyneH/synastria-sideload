#ifndef WOW_SOURCE_DB_REC_LIGHTINTBANDREC_H
#define WOW_SOURCE_DB_REC_LIGHTINTBANDREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class LightIntBandRec {
    public:
    static const int32_t s_numColumns = 34;
    static const int32_t s_rowSize = 136;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_num;
    /* 0008 */ int32_t m_time [16];
    /* 0048 */ int32_t m_data [16];

    LightIntBandRec();
    ~LightIntBandRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
