#include "WoW/Source/DB/Rec/WorldChunkSoundsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

WorldChunkSoundsRec::WorldChunkSoundsRec() {
}

WorldChunkSoundsRec::~WorldChunkSoundsRec() {
}

// VERIFY: 5575c9af auto
// OFFSET: 0x008BD0B0
const char* WorldChunkSoundsRec::GetFilename() {
    return "DBFilesClient\\WorldChunkSounds.dbc";
}

bool WorldChunkSoundsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t chunkXRead = SFileReadFile(file, &this->m_chunkX, sizeof(this->m_chunkX), nullptr, nullptr, 0);
    int32_t chunkYRead = SFileReadFile(file, &this->m_chunkY, sizeof(this->m_chunkY), nullptr, nullptr, 0);
    int32_t subchunkXRead = SFileReadFile(file, &this->m_subchunkX, sizeof(this->m_subchunkX), nullptr, nullptr, 0);
    int32_t subchunkYRead = SFileReadFile(file, &this->m_subchunkY, sizeof(this->m_subchunkY), nullptr, nullptr, 0);
    int32_t zoneIntroMusicIDRead = SFileReadFile(file, &this->m_zoneIntroMusicID, sizeof(this->m_zoneIntroMusicID), nullptr, nullptr, 0);
    int32_t zoneMusicIDRead = SFileReadFile(file, &this->m_zoneMusicID, sizeof(this->m_zoneMusicID), nullptr, nullptr, 0);
    int32_t soundAmbienceIDRead = SFileReadFile(file, &this->m_soundAmbienceID, sizeof(this->m_soundAmbienceID), nullptr, nullptr, 0);
    int32_t soundProviderPreferencesIDRead = SFileReadFile(file, &this->m_soundProviderPreferencesID, sizeof(this->m_soundProviderPreferencesID), nullptr, nullptr, 0);

    if (!idRead
        | !chunkXRead
        | !chunkYRead
        | !subchunkXRead
        | !subchunkYRead
        | !zoneIntroMusicIDRead
        | !zoneMusicIDRead
        | !soundAmbienceIDRead
        | !soundProviderPreferencesIDRead) {
        return false;
    }

    return true;
}
