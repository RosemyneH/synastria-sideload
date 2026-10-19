#include "WoW/Source/DB/Rec/SpellRuneCostRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellRuneCostRec::SpellRuneCostRec() {
}

SpellRuneCostRec::~SpellRuneCostRec() {
}

// VERIFY: 7949af86 auto
// OFFSET: 0x008B8B90
const char* SpellRuneCostRec::GetFilename() {
    return "DBFilesClient\\SpellRuneCost.dbc";
}

bool SpellRuneCostRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t bloodRead = SFileReadFile(file, &this->m_blood, sizeof(this->m_blood), nullptr, nullptr, 0);
    int32_t unholyRead = SFileReadFile(file, &this->m_unholy, sizeof(this->m_unholy), nullptr, nullptr, 0);
    int32_t frostRead = SFileReadFile(file, &this->m_frost, sizeof(this->m_frost), nullptr, nullptr, 0);
    int32_t runicPowerRead = SFileReadFile(file, &this->m_runicPower, sizeof(this->m_runicPower), nullptr, nullptr, 0);

    if (!idRead
        | !bloodRead
        | !unholyRead
        | !frostRead
        | !runicPowerRead) {
        return false;
    }

    return true;
}
