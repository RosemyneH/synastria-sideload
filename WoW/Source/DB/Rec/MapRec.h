#ifndef WOW_SOURCE_DB_REC_MAPREC_H
#define WOW_SOURCE_DB_REC_MAPREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class MapRec {
    public:
    static const int32_t s_numColumns = 66;
    static const int32_t s_rowSize = 264;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_directory;
    /* 0008 */ int32_t m_instanceType;
    /* 000C */ int32_t m_flags;
    /* 0010 */ int32_t m_pvp;
    /* 0014 */ const char* m_mapName;
    /* 0018 */ int32_t m_areaTableID;
    /* 001C */ const char* m_mapDescription0;
    /* 0020 */ const char* m_mapDescription1;
    /* 0024 */ int32_t m_loadingScreenID;
    /* 0028 */ float m_minimapIconScale;
    /* 002C */ int32_t m_corpseMapID;
    /* 0030 */ float m_corpse [2];
    /* 0038 */ int32_t m_timeOfDayOverride;
    /* 003C */ int32_t m_expansionID;
    /* 0040 */ int32_t m_raidOffset;
    /* 0044 */ int32_t m_maxPlayers;

    MapRec();
    ~MapRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
