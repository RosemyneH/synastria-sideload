#ifndef WOW_SOURCE_DB_REC_FOOTSTEPTERRAINLOOKUPREC_H
#define WOW_SOURCE_DB_REC_FOOTSTEPTERRAINLOOKUPREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class FootstepTerrainLookupRec {
    public:
    static const int32_t s_numColumns = 5;
    static const int32_t s_rowSize = 20;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_creatureFootstepID;
    /* 0008 */ int32_t m_terrainSoundID;
    /* 000C */ int32_t m_soundID;
    /* 0010 */ int32_t m_soundIdsplash;

    FootstepTerrainLookupRec();
    ~FootstepTerrainLookupRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
