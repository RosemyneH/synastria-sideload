#include "WoW/Source/DB/Rec/LightSkyboxRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

LightSkyboxRec::LightSkyboxRec() {
}

LightSkyboxRec::~LightSkyboxRec() {
}

// VERIFY: ab286ab1 auto
// OFFSET: 0x008BDFC0
const char* LightSkyboxRec::GetFilename() {
    return "DBFilesClient\\LightSkybox.dbc";
}

bool LightSkyboxRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead
        | !flagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
    } else {
        this->m_name = "";
    }

    return true;
}
