#include "WoW/Source/DB/Rec/ScalingStatDistributionRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ScalingStatDistributionRec::ScalingStatDistributionRec() {
}

ScalingStatDistributionRec::~ScalingStatDistributionRec() {
}

// VERIFY: 25e758a4 auto
// OFFSET: 0x008B5680
const char* ScalingStatDistributionRec::GetFilename() {
    return "DBFilesClient\\ScalingStatDistribution.dbc";
}

// VERIFY: f0b8d343 manual
// OFFSET: 0x008B5690
bool ScalingStatDistributionRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t statIDRead = SFileReadFile(file, this->m_statID, sizeof(this->m_statID), nullptr, nullptr, 0);
    int32_t bonusRead = SFileReadFile(file, this->m_bonus, sizeof(this->m_bonus), nullptr, nullptr, 0);
    int32_t maxlevelRead = SFileReadFile(file, &this->m_maxlevel, sizeof(this->m_maxlevel), nullptr, nullptr, 0);

    if (!idRead
        | !statIDRead
        | !bonusRead
        | !maxlevelRead) {
        return false;
    }

    return true;
}
