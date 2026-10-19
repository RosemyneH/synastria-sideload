#include "WoW/Source/DB/Rec/PageTextMaterialRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

PageTextMaterialRec::PageTextMaterialRec() {
}

PageTextMaterialRec::~PageTextMaterialRec() {
}

// VERIFY: 2dbeb3ae auto
// OFFSET: 0x008B4CA0
const char* PageTextMaterialRec::GetFilename() {
    return "DBFilesClient\\PageTextMaterial.dbc";
}

bool PageTextMaterialRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
    } else {
        this->m_name = "";
    }

    return true;
}
