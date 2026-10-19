#include "WoW/Source/DB/Rec/AreaGroupRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

AreaGroupRec::AreaGroupRec() {
}

AreaGroupRec::~AreaGroupRec() {
}

// VERIFY: 3a4fb022 auto
// OFFSET: 0x008A9520
const char* AreaGroupRec::GetFilename() {
    return "DBFilesClient\\AreaGroup.dbc";
}

// VERIFY: 9a2fa7da manual
// OFFSET: 0x008A9530
bool AreaGroupRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t areaIDRead = SFileReadFile(file, this->m_areaID, sizeof(this->m_areaID), nullptr, nullptr, 0);
    int32_t nextAreaIDRead = SFileReadFile(file, &this->m_nextAreaID, sizeof(this->m_nextAreaID), nullptr, nullptr, 0);

    if (!idRead
        | !areaIDRead
        | !nextAreaIDRead) {
        return false;
    }

    return true;
}
