#include "WoW/Source/DB/Rec/WorldSafeLocsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

WorldSafeLocsRec::WorldSafeLocsRec() {
}

WorldSafeLocsRec::~WorldSafeLocsRec() {
}

// VERIFY: 76e180ef auto
// OFFSET: 0x008BBEF0
const char* WorldSafeLocsRec::GetFilename() {
    return "DBFilesClient\\WorldSafeLocs.dbc";
}

// VERIFY: 1581896c auto
// OFFSET: 0x008BBF00
bool WorldSafeLocsRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t areaNameOffset[16];
    uint32_t areaNameFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t continentRead = SFileReadFile(file, &this->m_continent, sizeof(this->m_continent), nullptr, nullptr, 0);
    int32_t locRead0 = SFileReadFile(file, &this->m_loc[0], sizeof(this->m_loc[0]), nullptr, nullptr, 0);
    int32_t locRead1 = SFileReadFile(file, &this->m_loc[1], sizeof(this->m_loc[1]), nullptr, nullptr, 0);
    int32_t locRead2 = SFileReadFile(file, &this->m_loc[2], sizeof(this->m_loc[2]), nullptr, nullptr, 0);
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
        | !continentRead
        | !locRead0
        | !locRead1
        | !locRead2
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
