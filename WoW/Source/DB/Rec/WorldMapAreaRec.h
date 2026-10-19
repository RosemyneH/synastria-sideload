#ifndef WOW_SOURCE_DB_REC_WORLDMAPAREAREC_H
#define WOW_SOURCE_DB_REC_WORLDMAPAREAREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class WorldMapAreaRec {
    public:
    static const int32_t s_numColumns = 11;
    static const int32_t s_rowSize = 44;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_mapID;
    /* 0008 */ int32_t m_areaID;
    /* 000C */ const char* m_areaName;
    /* 0010 */ float m_locLeft;
    /* 0014 */ float m_locRight;
    /* 0018 */ float m_locTop;
    /* 001C */ float m_locBottom;
    /* 0020 */ int32_t m_displayMapID;
    /* 0024 */ int32_t m_defaultDungeonFloor;
    /* 0028 */ int32_t m_parentWorldMapID;

    WorldMapAreaRec();
    ~WorldMapAreaRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
