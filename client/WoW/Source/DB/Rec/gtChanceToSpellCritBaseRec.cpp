#include "WoW/Source/DB/Rec/gtChanceToSpellCritBaseRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

gtChanceToSpellCritBaseRec::gtChanceToSpellCritBaseRec() {
}

gtChanceToSpellCritBaseRec::~gtChanceToSpellCritBaseRec() {
}

// VERIFY: a9512ef9 auto
// OFFSET: 0x008AFFF0
const char* gtChanceToSpellCritBaseRec::GetFilename() {
    return "DBFilesClient\\gtChanceToSpellCritBase.dbc";
}

bool gtChanceToSpellCritBaseRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!dataRead) {
        return false;
    }

    return true;
}
