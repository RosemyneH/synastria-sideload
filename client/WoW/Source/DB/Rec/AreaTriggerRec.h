#ifndef WOW_SOURCE_DB_REC_AREATRIGGERREC_H
#define WOW_SOURCE_DB_REC_AREATRIGGERREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class AreaTriggerRec {
    public:
    static const int32_t s_numColumns = 10;
    static const int32_t s_rowSize = 40;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_continentID;
    /* 0008 */ float m_pos [3];
    /* 0014 */ float m_radius;
    /* 0018 */ float m_boxLength;
    /* 001C */ float m_boxWidth;
    /* 0020 */ float m_boxHeight;
    /* 0024 */ float m_boxYaw;

    AreaTriggerRec();
    ~AreaTriggerRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
