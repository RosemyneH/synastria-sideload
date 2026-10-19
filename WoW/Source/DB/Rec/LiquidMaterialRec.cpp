#include "WoW/Source/DB/Rec/LiquidMaterialRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

LiquidMaterialRec::LiquidMaterialRec() {
}

LiquidMaterialRec::~LiquidMaterialRec() {
}

// VERIFY: c7c6fa39 auto
// OFFSET: 0x008B2A90
const char* LiquidMaterialRec::GetFilename() {
    return "DBFilesClient\\LiquidMaterial.dbc";
}

bool LiquidMaterialRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t lvfRead = SFileReadFile(file, &this->m_lvf, sizeof(this->m_lvf), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);

    if (!idRead
        | !lvfRead
        | !flagsRead) {
        return false;
    }

    return true;
}
