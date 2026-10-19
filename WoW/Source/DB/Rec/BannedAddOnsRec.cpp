#include "WoW/Source/DB/Rec/BannedAddOnsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

BannedAddOnsRec::BannedAddOnsRec() {
}

BannedAddOnsRec::~BannedAddOnsRec() {
}

// VERIFY: 1dfaf6d7 auto
// OFFSET: 0x008AA130
const char* BannedAddOnsRec::GetFilename() {
    return "DBFilesClient\\BannedAddOns.dbc";
}

// VERIFY: 7176f2af manual
// OFFSET: 0x008AFED0
bool BannedAddOnsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameMd5Read = SFileReadFile(file, this->m_nameMd5, sizeof(this->m_nameMd5), nullptr, nullptr, 0);
    int32_t versionMd5Read = SFileReadFile(file, this->m_versionMd5, sizeof(this->m_versionMd5), nullptr, nullptr, 0);
    int32_t lastModifiedRead = SFileReadFile(file, &this->m_lastModified, sizeof(this->m_lastModified), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);

    if (!idRead
        | !nameMd5Read
        | !versionMd5Read
        | !lastModifiedRead
        | !flagsRead) {
        return false;
    }

    return true;
}
