#ifndef WOW_SOURCE_DB_REC_MAPDIFFICULTYREC_H
#define WOW_SOURCE_DB_REC_MAPDIFFICULTYREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class MapDifficultyRec {
    public:
    static const int32_t s_numColumns = 23;
    static const int32_t s_rowSize = 92;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_mapID;
    /* 0008 */ int32_t m_difficulty;
    /* 000C */ const char* m_message;
    /* 0010 */ int32_t m_raidDuration;
    /* 0014 */ int32_t m_maxPlayers;
    /* 0018 */ const char* m_difficultystring;

    MapDifficultyRec();
    ~MapDifficultyRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
