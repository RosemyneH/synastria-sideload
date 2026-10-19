#include "WoW/Source/DB/Rec/FactionTemplateRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

FactionTemplateRec::FactionTemplateRec() {
}

FactionTemplateRec::~FactionTemplateRec() {
}

// VERIFY: 82e76986 auto
// OFFSET: 0x008AF800
const char* FactionTemplateRec::GetFilename() {
    return "DBFilesClient\\FactionTemplate.dbc";
}

// VERIFY: 3723d25f manual
// OFFSET: 0x008AF810
bool FactionTemplateRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t factionRead = SFileReadFile(file, &this->m_faction, sizeof(this->m_faction), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t factionGroupRead = SFileReadFile(file, &this->m_factionGroup, sizeof(this->m_factionGroup), nullptr, nullptr, 0);
    int32_t friendGroupRead = SFileReadFile(file, &this->m_friendGroup, sizeof(this->m_friendGroup), nullptr, nullptr, 0);
    int32_t enemyGroupRead = SFileReadFile(file, &this->m_enemyGroup, sizeof(this->m_enemyGroup), nullptr, nullptr, 0);
    int32_t enemiesRead = SFileReadFile(file, this->m_enemies, sizeof(this->m_enemies), nullptr, nullptr, 0);
    int32_t friendRead = SFileReadFile(file, this->m_friend, sizeof(this->m_friend), nullptr, nullptr, 0);

    if (!idRead
        | !factionRead
        | !flagsRead
        | !factionGroupRead
        | !friendGroupRead
        | !enemyGroupRead
        | !enemiesRead
        | !friendRead) {
        return false;
    }

    return true;
}
