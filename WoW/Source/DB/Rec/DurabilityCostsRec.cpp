#include "WoW/Source/DB/Rec/DurabilityCostsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

DurabilityCostsRec::DurabilityCostsRec() {
}

DurabilityCostsRec::~DurabilityCostsRec() {
}

// VERIFY: 4a0f999a auto
// OFFSET: 0x008AED30
const char* DurabilityCostsRec::GetFilename() {
    return "DBFilesClient\\DurabilityCosts.dbc";
}

// VERIFY: 35693317 manual
// OFFSET: 0x008AED40
bool DurabilityCostsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t weaponSubClassCostRead = SFileReadFile(file, this->m_weaponSubClassCost, sizeof(this->m_weaponSubClassCost), nullptr, nullptr, 0);
    int32_t armorSubClassCostRead = SFileReadFile(file, this->m_armorSubClassCost, sizeof(this->m_armorSubClassCost), nullptr, nullptr, 0);

    if (!idRead
        | !weaponSubClassCostRead
        | !armorSubClassCostRead) {
        return false;
    }

    return true;
}
