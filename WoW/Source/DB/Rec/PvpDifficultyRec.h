#ifndef WOW_SOURCE_DB_REC_PVPDIFFICULTYREC_H
#define WOW_SOURCE_DB_REC_PVPDIFFICULTYREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class PvpDifficultyRec {
    public:
    static const int32_t s_numColumns = 6;
    static const int32_t s_rowSize = 24;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_mapID;
    /* 0008 */ int32_t m_rangeIndex;
    /* 000C */ int32_t m_minLevel;
    /* 0010 */ int32_t m_maxLevel;
    /* 0014 */ int32_t m_difficulty;

    PvpDifficultyRec();
    ~PvpDifficultyRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
