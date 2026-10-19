#ifndef WOW_SOURCE_DB_REC_AREATABLEREC_H
#define WOW_SOURCE_DB_REC_AREATABLEREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class AreaTableRec {
    public:
    static const int32_t s_numColumns = 36;
    static const int32_t s_rowSize = 144;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_continentID;
    /* 0008 */ int32_t m_parentAreaID;
    /* 000C */ int32_t m_areaBit;
    /* 0010 */ int32_t m_flags;
    /* 0014 */ int32_t m_soundProviderPref;
    /* 0018 */ int32_t m_soundProviderPrefUnderwater;
    /* 001C */ int32_t m_ambienceID;
    /* 0020 */ int32_t m_zoneMusic;
    /* 0024 */ int32_t m_introSound;
    /* 0028 */ int32_t m_explorationLevel;
    /* 002C */ const char* m_areaName;
    /* 0030 */ int32_t m_factionGroupMask;
    /* 0034 */ int32_t m_liquidTypeID [4];
    /* 0044 */ float m_minElevation;
    /* 0048 */ float m_ambientMultiplier;
    /* 004C */ int32_t m_lightID;

    AreaTableRec();
    ~AreaTableRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
