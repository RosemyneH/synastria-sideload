#include "WoW/Source/DB/Rec/Cfg_ConfigsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

Cfg_ConfigsRec::Cfg_ConfigsRec() {
}

Cfg_ConfigsRec::~Cfg_ConfigsRec() {
}

// VERIFY: d11d1835 auto
// OFFSET: 0x008AAD90
const char* Cfg_ConfigsRec::GetFilename() {
    return "DBFilesClient\\Cfg_Configs.dbc";
}

// VERIFY: ddfb4f4a manual
// OFFSET: 0x008B72B0
bool Cfg_ConfigsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t realmTypeRead = SFileReadFile(file, &this->m_realmType, sizeof(this->m_realmType), nullptr, nullptr, 0);
    int32_t playerKillingAllowedRead = SFileReadFile(file, &this->m_playerKillingAllowed, sizeof(this->m_playerKillingAllowed), nullptr, nullptr, 0);
    int32_t roleplayingRead = SFileReadFile(file, &this->m_roleplaying, sizeof(this->m_roleplaying), nullptr, nullptr, 0);

    if (!idRead
        | !realmTypeRead
        | !playerKillingAllowedRead
        | !roleplayingRead) {
        return false;
    }

    return true;
}
