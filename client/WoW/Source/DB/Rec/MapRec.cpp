#include "WoW/Source/DB/Rec/MapRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

MapRec::MapRec() {
}

MapRec::~MapRec() {
}

// VERIFY: ed5b3b7c auto
// OFFSET: 0x008B3AA0
const char* MapRec::GetFilename() {
    return "DBFilesClient\\Map.dbc";
}

// VERIFY: 33a8392a auto
// OFFSET: 0x008B3AB0
bool MapRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t directoryOffset;
    uint32_t mapNameOffset[16];
    uint32_t mapNameFlags;
    uint32_t mapDescription0Offset[16];
    uint32_t mapDescription0Flags;
    uint32_t mapDescription1Offset[16];
    uint32_t mapDescription1Flags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t directoryRead = SFileReadFile(file, &directoryOffset, sizeof(directoryOffset), nullptr, nullptr, 0);
    int32_t instanceTypeRead = SFileReadFile(file, &this->m_instanceType, sizeof(this->m_instanceType), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t pvpRead = SFileReadFile(file, &this->m_pvp, sizeof(this->m_pvp), nullptr, nullptr, 0);
    int32_t mapNameRead0 = SFileReadFile(file, &mapNameOffset[0], sizeof(mapNameOffset[0]), nullptr, nullptr, 0);
    int32_t mapNameRead1 = SFileReadFile(file, &mapNameOffset[1], sizeof(mapNameOffset[1]), nullptr, nullptr, 0);
    int32_t mapNameRead2 = SFileReadFile(file, &mapNameOffset[2], sizeof(mapNameOffset[2]), nullptr, nullptr, 0);
    int32_t mapNameRead3 = SFileReadFile(file, &mapNameOffset[3], sizeof(mapNameOffset[3]), nullptr, nullptr, 0);
    int32_t mapNameRead4 = SFileReadFile(file, &mapNameOffset[4], sizeof(mapNameOffset[4]), nullptr, nullptr, 0);
    int32_t mapNameRead5 = SFileReadFile(file, &mapNameOffset[5], sizeof(mapNameOffset[5]), nullptr, nullptr, 0);
    int32_t mapNameRead6 = SFileReadFile(file, &mapNameOffset[6], sizeof(mapNameOffset[6]), nullptr, nullptr, 0);
    int32_t mapNameRead7 = SFileReadFile(file, &mapNameOffset[7], sizeof(mapNameOffset[7]), nullptr, nullptr, 0);
    int32_t mapNameRead8 = SFileReadFile(file, &mapNameOffset[8], sizeof(mapNameOffset[8]), nullptr, nullptr, 0);
    int32_t mapNameRead9 = SFileReadFile(file, &mapNameOffset[9], sizeof(mapNameOffset[9]), nullptr, nullptr, 0);
    int32_t mapNameRead10 = SFileReadFile(file, &mapNameOffset[10], sizeof(mapNameOffset[10]), nullptr, nullptr, 0);
    int32_t mapNameRead11 = SFileReadFile(file, &mapNameOffset[11], sizeof(mapNameOffset[11]), nullptr, nullptr, 0);
    int32_t mapNameRead12 = SFileReadFile(file, &mapNameOffset[12], sizeof(mapNameOffset[12]), nullptr, nullptr, 0);
    int32_t mapNameRead13 = SFileReadFile(file, &mapNameOffset[13], sizeof(mapNameOffset[13]), nullptr, nullptr, 0);
    int32_t mapNameRead14 = SFileReadFile(file, &mapNameOffset[14], sizeof(mapNameOffset[14]), nullptr, nullptr, 0);
    int32_t mapNameRead15 = SFileReadFile(file, &mapNameOffset[15], sizeof(mapNameOffset[15]), nullptr, nullptr, 0);
    int32_t mapNameFlagsRead = SFileReadFile(file, &mapNameFlags, sizeof(mapNameFlags), nullptr, nullptr, 0);
    int32_t areaTableIDRead = SFileReadFile(file, &this->m_areaTableID, sizeof(this->m_areaTableID), nullptr, nullptr, 0);
    int32_t mapDescription0Read0 = SFileReadFile(file, &mapDescription0Offset[0], sizeof(mapDescription0Offset[0]), nullptr, nullptr, 0);
    int32_t mapDescription0Read1 = SFileReadFile(file, &mapDescription0Offset[1], sizeof(mapDescription0Offset[1]), nullptr, nullptr, 0);
    int32_t mapDescription0Read2 = SFileReadFile(file, &mapDescription0Offset[2], sizeof(mapDescription0Offset[2]), nullptr, nullptr, 0);
    int32_t mapDescription0Read3 = SFileReadFile(file, &mapDescription0Offset[3], sizeof(mapDescription0Offset[3]), nullptr, nullptr, 0);
    int32_t mapDescription0Read4 = SFileReadFile(file, &mapDescription0Offset[4], sizeof(mapDescription0Offset[4]), nullptr, nullptr, 0);
    int32_t mapDescription0Read5 = SFileReadFile(file, &mapDescription0Offset[5], sizeof(mapDescription0Offset[5]), nullptr, nullptr, 0);
    int32_t mapDescription0Read6 = SFileReadFile(file, &mapDescription0Offset[6], sizeof(mapDescription0Offset[6]), nullptr, nullptr, 0);
    int32_t mapDescription0Read7 = SFileReadFile(file, &mapDescription0Offset[7], sizeof(mapDescription0Offset[7]), nullptr, nullptr, 0);
    int32_t mapDescription0Read8 = SFileReadFile(file, &mapDescription0Offset[8], sizeof(mapDescription0Offset[8]), nullptr, nullptr, 0);
    int32_t mapDescription0Read9 = SFileReadFile(file, &mapDescription0Offset[9], sizeof(mapDescription0Offset[9]), nullptr, nullptr, 0);
    int32_t mapDescription0Read10 = SFileReadFile(file, &mapDescription0Offset[10], sizeof(mapDescription0Offset[10]), nullptr, nullptr, 0);
    int32_t mapDescription0Read11 = SFileReadFile(file, &mapDescription0Offset[11], sizeof(mapDescription0Offset[11]), nullptr, nullptr, 0);
    int32_t mapDescription0Read12 = SFileReadFile(file, &mapDescription0Offset[12], sizeof(mapDescription0Offset[12]), nullptr, nullptr, 0);
    int32_t mapDescription0Read13 = SFileReadFile(file, &mapDescription0Offset[13], sizeof(mapDescription0Offset[13]), nullptr, nullptr, 0);
    int32_t mapDescription0Read14 = SFileReadFile(file, &mapDescription0Offset[14], sizeof(mapDescription0Offset[14]), nullptr, nullptr, 0);
    int32_t mapDescription0Read15 = SFileReadFile(file, &mapDescription0Offset[15], sizeof(mapDescription0Offset[15]), nullptr, nullptr, 0);
    int32_t mapDescription0FlagsRead = SFileReadFile(file, &mapDescription0Flags, sizeof(mapDescription0Flags), nullptr, nullptr, 0);
    int32_t mapDescription1Read0 = SFileReadFile(file, &mapDescription1Offset[0], sizeof(mapDescription1Offset[0]), nullptr, nullptr, 0);
    int32_t mapDescription1Read1 = SFileReadFile(file, &mapDescription1Offset[1], sizeof(mapDescription1Offset[1]), nullptr, nullptr, 0);
    int32_t mapDescription1Read2 = SFileReadFile(file, &mapDescription1Offset[2], sizeof(mapDescription1Offset[2]), nullptr, nullptr, 0);
    int32_t mapDescription1Read3 = SFileReadFile(file, &mapDescription1Offset[3], sizeof(mapDescription1Offset[3]), nullptr, nullptr, 0);
    int32_t mapDescription1Read4 = SFileReadFile(file, &mapDescription1Offset[4], sizeof(mapDescription1Offset[4]), nullptr, nullptr, 0);
    int32_t mapDescription1Read5 = SFileReadFile(file, &mapDescription1Offset[5], sizeof(mapDescription1Offset[5]), nullptr, nullptr, 0);
    int32_t mapDescription1Read6 = SFileReadFile(file, &mapDescription1Offset[6], sizeof(mapDescription1Offset[6]), nullptr, nullptr, 0);
    int32_t mapDescription1Read7 = SFileReadFile(file, &mapDescription1Offset[7], sizeof(mapDescription1Offset[7]), nullptr, nullptr, 0);
    int32_t mapDescription1Read8 = SFileReadFile(file, &mapDescription1Offset[8], sizeof(mapDescription1Offset[8]), nullptr, nullptr, 0);
    int32_t mapDescription1Read9 = SFileReadFile(file, &mapDescription1Offset[9], sizeof(mapDescription1Offset[9]), nullptr, nullptr, 0);
    int32_t mapDescription1Read10 = SFileReadFile(file, &mapDescription1Offset[10], sizeof(mapDescription1Offset[10]), nullptr, nullptr, 0);
    int32_t mapDescription1Read11 = SFileReadFile(file, &mapDescription1Offset[11], sizeof(mapDescription1Offset[11]), nullptr, nullptr, 0);
    int32_t mapDescription1Read12 = SFileReadFile(file, &mapDescription1Offset[12], sizeof(mapDescription1Offset[12]), nullptr, nullptr, 0);
    int32_t mapDescription1Read13 = SFileReadFile(file, &mapDescription1Offset[13], sizeof(mapDescription1Offset[13]), nullptr, nullptr, 0);
    int32_t mapDescription1Read14 = SFileReadFile(file, &mapDescription1Offset[14], sizeof(mapDescription1Offset[14]), nullptr, nullptr, 0);
    int32_t mapDescription1Read15 = SFileReadFile(file, &mapDescription1Offset[15], sizeof(mapDescription1Offset[15]), nullptr, nullptr, 0);
    int32_t mapDescription1FlagsRead = SFileReadFile(file, &mapDescription1Flags, sizeof(mapDescription1Flags), nullptr, nullptr, 0);
    int32_t loadingScreenIDRead = SFileReadFile(file, &this->m_loadingScreenID, sizeof(this->m_loadingScreenID), nullptr, nullptr, 0);
    int32_t minimapIconScaleRead = SFileReadFile(file, &this->m_minimapIconScale, sizeof(this->m_minimapIconScale), nullptr, nullptr, 0);
    int32_t corpseMapIDRead = SFileReadFile(file, &this->m_corpseMapID, sizeof(this->m_corpseMapID), nullptr, nullptr, 0);
    int32_t corpseRead0 = SFileReadFile(file, &this->m_corpse[0], sizeof(this->m_corpse[0]), nullptr, nullptr, 0);
    int32_t corpseRead1 = SFileReadFile(file, &this->m_corpse[1], sizeof(this->m_corpse[1]), nullptr, nullptr, 0);
    int32_t timeOfDayOverrideRead = SFileReadFile(file, &this->m_timeOfDayOverride, sizeof(this->m_timeOfDayOverride), nullptr, nullptr, 0);
    int32_t expansionIDRead = SFileReadFile(file, &this->m_expansionID, sizeof(this->m_expansionID), nullptr, nullptr, 0);
    int32_t raidRead = SFileReadFile(file, &this->m_raidOffset, sizeof(this->m_raidOffset), nullptr, nullptr, 0);
    int32_t maxPlayersRead = SFileReadFile(file, &this->m_maxPlayers, sizeof(this->m_maxPlayers), nullptr, nullptr, 0);

    if (!idRead
        | !directoryRead
        | !instanceTypeRead
        | !flagsRead
        | !pvpRead
        | !mapNameRead0
        | !mapNameRead1
        | !mapNameRead2
        | !mapNameRead3
        | !mapNameRead4
        | !mapNameRead5
        | !mapNameRead6
        | !mapNameRead7
        | !mapNameRead8
        | !mapNameRead9
        | !mapNameRead10
        | !mapNameRead11
        | !mapNameRead12
        | !mapNameRead13
        | !mapNameRead14
        | !mapNameRead15
        | !mapNameFlagsRead
        | !areaTableIDRead
        | !mapDescription0Read0
        | !mapDescription0Read1
        | !mapDescription0Read2
        | !mapDescription0Read3
        | !mapDescription0Read4
        | !mapDescription0Read5
        | !mapDescription0Read6
        | !mapDescription0Read7
        | !mapDescription0Read8
        | !mapDescription0Read9
        | !mapDescription0Read10
        | !mapDescription0Read11
        | !mapDescription0Read12
        | !mapDescription0Read13
        | !mapDescription0Read14
        | !mapDescription0Read15
        | !mapDescription0FlagsRead
        | !mapDescription1Read0
        | !mapDescription1Read1
        | !mapDescription1Read2
        | !mapDescription1Read3
        | !mapDescription1Read4
        | !mapDescription1Read5
        | !mapDescription1Read6
        | !mapDescription1Read7
        | !mapDescription1Read8
        | !mapDescription1Read9
        | !mapDescription1Read10
        | !mapDescription1Read11
        | !mapDescription1Read12
        | !mapDescription1Read13
        | !mapDescription1Read14
        | !mapDescription1Read15
        | !mapDescription1FlagsRead
        | !loadingScreenIDRead
        | !minimapIconScaleRead
        | !corpseMapIDRead
        | !corpseRead0
        | !corpseRead1
        | !timeOfDayOverrideRead
        | !expansionIDRead
        | !raidRead
        | !maxPlayersRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_directory = &stringBuffer[directoryOffset];
        this->m_mapName = &stringBuffer[mapNameOffset[s_locale]];
        this->m_mapDescription0 = &stringBuffer[mapDescription0Offset[s_locale]];
        this->m_mapDescription1 = &stringBuffer[mapDescription1Offset[s_locale]];
    } else {
        this->m_directory = "";
        this->m_mapName = "";
        this->m_mapDescription0 = "";
        this->m_mapDescription1 = "";
    }

    return true;
}
