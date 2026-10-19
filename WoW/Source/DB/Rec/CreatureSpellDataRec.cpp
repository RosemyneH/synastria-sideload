#include "WoW/Source/DB/Rec/CreatureSpellDataRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CreatureSpellDataRec::CreatureSpellDataRec() {
}

CreatureSpellDataRec::~CreatureSpellDataRec() {
}

// VERIFY: 6cb1e51c auto
// OFFSET: 0x008ADFB0
const char* CreatureSpellDataRec::GetFilename() {
    return "DBFilesClient\\CreatureSpellData.dbc";
}

// VERIFY: e5f04188 manual
// OFFSET: 0x008ADFC0
bool CreatureSpellDataRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t spellsRead = SFileReadFile(file, this->m_spells, sizeof(this->m_spells), nullptr, nullptr, 0);
    int32_t availabilityRead = SFileReadFile(file, this->m_availability, sizeof(this->m_availability), nullptr, nullptr, 0);

    if (!idRead
        | !spellsRead
        | !availabilityRead) {
        return false;
    }

    return true;
}
