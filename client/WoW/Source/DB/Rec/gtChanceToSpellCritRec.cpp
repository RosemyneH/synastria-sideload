#include "WoW/Source/DB/Rec/gtChanceToSpellCritRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

gtChanceToSpellCritRec::gtChanceToSpellCritRec() {
}

gtChanceToSpellCritRec::~gtChanceToSpellCritRec() {
}

// VERIFY: 54142bc0 auto
// OFFSET: 0x008AFFC0
const char* gtChanceToSpellCritRec::GetFilename() {
    return "DBFilesClient\\gtChanceToSpellCrit.dbc";
}

bool gtChanceToSpellCritRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!dataRead) {
        return false;
    }

    return true;
}
