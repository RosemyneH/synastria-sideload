#include "WoW/Source/DB/Rec/ObjectEffectGroupRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ObjectEffectGroupRec::ObjectEffectGroupRec() {
}

ObjectEffectGroupRec::~ObjectEffectGroupRec() {
}

// VERIFY: 2bce8cb4 auto
// OFFSET: 0x008BD6F0
const char* ObjectEffectGroupRec::GetFilename() {
    return "DBFilesClient\\ObjectEffectGroup.dbc";
}

bool ObjectEffectGroupRec::Read(SFileImpl* file, const char* stringBuffer) {
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
