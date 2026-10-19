#ifndef WOW_SOURCE_DB_REC_WORLDMAPTRANSFORMSREC_H
#define WOW_SOURCE_DB_REC_WORLDMAPTRANSFORMSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class WorldMapTransformsRec {
    public:
    static const int32_t s_numColumns = 10;
    static const int32_t s_rowSize = 40;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_mapID;
    /* 0008 */ float m_regionMin [2];
    /* 0010 */ float m_regionMax [2];
    /* 0018 */ int32_t m_newMapID;
    /* 001C */ float m_regionOffset [2];
    /* 0024 */ int32_t m_newDungeonMapID;

    WorldMapTransformsRec();
    ~WorldMapTransformsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
