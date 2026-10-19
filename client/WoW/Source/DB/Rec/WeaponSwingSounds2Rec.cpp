#include "WoW/Source/DB/Rec/WeaponSwingSounds2Rec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

WeaponSwingSounds2Rec::WeaponSwingSounds2Rec() {
}

WeaponSwingSounds2Rec::~WeaponSwingSounds2Rec() {
}

// VERIFY: 2c179fc0 auto
// OFFSET: 0x008BB6E0
const char* WeaponSwingSounds2Rec::GetFilename() {
    return "DBFilesClient\\WeaponSwingSounds2.dbc";
}

bool WeaponSwingSounds2Rec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t swingTypeRead = SFileReadFile(file, &this->m_swingType, sizeof(this->m_swingType), nullptr, nullptr, 0);
    int32_t critRead = SFileReadFile(file, &this->m_crit, sizeof(this->m_crit), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, &this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);

    if (!idRead
        | !swingTypeRead
        | !critRead
        | !soundIDRead) {
        return false;
    }

    return true;
}
