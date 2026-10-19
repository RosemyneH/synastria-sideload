#ifndef WOW_SOURCE_DB_REC_WORLDSTATEZONESOUNDSREC_H
#define WOW_SOURCE_DB_REC_WORLDSTATEZONESOUNDSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class WorldStateZoneSoundsRec {
    public:
    static const int32_t s_numColumns = 8;
    static const int32_t s_rowSize = 32;
    static const bool s_generatedID = true;

    /* 0000 */ int32_t m_worldStateID;
    /* 0004 */ int32_t m_worldStateValue;
    /* 0008 */ int32_t m_areaID;
    /* 000C */ int32_t m_wmoareaID;
    /* 0010 */ int32_t m_zoneIntroMusicID;
    /* 0014 */ int32_t m_zoneMusicID;
    /* 0018 */ int32_t m_soundAmbienceID;
    /* 001C */ int32_t m_soundProviderPreferencesID;
    /* 0020 */ int32_t m_generatedID;

    WorldStateZoneSoundsRec();
    ~WorldStateZoneSoundsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
