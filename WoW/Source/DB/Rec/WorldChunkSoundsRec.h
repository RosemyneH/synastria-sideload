#ifndef WOW_SOURCE_DB_REC_WORLDCHUNKSOUNDSREC_H
#define WOW_SOURCE_DB_REC_WORLDCHUNKSOUNDSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class WorldChunkSoundsRec {
    public:
    static const int32_t s_numColumns = 9;
    static const int32_t s_rowSize = 36;
    static const bool s_generatedID = true;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_chunkX;
    /* 0008 */ int32_t m_chunkY;
    /* 000C */ int32_t m_subchunkX;
    /* 0010 */ int32_t m_subchunkY;
    /* 0014 */ int32_t m_zoneIntroMusicID;
    /* 0018 */ int32_t m_zoneMusicID;
    /* 001C */ int32_t m_soundAmbienceID;
    /* 0020 */ int32_t m_soundProviderPreferencesID;
    /* 0024 */ int32_t m_generatedID;

    WorldChunkSoundsRec();
    ~WorldChunkSoundsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif
