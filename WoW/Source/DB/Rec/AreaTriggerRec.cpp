#include "WoW/Source/DB/Rec/AreaTriggerRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

AreaTriggerRec::AreaTriggerRec() {
}

AreaTriggerRec::~AreaTriggerRec() {
}

// VERIFY: 5c27e72a auto
// OFFSET: 0x008AA0E0
const char* AreaTriggerRec::GetFilename() {
    return "DBFilesClient\\AreaTrigger.dbc";
}

// VERIFY: b80c185a auto
// OFFSET: 0x008BBDA0
bool AreaTriggerRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t continentIDRead = SFileReadFile(file, &this->m_continentID, sizeof(this->m_continentID), nullptr, nullptr, 0);
    int32_t posRead0 = SFileReadFile(file, &this->m_pos[0], sizeof(this->m_pos[0]), nullptr, nullptr, 0);
    int32_t posRead1 = SFileReadFile(file, &this->m_pos[1], sizeof(this->m_pos[1]), nullptr, nullptr, 0);
    int32_t posRead2 = SFileReadFile(file, &this->m_pos[2], sizeof(this->m_pos[2]), nullptr, nullptr, 0);
    int32_t radiusRead = SFileReadFile(file, &this->m_radius, sizeof(this->m_radius), nullptr, nullptr, 0);
    int32_t boxLengthRead = SFileReadFile(file, &this->m_boxLength, sizeof(this->m_boxLength), nullptr, nullptr, 0);
    int32_t boxWidthRead = SFileReadFile(file, &this->m_boxWidth, sizeof(this->m_boxWidth), nullptr, nullptr, 0);
    int32_t boxHeightRead = SFileReadFile(file, &this->m_boxHeight, sizeof(this->m_boxHeight), nullptr, nullptr, 0);
    int32_t boxYawRead = SFileReadFile(file, &this->m_boxYaw, sizeof(this->m_boxYaw), nullptr, nullptr, 0);

    if (!idRead
        | !continentIDRead
        | !posRead0
        | !posRead1
        | !posRead2
        | !radiusRead
        | !boxLengthRead
        | !boxWidthRead
        | !boxHeightRead
        | !boxYawRead) {
        return false;
    }

    return true;
}
