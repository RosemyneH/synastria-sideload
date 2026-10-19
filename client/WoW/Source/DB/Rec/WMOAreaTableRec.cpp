#include "WoW/Source/DB/Rec/WMOAreaTableRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

WMOAreaTableRec::WMOAreaTableRec() {
}

WMOAreaTableRec::~WMOAreaTableRec() {
}

// VERIFY: ec555e47 auto
// OFFSET: 0x008BD8B0
const char* WMOAreaTableRec::GetFilename() {
    return "DBFilesClient\\WMOAreaTable.dbc";
}

// VERIFY: 5dc2c161 auto
// OFFSET: 0x008BD8C0
bool WMOAreaTableRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t areaNameOffset[16];
    uint32_t areaNameFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t wmoidRead = SFileReadFile(file, &this->m_wmoid, sizeof(this->m_wmoid), nullptr, nullptr, 0);
    int32_t nameSetIDRead = SFileReadFile(file, &this->m_nameSetID, sizeof(this->m_nameSetID), nullptr, nullptr, 0);
    int32_t wmogroupIDRead = SFileReadFile(file, &this->m_wmogroupID, sizeof(this->m_wmogroupID), nullptr, nullptr, 0);
    int32_t soundProviderPrefRead = SFileReadFile(file, &this->m_soundProviderPref, sizeof(this->m_soundProviderPref), nullptr, nullptr, 0);
    int32_t soundProviderPrefUnderwaterRead = SFileReadFile(file, &this->m_soundProviderPrefUnderwater, sizeof(this->m_soundProviderPrefUnderwater), nullptr, nullptr, 0);
    int32_t ambienceIDRead = SFileReadFile(file, &this->m_ambienceID, sizeof(this->m_ambienceID), nullptr, nullptr, 0);
    int32_t zoneMusicRead = SFileReadFile(file, &this->m_zoneMusic, sizeof(this->m_zoneMusic), nullptr, nullptr, 0);
    int32_t introSoundRead = SFileReadFile(file, &this->m_introSound, sizeof(this->m_introSound), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t areaTableIDRead = SFileReadFile(file, &this->m_areaTableID, sizeof(this->m_areaTableID), nullptr, nullptr, 0);
    int32_t areaNameRead0 = SFileReadFile(file, &areaNameOffset[0], sizeof(areaNameOffset[0]), nullptr, nullptr, 0);
    int32_t areaNameRead1 = SFileReadFile(file, &areaNameOffset[1], sizeof(areaNameOffset[1]), nullptr, nullptr, 0);
    int32_t areaNameRead2 = SFileReadFile(file, &areaNameOffset[2], sizeof(areaNameOffset[2]), nullptr, nullptr, 0);
    int32_t areaNameRead3 = SFileReadFile(file, &areaNameOffset[3], sizeof(areaNameOffset[3]), nullptr, nullptr, 0);
    int32_t areaNameRead4 = SFileReadFile(file, &areaNameOffset[4], sizeof(areaNameOffset[4]), nullptr, nullptr, 0);
    int32_t areaNameRead5 = SFileReadFile(file, &areaNameOffset[5], sizeof(areaNameOffset[5]), nullptr, nullptr, 0);
    int32_t areaNameRead6 = SFileReadFile(file, &areaNameOffset[6], sizeof(areaNameOffset[6]), nullptr, nullptr, 0);
    int32_t areaNameRead7 = SFileReadFile(file, &areaNameOffset[7], sizeof(areaNameOffset[7]), nullptr, nullptr, 0);
    int32_t areaNameRead8 = SFileReadFile(file, &areaNameOffset[8], sizeof(areaNameOffset[8]), nullptr, nullptr, 0);
    int32_t areaNameRead9 = SFileReadFile(file, &areaNameOffset[9], sizeof(areaNameOffset[9]), nullptr, nullptr, 0);
    int32_t areaNameRead10 = SFileReadFile(file, &areaNameOffset[10], sizeof(areaNameOffset[10]), nullptr, nullptr, 0);
    int32_t areaNameRead11 = SFileReadFile(file, &areaNameOffset[11], sizeof(areaNameOffset[11]), nullptr, nullptr, 0);
    int32_t areaNameRead12 = SFileReadFile(file, &areaNameOffset[12], sizeof(areaNameOffset[12]), nullptr, nullptr, 0);
    int32_t areaNameRead13 = SFileReadFile(file, &areaNameOffset[13], sizeof(areaNameOffset[13]), nullptr, nullptr, 0);
    int32_t areaNameRead14 = SFileReadFile(file, &areaNameOffset[14], sizeof(areaNameOffset[14]), nullptr, nullptr, 0);
    int32_t areaNameRead15 = SFileReadFile(file, &areaNameOffset[15], sizeof(areaNameOffset[15]), nullptr, nullptr, 0);
    int32_t areaNameFlagsRead = SFileReadFile(file, &areaNameFlags, sizeof(areaNameFlags), nullptr, nullptr, 0);

    if (!idRead
        | !wmoidRead
        | !nameSetIDRead
        | !wmogroupIDRead
        | !soundProviderPrefRead
        | !soundProviderPrefUnderwaterRead
        | !ambienceIDRead
        | !zoneMusicRead
        | !introSoundRead
        | !flagsRead
        | !areaTableIDRead
        | !areaNameRead0
        | !areaNameRead1
        | !areaNameRead2
        | !areaNameRead3
        | !areaNameRead4
        | !areaNameRead5
        | !areaNameRead6
        | !areaNameRead7
        | !areaNameRead8
        | !areaNameRead9
        | !areaNameRead10
        | !areaNameRead11
        | !areaNameRead12
        | !areaNameRead13
        | !areaNameRead14
        | !areaNameRead15
        | !areaNameFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_areaName = &stringBuffer[areaNameOffset[s_locale]];
    } else {
        this->m_areaName = "";
    }

    return true;
}
