#include "WoW/Source/DB/Rec/gtChanceToMeleeCritRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

gtChanceToMeleeCritRec::gtChanceToMeleeCritRec() {
}

gtChanceToMeleeCritRec::~gtChanceToMeleeCritRec() {
}

// VERIFY: 3b160a25 auto
// OFFSET: 0x008AFFA0
const char* gtChanceToMeleeCritRec::GetFilename() {
    return "DBFilesClient\\gtChanceToMeleeCrit.dbc";
}

bool gtChanceToMeleeCritRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!dataRead) {
        return false;
    }

    return true;
}
