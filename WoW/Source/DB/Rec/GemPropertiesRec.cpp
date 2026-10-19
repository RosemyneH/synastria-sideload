#include "WoW/Source/DB/Rec/GemPropertiesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

GemPropertiesRec::GemPropertiesRec() {
}

GemPropertiesRec::~GemPropertiesRec() {
}

// VERIFY: 51f3d8a1 auto
// OFFSET: 0x008AFDE0
const char* GemPropertiesRec::GetFilename() {
    return "DBFilesClient\\GemProperties.dbc";
}

bool GemPropertiesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t enchantIDRead = SFileReadFile(file, &this->m_enchantID, sizeof(this->m_enchantID), nullptr, nullptr, 0);
    int32_t maxcountInvRead = SFileReadFile(file, &this->m_maxcountInv, sizeof(this->m_maxcountInv), nullptr, nullptr, 0);
    int32_t maxcountItemRead = SFileReadFile(file, &this->m_maxcountItem, sizeof(this->m_maxcountItem), nullptr, nullptr, 0);
    int32_t typeRead = SFileReadFile(file, &this->m_type, sizeof(this->m_type), nullptr, nullptr, 0);

    if (!idRead
        | !enchantIDRead
        | !maxcountInvRead
        | !maxcountItemRead
        | !typeRead) {
        return false;
    }

    return true;
}
