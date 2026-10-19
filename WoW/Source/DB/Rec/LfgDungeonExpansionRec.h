#ifndef WOW_SOURCE_DB_REC_LFGDUNGEONEXPANSIONREC_H
#define WOW_SOURCE_DB_REC_LFGDUNGEONEXPANSIONREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class LfgDungeonExpansionRec {
    public:
    static const int32_t s_numColumns = 8;
    static const int32_t s_rowSize = 32;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_lfgID;
    /* 0008 */ int32_t m_expansionLevel;
    /* 000C */ int32_t m_randomID;
    /* 0010 */ int32_t m_hardLevelMin;
    /* 0014 */ int32_t m_hardLevelMax;
    /* 0018 */ int32_t m_targetLevelMin;
    /* 001C */ int32_t m_targetLevelMax;

    LfgDungeonExpansionRec();
    ~LfgDungeonExpansionRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
