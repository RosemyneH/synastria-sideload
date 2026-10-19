#include "WoW/Source/DB/Rec/SkillCostsDataRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SkillCostsDataRec::SkillCostsDataRec() {
}

SkillCostsDataRec::~SkillCostsDataRec() {
}

// VERIFY: 8003133c auto
// OFFSET: 0x008B5C70
const char* SkillCostsDataRec::GetFilename() {
    return "DBFilesClient\\SkillCostsData.dbc";
}

// VERIFY: 7e08b63b manual
// OFFSET: 0x008B5C80
bool SkillCostsDataRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t skillCostsIDRead = SFileReadFile(file, &this->m_skillCostsID, sizeof(this->m_skillCostsID), nullptr, nullptr, 0);
    int32_t costRead = SFileReadFile(file, this->m_cost, sizeof(this->m_cost), nullptr, nullptr, 0);

    if (!idRead
        | !skillCostsIDRead
        | !costRead) {
        return false;
    }

    return true;
}
