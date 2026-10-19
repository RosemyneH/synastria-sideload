#include "WoW/Source/DB/Rec/ObjectEffectPackageElemRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ObjectEffectPackageElemRec::ObjectEffectPackageElemRec() {
}

ObjectEffectPackageElemRec::~ObjectEffectPackageElemRec() {
}

// VERIFY: 6739603e auto
// OFFSET: 0x008BD7D0
const char* ObjectEffectPackageElemRec::GetFilename() {
    return "DBFilesClient\\ObjectEffectPackageElem.dbc";
}

bool ObjectEffectPackageElemRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t objectEffectPackageIDRead = SFileReadFile(file, &this->m_objectEffectPackageID, sizeof(this->m_objectEffectPackageID), nullptr, nullptr, 0);
    int32_t objectEffectGroupIDRead = SFileReadFile(file, &this->m_objectEffectGroupID, sizeof(this->m_objectEffectGroupID), nullptr, nullptr, 0);
    int32_t stateTypeRead = SFileReadFile(file, &this->m_stateType, sizeof(this->m_stateType), nullptr, nullptr, 0);

    if (!idRead
        | !objectEffectPackageIDRead
        | !objectEffectGroupIDRead
        | !stateTypeRead) {
        return false;
    }

    return true;
}
