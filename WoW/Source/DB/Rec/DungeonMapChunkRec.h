#ifndef WOW_SOURCE_DB_REC_DUNGEONMAPCHUNKREC_H
#define WOW_SOURCE_DB_REC_DUNGEONMAPCHUNKREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class DungeonMapChunkRec {
    public:
    static const int32_t s_numColumns = 5;
    static const int32_t s_rowSize = 20;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_mapID;
    /* 0008 */ int32_t m_wmogroupID;
    /* 000C */ int32_t m_dungeonMapID;
    /* 0010 */ float m_minZ;

    DungeonMapChunkRec();
    ~DungeonMapChunkRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
