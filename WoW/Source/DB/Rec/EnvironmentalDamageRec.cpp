#include "WoW/Source/DB/Rec/EnvironmentalDamageRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

EnvironmentalDamageRec::EnvironmentalDamageRec() {
}

EnvironmentalDamageRec::~EnvironmentalDamageRec() {
}

// VERIFY: ef32af3c auto
// OFFSET: 0x008AEEC0
const char* EnvironmentalDamageRec::GetFilename() {
    return "DBFilesClient\\EnvironmentalDamage.dbc";
}

// VERIFY: d43168b5 manual
// OFFSET: 0x008B2AA0
bool EnvironmentalDamageRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t enumIDRead = SFileReadFile(file, &this->m_enumID, sizeof(this->m_enumID), nullptr, nullptr, 0);
    int32_t visualkitIDRead = SFileReadFile(file, &this->m_visualkitID, sizeof(this->m_visualkitID), nullptr, nullptr, 0);

    if (!idRead
        | !enumIDRead
        | !visualkitIDRead) {
        return false;
    }

    return true;
}
