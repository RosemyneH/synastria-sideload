#ifndef WOW_SOURCE_DB_REC_PARTICLECOLORREC_H
#define WOW_SOURCE_DB_REC_PARTICLECOLORREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class ParticleColorRec {
    public:
    static const int32_t s_numColumns = 10;
    static const int32_t s_rowSize = 40;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_start [3];
    /* 0010 */ int32_t m_mid [3];
    /* 001C */ int32_t m_end [3];

    ParticleColorRec();
    ~ParticleColorRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
