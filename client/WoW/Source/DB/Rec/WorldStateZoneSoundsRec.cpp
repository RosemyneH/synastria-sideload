#include "WoW/Source/DB/Rec/WorldStateZoneSoundsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

WorldStateZoneSoundsRec::WorldStateZoneSoundsRec() {
}

WorldStateZoneSoundsRec::~WorldStateZoneSoundsRec() {
}

// VERIFY: 294489af auto
// OFFSET: 0x008BCF90
const char* WorldStateZoneSoundsRec::GetFilename() {
    return "DBFilesClient\\WorldStateZoneSounds.dbc";
}

bool WorldStateZoneSoundsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t worldStateIDRead = SFileReadFile(file, &this->m_worldStateID, sizeof(this->m_worldStateID), nullptr, nullptr, 0);
    int32_t worldStateValueRead = SFileReadFile(file, &this->m_worldStateValue, sizeof(this->m_worldStateValue), nullptr, nullptr, 0);
    int32_t areaIDRead = SFileReadFile(file, &this->m_areaID, sizeof(this->m_areaID), nullptr, nullptr, 0);
    int32_t wmoareaIDRead = SFileReadFile(file, &this->m_wmoareaID, sizeof(this->m_wmoareaID), nullptr, nullptr, 0);
    int32_t zoneIntroMusicIDRead = SFileReadFile(file, &this->m_zoneIntroMusicID, sizeof(this->m_zoneIntroMusicID), nullptr, nullptr, 0);
    int32_t zoneMusicIDRead = SFileReadFile(file, &this->m_zoneMusicID, sizeof(this->m_zoneMusicID), nullptr, nullptr, 0);
    int32_t soundAmbienceIDRead = SFileReadFile(file, &this->m_soundAmbienceID, sizeof(this->m_soundAmbienceID), nullptr, nullptr, 0);
    int32_t soundProviderPreferencesIDRead = SFileReadFile(file, &this->m_soundProviderPreferencesID, sizeof(this->m_soundProviderPreferencesID), nullptr, nullptr, 0);

    if (!worldStateIDRead
        | !worldStateValueRead
        | !areaIDRead
        | !wmoareaIDRead
        | !zoneIntroMusicIDRead
        | !zoneMusicIDRead
        | !soundAmbienceIDRead
        | !soundProviderPreferencesIDRead) {
        return false;
    }

    return true;
}
