#ifndef WOW_SOURCE_DB_REC_DUNGEONENCOUNTERREC_H
#define WOW_SOURCE_DB_REC_DUNGEONENCOUNTERREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class DungeonEncounterRec {
    public:
    static const int32_t s_numColumns = 23;
    static const int32_t s_rowSize = 92;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_mapID;
    /* 0008 */ int32_t m_difficulty;
    /* 000C */ int32_t m_orderIndex;
    /* 0010 */ int32_t m_bit;
    /* 0014 */ const char* m_name;
    /* 0018 */ int32_t m_spellIconID;

    DungeonEncounterRec();
    ~DungeonEncounterRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
