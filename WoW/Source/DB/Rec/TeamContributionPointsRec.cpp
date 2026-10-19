#include "WoW/Source/DB/Rec/TeamContributionPointsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

TeamContributionPointsRec::TeamContributionPointsRec() {
}

TeamContributionPointsRec::~TeamContributionPointsRec() {
}

// VERIFY: 93076cbe auto
// OFFSET: 0x008BA190
const char* TeamContributionPointsRec::GetFilename() {
    return "DBFilesClient\\TeamContributionPoints.dbc";
}

bool TeamContributionPointsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!idRead
        | !dataRead) {
        return false;
    }

    return true;
}
