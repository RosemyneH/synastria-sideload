#include "WoW/Source/DB/Rec/AttackAnimTypesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

AttackAnimTypesRec::AttackAnimTypesRec() {
}

AttackAnimTypesRec::~AttackAnimTypesRec() {
}

// VERIFY: ca38451f auto
// OFFSET: 0x008AA100
const char* AttackAnimTypesRec::GetFilename() {
    return "DBFilesClient\\AttackAnimTypes.dbc";
}

// VERIFY: 1df96a89 auto
// OFFSET: 0x008B98D0
bool AttackAnimTypesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t animNameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t animNameRead = SFileReadFile(file, &animNameOffset, sizeof(animNameOffset), nullptr, nullptr, 0);

    if (!idRead
        | !animNameRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_animName = &stringBuffer[animNameOffset];
    } else {
        this->m_animName = "";
    }

    return true;
}
