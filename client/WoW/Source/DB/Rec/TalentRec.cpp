#include "WoW/Source/DB/Rec/TalentRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

TalentRec::TalentRec() {
}

TalentRec::~TalentRec() {
}

// VERIFY: 778355fb auto
// OFFSET: 0x008B9960
const char* TalentRec::GetFilename() {
    return "DBFilesClient\\Talent.dbc";
}

// VERIFY: e8abb218 auto
// OFFSET: 0x008B9970
bool TalentRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t tabIDRead = SFileReadFile(file, &this->m_tabID, sizeof(this->m_tabID), nullptr, nullptr, 0);
    int32_t tierIDRead = SFileReadFile(file, &this->m_tierID, sizeof(this->m_tierID), nullptr, nullptr, 0);
    int32_t columnIndexRead = SFileReadFile(file, &this->m_columnIndex, sizeof(this->m_columnIndex), nullptr, nullptr, 0);
    int32_t spellRankRead = SFileReadFile(file, this->m_spellRank, sizeof(this->m_spellRank), nullptr, nullptr, 0);
    int32_t prereqTalentRead = SFileReadFile(file, this->m_prereqTalent, sizeof(this->m_prereqTalent), nullptr, nullptr, 0);
    int32_t prereqRankRead = SFileReadFile(file, this->m_prereqRank, sizeof(this->m_prereqRank), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t requiredSpellIDRead = SFileReadFile(file, &this->m_requiredSpellID, sizeof(this->m_requiredSpellID), nullptr, nullptr, 0);
    int32_t categoryMaskRead = SFileReadFile(file, this->m_categoryMask, sizeof(this->m_categoryMask), nullptr, nullptr, 0);

    if (!idRead
        | !tabIDRead
        | !tierIDRead
        | !columnIndexRead
        | !spellRankRead
        | !prereqTalentRead
        | !prereqRankRead
        | !flagsRead
        | !requiredSpellIDRead
        | !categoryMaskRead) {
        return false;
    }

    return true;
}
