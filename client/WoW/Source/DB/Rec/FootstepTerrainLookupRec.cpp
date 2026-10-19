#include "WoW/Source/DB/Rec/FootstepTerrainLookupRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

FootstepTerrainLookupRec::FootstepTerrainLookupRec() {
}

FootstepTerrainLookupRec::~FootstepTerrainLookupRec() {
}

// VERIFY: 26ae3b6e auto
// OFFSET: 0x008AF9F0
const char* FootstepTerrainLookupRec::GetFilename() {
    return "DBFilesClient\\FootstepTerrainLookup.dbc";
}

bool FootstepTerrainLookupRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t creatureFootstepIDRead = SFileReadFile(file, &this->m_creatureFootstepID, sizeof(this->m_creatureFootstepID), nullptr, nullptr, 0);
    int32_t terrainSoundIDRead = SFileReadFile(file, &this->m_terrainSoundID, sizeof(this->m_terrainSoundID), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, &this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);
    int32_t soundIdsplashRead = SFileReadFile(file, &this->m_soundIdsplash, sizeof(this->m_soundIdsplash), nullptr, nullptr, 0);

    if (!idRead
        | !creatureFootstepIDRead
        | !terrainSoundIDRead
        | !soundIDRead
        | !soundIdsplashRead) {
        return false;
    }

    return true;
}
