#include "WoW/Source/DB/Rec/UnitBloodRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

UnitBloodRec::UnitBloodRec() {
}

UnitBloodRec::~UnitBloodRec() {
}

// VERIFY: e716cdfe auto
// OFFSET: 0x008BA7A0
const char* UnitBloodRec::GetFilename() {
    return "DBFilesClient\\UnitBlood.dbc";
}

// VERIFY: 7d8a1069 manual
// OFFSET: 0x008BA7B0
bool UnitBloodRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t groundBloodOffset[5];

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t combatBloodSpurtFrontRead = SFileReadFile(file, this->m_combatBloodSpurtFront, sizeof(this->m_combatBloodSpurtFront), nullptr, nullptr, 0);
    int32_t combatBloodSpurtBackRead = SFileReadFile(file, this->m_combatBloodSpurtBack, sizeof(this->m_combatBloodSpurtBack), nullptr, nullptr, 0);
    int32_t groundBloodRead0 = SFileReadFile(file, &groundBloodOffset[0], sizeof(groundBloodOffset[0]), nullptr, nullptr, 0);
    int32_t groundBloodRead1 = SFileReadFile(file, &groundBloodOffset[1], sizeof(groundBloodOffset[1]), nullptr, nullptr, 0);
    int32_t groundBloodRead2 = SFileReadFile(file, &groundBloodOffset[2], sizeof(groundBloodOffset[2]), nullptr, nullptr, 0);
    int32_t groundBloodRead3 = SFileReadFile(file, &groundBloodOffset[3], sizeof(groundBloodOffset[3]), nullptr, nullptr, 0);
    int32_t groundBloodRead4 = SFileReadFile(file, &groundBloodOffset[4], sizeof(groundBloodOffset[4]), nullptr, nullptr, 0);

    if (!idRead
        | !combatBloodSpurtFrontRead
        | !combatBloodSpurtBackRead
        | !groundBloodRead0
        | !groundBloodRead1
        | !groundBloodRead2
        | !groundBloodRead3
        | !groundBloodRead4) {
        return false;
    }

    if (stringBuffer) {
        this->m_groundBlood[0] = &stringBuffer[groundBloodOffset[0]];
        this->m_groundBlood[1] = &stringBuffer[groundBloodOffset[1]];
        this->m_groundBlood[2] = &stringBuffer[groundBloodOffset[2]];
        this->m_groundBlood[3] = &stringBuffer[groundBloodOffset[3]];
        this->m_groundBlood[4] = &stringBuffer[groundBloodOffset[4]];
    } else {
        this->m_groundBlood[0] = "";
        this->m_groundBlood[1] = "";
        this->m_groundBlood[2] = "";
        this->m_groundBlood[3] = "";
        this->m_groundBlood[4] = "";
    }

    return true;
}
