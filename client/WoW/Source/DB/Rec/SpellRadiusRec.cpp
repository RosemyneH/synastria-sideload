#include "WoW/Source/DB/Rec/SpellRadiusRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellRadiusRec::SpellRadiusRec() {
}

SpellRadiusRec::~SpellRadiusRec() {
}

// VERIFY: b498825f auto
// OFFSET: 0x008B85F0
const char* SpellRadiusRec::GetFilename() {
    return "DBFilesClient\\SpellRadius.dbc";
}

bool SpellRadiusRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t radiusRead = SFileReadFile(file, &this->m_radius, sizeof(this->m_radius), nullptr, nullptr, 0);
    int32_t radiusPerLevelRead = SFileReadFile(file, &this->m_radiusPerLevel, sizeof(this->m_radiusPerLevel), nullptr, nullptr, 0);
    int32_t radiusMaxRead = SFileReadFile(file, &this->m_radiusMax, sizeof(this->m_radiusMax), nullptr, nullptr, 0);

    if (!idRead
        | !radiusRead
        | !radiusPerLevelRead
        | !radiusMaxRead) {
        return false;
    }

    return true;
}
