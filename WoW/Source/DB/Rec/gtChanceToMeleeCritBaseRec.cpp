#include "WoW/Source/DB/Rec/gtChanceToMeleeCritBaseRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

gtChanceToMeleeCritBaseRec::gtChanceToMeleeCritBaseRec() {
}

gtChanceToMeleeCritBaseRec::~gtChanceToMeleeCritBaseRec() {
}

// VERIFY: 6fe36c0e auto
// OFFSET: 0x008AFFB0
const char* gtChanceToMeleeCritBaseRec::GetFilename() {
    return "DBFilesClient\\gtChanceToMeleeCritBase.dbc";
}

bool gtChanceToMeleeCritBaseRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!dataRead) {
        return false;
    }

    return true;
}
