#ifndef WOW_SOURCE_DB_REC_RANDPROPPOINTSREC_H
#define WOW_SOURCE_DB_REC_RANDPROPPOINTSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class RandPropPointsRec {
    public:
    static const int32_t s_numColumns = 16;
    static const int32_t s_rowSize = 64;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_epic [5];
    /* 0018 */ int32_t m_superior [5];
    /* 002C */ int32_t m_good [5];

    RandPropPointsRec();
    ~RandPropPointsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
