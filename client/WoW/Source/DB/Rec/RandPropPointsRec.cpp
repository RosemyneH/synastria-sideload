#include "WoW/Source/DB/Rec/RandPropPointsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

RandPropPointsRec::RandPropPointsRec() {
}

RandPropPointsRec::~RandPropPointsRec() {
}

// VERIFY: 655a04e0 auto
// OFFSET: 0x008B5310
const char* RandPropPointsRec::GetFilename() {
    return "DBFilesClient\\RandPropPoints.dbc";
}

// VERIFY: 1ab2e275 manual
// OFFSET: 0x008B5320
bool RandPropPointsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t epicRead = SFileReadFile(file, this->m_epic, sizeof(this->m_epic), nullptr, nullptr, 0);
    int32_t superiorRead = SFileReadFile(file, this->m_superior, sizeof(this->m_superior), nullptr, nullptr, 0);
    int32_t goodRead = SFileReadFile(file, this->m_good, sizeof(this->m_good), nullptr, nullptr, 0);

    if (!idRead
        | !epicRead
        | !superiorRead
        | !goodRead) {
        return false;
    }

    return true;
}
