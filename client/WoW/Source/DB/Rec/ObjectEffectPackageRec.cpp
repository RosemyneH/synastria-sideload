#include "WoW/Source/DB/Rec/ObjectEffectPackageRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ObjectEffectPackageRec::ObjectEffectPackageRec() {
}

ObjectEffectPackageRec::~ObjectEffectPackageRec() {
}

// VERIFY: d1dd9b1d auto
// OFFSET: 0x008BD7C0
const char* ObjectEffectPackageRec::GetFilename() {
    return "DBFilesClient\\ObjectEffectPackage.dbc";
}

bool ObjectEffectPackageRec::Read(SFileImpl* file, const char* stringBuffer) {
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
