#include "WoW/Source/DB/Rec/SkillTiersRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SkillTiersRec::SkillTiersRec() {
}

SkillTiersRec::~SkillTiersRec() {
}

// VERIFY: b72ff1c5 auto
// OFFSET: 0x008B6740
const char* SkillTiersRec::GetFilename() {
    return "DBFilesClient\\SkillTiers.dbc";
}

// VERIFY: 4ec6b928 manual
// OFFSET: 0x008B6750
bool SkillTiersRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t costRead = SFileReadFile(file, this->m_cost, sizeof(this->m_cost), nullptr, nullptr, 0);
    int32_t valueRead = SFileReadFile(file, this->m_value, sizeof(this->m_value), nullptr, nullptr, 0);

    if (!idRead
        | !costRead
        | !valueRead) {
        return false;
    }

    return true;
}
