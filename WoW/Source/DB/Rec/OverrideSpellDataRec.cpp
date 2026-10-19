#include "WoW/Source/DB/Rec/OverrideSpellDataRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

OverrideSpellDataRec::OverrideSpellDataRec() {
}

OverrideSpellDataRec::~OverrideSpellDataRec() {
}

// VERIFY: af8274c4 auto
// OFFSET: 0x008B4940
const char* OverrideSpellDataRec::GetFilename() {
    return "DBFilesClient\\OverrideSpellData.dbc";
}

// VERIFY: bcc600b2 manual
// OFFSET: 0x008B4950
bool OverrideSpellDataRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t spellsRead = SFileReadFile(file, this->m_spells, sizeof(this->m_spells), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);

    if (!idRead
        | !spellsRead
        | !flagsRead) {
        return false;
    }

    return true;
}
