#include "WoW/Source/DB/Rec/BattlemasterListRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

BattlemasterListRec::BattlemasterListRec() {
}

BattlemasterListRec::~BattlemasterListRec() {
}

// VERIFY: 63a2fa28 auto
// OFFSET: 0x008AA710
const char* BattlemasterListRec::GetFilename() {
    return "DBFilesClient\\BattlemasterList.dbc";
}

// VERIFY: fd343852 auto
// OFFSET: 0x008AA720
bool BattlemasterListRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset[16];
    uint32_t nameFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t mapIDRead = SFileReadFile(file, this->m_mapID, sizeof(this->m_mapID), nullptr, nullptr, 0);
    int32_t instanceTypeRead = SFileReadFile(file, &this->m_instanceType, sizeof(this->m_instanceType), nullptr, nullptr, 0);
    int32_t groupsAllowedRead = SFileReadFile(file, &this->m_groupsAllowed, sizeof(this->m_groupsAllowed), nullptr, nullptr, 0);
    int32_t nameRead0 = SFileReadFile(file, &nameOffset[0], sizeof(nameOffset[0]), nullptr, nullptr, 0);
    int32_t nameRead1 = SFileReadFile(file, &nameOffset[1], sizeof(nameOffset[1]), nullptr, nullptr, 0);
    int32_t nameRead2 = SFileReadFile(file, &nameOffset[2], sizeof(nameOffset[2]), nullptr, nullptr, 0);
    int32_t nameRead3 = SFileReadFile(file, &nameOffset[3], sizeof(nameOffset[3]), nullptr, nullptr, 0);
    int32_t nameRead4 = SFileReadFile(file, &nameOffset[4], sizeof(nameOffset[4]), nullptr, nullptr, 0);
    int32_t nameRead5 = SFileReadFile(file, &nameOffset[5], sizeof(nameOffset[5]), nullptr, nullptr, 0);
    int32_t nameRead6 = SFileReadFile(file, &nameOffset[6], sizeof(nameOffset[6]), nullptr, nullptr, 0);
    int32_t nameRead7 = SFileReadFile(file, &nameOffset[7], sizeof(nameOffset[7]), nullptr, nullptr, 0);
    int32_t nameRead8 = SFileReadFile(file, &nameOffset[8], sizeof(nameOffset[8]), nullptr, nullptr, 0);
    int32_t nameRead9 = SFileReadFile(file, &nameOffset[9], sizeof(nameOffset[9]), nullptr, nullptr, 0);
    int32_t nameRead10 = SFileReadFile(file, &nameOffset[10], sizeof(nameOffset[10]), nullptr, nullptr, 0);
    int32_t nameRead11 = SFileReadFile(file, &nameOffset[11], sizeof(nameOffset[11]), nullptr, nullptr, 0);
    int32_t nameRead12 = SFileReadFile(file, &nameOffset[12], sizeof(nameOffset[12]), nullptr, nullptr, 0);
    int32_t nameRead13 = SFileReadFile(file, &nameOffset[13], sizeof(nameOffset[13]), nullptr, nullptr, 0);
    int32_t nameRead14 = SFileReadFile(file, &nameOffset[14], sizeof(nameOffset[14]), nullptr, nullptr, 0);
    int32_t nameRead15 = SFileReadFile(file, &nameOffset[15], sizeof(nameOffset[15]), nullptr, nullptr, 0);
    int32_t nameFlagsRead = SFileReadFile(file, &nameFlags, sizeof(nameFlags), nullptr, nullptr, 0);
    int32_t maxGroupSizeRead = SFileReadFile(file, &this->m_maxGroupSize, sizeof(this->m_maxGroupSize), nullptr, nullptr, 0);
    int32_t holidayWorldStateRead = SFileReadFile(file, &this->m_holidayWorldState, sizeof(this->m_holidayWorldState), nullptr, nullptr, 0);
    int32_t minLevelRead = SFileReadFile(file, &this->m_minLevel, sizeof(this->m_minLevel), nullptr, nullptr, 0);
    int32_t maxLevelRead = SFileReadFile(file, &this->m_maxLevel, sizeof(this->m_maxLevel), nullptr, nullptr, 0);

    if (!idRead
        | !mapIDRead
        | !instanceTypeRead
        | !groupsAllowedRead
        | !nameRead0
        | !nameRead1
        | !nameRead2
        | !nameRead3
        | !nameRead4
        | !nameRead5
        | !nameRead6
        | !nameRead7
        | !nameRead8
        | !nameRead9
        | !nameRead10
        | !nameRead11
        | !nameRead12
        | !nameRead13
        | !nameRead14
        | !nameRead15
        | !nameFlagsRead
        | !maxGroupSizeRead
        | !holidayWorldStateRead
        | !minLevelRead
        | !maxLevelRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset[s_locale]];
    } else {
        this->m_name = "";
    }

    return true;
}
