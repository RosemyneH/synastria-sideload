#include "WoW/Source/DB/Rec/LfgDungeonExpansionRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

LfgDungeonExpansionRec::LfgDungeonExpansionRec() {
}

LfgDungeonExpansionRec::~LfgDungeonExpansionRec() {
}

// VERIFY: 979ed05f auto
// OFFSET: 0x008B1CE0
const char* LfgDungeonExpansionRec::GetFilename() {
    return "DBFilesClient\\LfgDungeonExpansion.dbc";
}

bool LfgDungeonExpansionRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t lfgIDRead = SFileReadFile(file, &this->m_lfgID, sizeof(this->m_lfgID), nullptr, nullptr, 0);
    int32_t expansionLevelRead = SFileReadFile(file, &this->m_expansionLevel, sizeof(this->m_expansionLevel), nullptr, nullptr, 0);
    int32_t randomIDRead = SFileReadFile(file, &this->m_randomID, sizeof(this->m_randomID), nullptr, nullptr, 0);
    int32_t hardLevelMinRead = SFileReadFile(file, &this->m_hardLevelMin, sizeof(this->m_hardLevelMin), nullptr, nullptr, 0);
    int32_t hardLevelMaxRead = SFileReadFile(file, &this->m_hardLevelMax, sizeof(this->m_hardLevelMax), nullptr, nullptr, 0);
    int32_t targetLevelMinRead = SFileReadFile(file, &this->m_targetLevelMin, sizeof(this->m_targetLevelMin), nullptr, nullptr, 0);
    int32_t targetLevelMaxRead = SFileReadFile(file, &this->m_targetLevelMax, sizeof(this->m_targetLevelMax), nullptr, nullptr, 0);

    if (!idRead
        | !lfgIDRead
        | !expansionLevelRead
        | !randomIDRead
        | !hardLevelMinRead
        | !hardLevelMaxRead
        | !targetLevelMinRead
        | !targetLevelMaxRead) {
        return false;
    }

    return true;
}
