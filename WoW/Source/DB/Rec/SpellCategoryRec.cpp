#include "WoW/Source/DB/Rec/SpellCategoryRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellCategoryRec::SpellCategoryRec() {
}

SpellCategoryRec::~SpellCategoryRec() {
}

// VERIFY: 5bcf5223 auto
// OFFSET: 0x008B7340
const char* SpellCategoryRec::GetFilename() {
    return "DBFilesClient\\SpellCategory.dbc";
}

bool SpellCategoryRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);

    if (!idRead
        | !flagsRead) {
        return false;
    }

    return true;
}
