#include "WoW/Source/DB/Rec/CreatureMovementInfoRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CreatureMovementInfoRec::CreatureMovementInfoRec() {
}

CreatureMovementInfoRec::~CreatureMovementInfoRec() {
}

// VERIFY: 841623ee auto
// OFFSET: 0x008ADBA0
const char* CreatureMovementInfoRec::GetFilename() {
    return "DBFilesClient\\CreatureMovementInfo.dbc";
}

bool CreatureMovementInfoRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t smoothFacingChaseRateRead = SFileReadFile(file, &this->m_smoothFacingChaseRate, sizeof(this->m_smoothFacingChaseRate), nullptr, nullptr, 0);

    if (!idRead
        | !smoothFacingChaseRateRead) {
        return false;
    }

    return true;
}
