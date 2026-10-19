#include "WoW/Source/DB/Rec/AchievementRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

AchievementRec::AchievementRec() {
}

AchievementRec::~AchievementRec() {
}

// VERIFY: e4636bf3 auto
// OFFSET: 0x008A8360
const char* AchievementRec::GetFilename() {
    return "DBFilesClient\\Achievement.dbc";
}

// VERIFY: 3052a4c9 auto
// OFFSET: 0x008A8370
bool AchievementRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t titleOffset[16];
    uint32_t titleFlags;
    uint32_t descriptionOffset[16];
    uint32_t descriptionFlags;
    uint32_t rewardOffset[16];
    uint32_t rewardFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t factionRead = SFileReadFile(file, &this->m_faction, sizeof(this->m_faction), nullptr, nullptr, 0);
    int32_t instanceIDRead = SFileReadFile(file, &this->m_instanceID, sizeof(this->m_instanceID), nullptr, nullptr, 0);
    int32_t supercedesRead = SFileReadFile(file, &this->m_supercedes, sizeof(this->m_supercedes), nullptr, nullptr, 0);
    int32_t titleRead0 = SFileReadFile(file, &titleOffset[0], sizeof(titleOffset[0]), nullptr, nullptr, 0);
    int32_t titleRead1 = SFileReadFile(file, &titleOffset[1], sizeof(titleOffset[1]), nullptr, nullptr, 0);
    int32_t titleRead2 = SFileReadFile(file, &titleOffset[2], sizeof(titleOffset[2]), nullptr, nullptr, 0);
    int32_t titleRead3 = SFileReadFile(file, &titleOffset[3], sizeof(titleOffset[3]), nullptr, nullptr, 0);
    int32_t titleRead4 = SFileReadFile(file, &titleOffset[4], sizeof(titleOffset[4]), nullptr, nullptr, 0);
    int32_t titleRead5 = SFileReadFile(file, &titleOffset[5], sizeof(titleOffset[5]), nullptr, nullptr, 0);
    int32_t titleRead6 = SFileReadFile(file, &titleOffset[6], sizeof(titleOffset[6]), nullptr, nullptr, 0);
    int32_t titleRead7 = SFileReadFile(file, &titleOffset[7], sizeof(titleOffset[7]), nullptr, nullptr, 0);
    int32_t titleRead8 = SFileReadFile(file, &titleOffset[8], sizeof(titleOffset[8]), nullptr, nullptr, 0);
    int32_t titleRead9 = SFileReadFile(file, &titleOffset[9], sizeof(titleOffset[9]), nullptr, nullptr, 0);
    int32_t titleRead10 = SFileReadFile(file, &titleOffset[10], sizeof(titleOffset[10]), nullptr, nullptr, 0);
    int32_t titleRead11 = SFileReadFile(file, &titleOffset[11], sizeof(titleOffset[11]), nullptr, nullptr, 0);
    int32_t titleRead12 = SFileReadFile(file, &titleOffset[12], sizeof(titleOffset[12]), nullptr, nullptr, 0);
    int32_t titleRead13 = SFileReadFile(file, &titleOffset[13], sizeof(titleOffset[13]), nullptr, nullptr, 0);
    int32_t titleRead14 = SFileReadFile(file, &titleOffset[14], sizeof(titleOffset[14]), nullptr, nullptr, 0);
    int32_t titleRead15 = SFileReadFile(file, &titleOffset[15], sizeof(titleOffset[15]), nullptr, nullptr, 0);
    int32_t titleFlagsRead = SFileReadFile(file, &titleFlags, sizeof(titleFlags), nullptr, nullptr, 0);
    int32_t descriptionRead0 = SFileReadFile(file, &descriptionOffset[0], sizeof(descriptionOffset[0]), nullptr, nullptr, 0);
    int32_t descriptionRead1 = SFileReadFile(file, &descriptionOffset[1], sizeof(descriptionOffset[1]), nullptr, nullptr, 0);
    int32_t descriptionRead2 = SFileReadFile(file, &descriptionOffset[2], sizeof(descriptionOffset[2]), nullptr, nullptr, 0);
    int32_t descriptionRead3 = SFileReadFile(file, &descriptionOffset[3], sizeof(descriptionOffset[3]), nullptr, nullptr, 0);
    int32_t descriptionRead4 = SFileReadFile(file, &descriptionOffset[4], sizeof(descriptionOffset[4]), nullptr, nullptr, 0);
    int32_t descriptionRead5 = SFileReadFile(file, &descriptionOffset[5], sizeof(descriptionOffset[5]), nullptr, nullptr, 0);
    int32_t descriptionRead6 = SFileReadFile(file, &descriptionOffset[6], sizeof(descriptionOffset[6]), nullptr, nullptr, 0);
    int32_t descriptionRead7 = SFileReadFile(file, &descriptionOffset[7], sizeof(descriptionOffset[7]), nullptr, nullptr, 0);
    int32_t descriptionRead8 = SFileReadFile(file, &descriptionOffset[8], sizeof(descriptionOffset[8]), nullptr, nullptr, 0);
    int32_t descriptionRead9 = SFileReadFile(file, &descriptionOffset[9], sizeof(descriptionOffset[9]), nullptr, nullptr, 0);
    int32_t descriptionRead10 = SFileReadFile(file, &descriptionOffset[10], sizeof(descriptionOffset[10]), nullptr, nullptr, 0);
    int32_t descriptionRead11 = SFileReadFile(file, &descriptionOffset[11], sizeof(descriptionOffset[11]), nullptr, nullptr, 0);
    int32_t descriptionRead12 = SFileReadFile(file, &descriptionOffset[12], sizeof(descriptionOffset[12]), nullptr, nullptr, 0);
    int32_t descriptionRead13 = SFileReadFile(file, &descriptionOffset[13], sizeof(descriptionOffset[13]), nullptr, nullptr, 0);
    int32_t descriptionRead14 = SFileReadFile(file, &descriptionOffset[14], sizeof(descriptionOffset[14]), nullptr, nullptr, 0);
    int32_t descriptionRead15 = SFileReadFile(file, &descriptionOffset[15], sizeof(descriptionOffset[15]), nullptr, nullptr, 0);
    int32_t descriptionFlagsRead = SFileReadFile(file, &descriptionFlags, sizeof(descriptionFlags), nullptr, nullptr, 0);
    int32_t categoryRead = SFileReadFile(file, &this->m_category, sizeof(this->m_category), nullptr, nullptr, 0);
    int32_t pointsRead = SFileReadFile(file, &this->m_points, sizeof(this->m_points), nullptr, nullptr, 0);
    int32_t uiOrderRead = SFileReadFile(file, &this->m_uiOrder, sizeof(this->m_uiOrder), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t iconIDRead = SFileReadFile(file, &this->m_iconID, sizeof(this->m_iconID), nullptr, nullptr, 0);
    int32_t rewardRead0 = SFileReadFile(file, &rewardOffset[0], sizeof(rewardOffset[0]), nullptr, nullptr, 0);
    int32_t rewardRead1 = SFileReadFile(file, &rewardOffset[1], sizeof(rewardOffset[1]), nullptr, nullptr, 0);
    int32_t rewardRead2 = SFileReadFile(file, &rewardOffset[2], sizeof(rewardOffset[2]), nullptr, nullptr, 0);
    int32_t rewardRead3 = SFileReadFile(file, &rewardOffset[3], sizeof(rewardOffset[3]), nullptr, nullptr, 0);
    int32_t rewardRead4 = SFileReadFile(file, &rewardOffset[4], sizeof(rewardOffset[4]), nullptr, nullptr, 0);
    int32_t rewardRead5 = SFileReadFile(file, &rewardOffset[5], sizeof(rewardOffset[5]), nullptr, nullptr, 0);
    int32_t rewardRead6 = SFileReadFile(file, &rewardOffset[6], sizeof(rewardOffset[6]), nullptr, nullptr, 0);
    int32_t rewardRead7 = SFileReadFile(file, &rewardOffset[7], sizeof(rewardOffset[7]), nullptr, nullptr, 0);
    int32_t rewardRead8 = SFileReadFile(file, &rewardOffset[8], sizeof(rewardOffset[8]), nullptr, nullptr, 0);
    int32_t rewardRead9 = SFileReadFile(file, &rewardOffset[9], sizeof(rewardOffset[9]), nullptr, nullptr, 0);
    int32_t rewardRead10 = SFileReadFile(file, &rewardOffset[10], sizeof(rewardOffset[10]), nullptr, nullptr, 0);
    int32_t rewardRead11 = SFileReadFile(file, &rewardOffset[11], sizeof(rewardOffset[11]), nullptr, nullptr, 0);
    int32_t rewardRead12 = SFileReadFile(file, &rewardOffset[12], sizeof(rewardOffset[12]), nullptr, nullptr, 0);
    int32_t rewardRead13 = SFileReadFile(file, &rewardOffset[13], sizeof(rewardOffset[13]), nullptr, nullptr, 0);
    int32_t rewardRead14 = SFileReadFile(file, &rewardOffset[14], sizeof(rewardOffset[14]), nullptr, nullptr, 0);
    int32_t rewardRead15 = SFileReadFile(file, &rewardOffset[15], sizeof(rewardOffset[15]), nullptr, nullptr, 0);
    int32_t rewardFlagsRead = SFileReadFile(file, &rewardFlags, sizeof(rewardFlags), nullptr, nullptr, 0);
    int32_t minimumCriteriaRead = SFileReadFile(file, &this->m_minimumCriteria, sizeof(this->m_minimumCriteria), nullptr, nullptr, 0);
    int32_t sharesCriteriaRead = SFileReadFile(file, &this->m_sharesCriteria, sizeof(this->m_sharesCriteria), nullptr, nullptr, 0);

    if (!idRead
        | !factionRead
        | !instanceIDRead
        | !supercedesRead
        | !titleRead0
        | !titleRead1
        | !titleRead2
        | !titleRead3
        | !titleRead4
        | !titleRead5
        | !titleRead6
        | !titleRead7
        | !titleRead8
        | !titleRead9
        | !titleRead10
        | !titleRead11
        | !titleRead12
        | !titleRead13
        | !titleRead14
        | !titleRead15
        | !titleFlagsRead
        | !descriptionRead0
        | !descriptionRead1
        | !descriptionRead2
        | !descriptionRead3
        | !descriptionRead4
        | !descriptionRead5
        | !descriptionRead6
        | !descriptionRead7
        | !descriptionRead8
        | !descriptionRead9
        | !descriptionRead10
        | !descriptionRead11
        | !descriptionRead12
        | !descriptionRead13
        | !descriptionRead14
        | !descriptionRead15
        | !descriptionFlagsRead
        | !categoryRead
        | !pointsRead
        | !uiOrderRead
        | !flagsRead
        | !iconIDRead
        | !rewardRead0
        | !rewardRead1
        | !rewardRead2
        | !rewardRead3
        | !rewardRead4
        | !rewardRead5
        | !rewardRead6
        | !rewardRead7
        | !rewardRead8
        | !rewardRead9
        | !rewardRead10
        | !rewardRead11
        | !rewardRead12
        | !rewardRead13
        | !rewardRead14
        | !rewardRead15
        | !rewardFlagsRead
        | !minimumCriteriaRead
        | !sharesCriteriaRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_title = &stringBuffer[titleOffset[s_locale]];
        this->m_description = &stringBuffer[descriptionOffset[s_locale]];
        this->m_reward = &stringBuffer[rewardOffset[s_locale]];
    } else {
        this->m_title = "";
        this->m_description = "";
        this->m_reward = "";
    }

    return true;
}
