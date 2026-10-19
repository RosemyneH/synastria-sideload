#include "WoW/Source/DB/Rec/Achievement_CriteriaRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

Achievement_CriteriaRec::Achievement_CriteriaRec() {
}

Achievement_CriteriaRec::~Achievement_CriteriaRec() {
}

// VERIFY: 3fed5d5e auto
// OFFSET: 0x008A8CB0
const char* Achievement_CriteriaRec::GetFilename() {
    return "DBFilesClient\\Achievement_Criteria.dbc";
}

// VERIFY: 578704de auto
// OFFSET: 0x008A8CC0
bool Achievement_CriteriaRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t descriptionOffset[16];
    uint32_t descriptionFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t achievementIDRead = SFileReadFile(file, &this->m_achievementID, sizeof(this->m_achievementID), nullptr, nullptr, 0);
    int32_t typeRead = SFileReadFile(file, &this->m_type, sizeof(this->m_type), nullptr, nullptr, 0);
    int32_t assetIDRead = SFileReadFile(file, &this->m_assetID, sizeof(this->m_assetID), nullptr, nullptr, 0);
    int32_t quantityRead = SFileReadFile(file, &this->m_quantity, sizeof(this->m_quantity), nullptr, nullptr, 0);
    int32_t startEventRead = SFileReadFile(file, &this->m_startEvent, sizeof(this->m_startEvent), nullptr, nullptr, 0);
    int32_t startAssetRead = SFileReadFile(file, &this->m_startAsset, sizeof(this->m_startAsset), nullptr, nullptr, 0);
    int32_t failEventRead = SFileReadFile(file, &this->m_failEvent, sizeof(this->m_failEvent), nullptr, nullptr, 0);
    int32_t failAssetRead = SFileReadFile(file, &this->m_failAsset, sizeof(this->m_failAsset), nullptr, nullptr, 0);
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
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t timerStartEventRead = SFileReadFile(file, &this->m_timerStartEvent, sizeof(this->m_timerStartEvent), nullptr, nullptr, 0);
    int32_t timerAssetIDRead = SFileReadFile(file, &this->m_timerAssetID, sizeof(this->m_timerAssetID), nullptr, nullptr, 0);
    int32_t timerTimeRead = SFileReadFile(file, &this->m_timerTime, sizeof(this->m_timerTime), nullptr, nullptr, 0);
    int32_t uiOrderRead = SFileReadFile(file, &this->m_uiOrder, sizeof(this->m_uiOrder), nullptr, nullptr, 0);

    if (!idRead
        | !achievementIDRead
        | !typeRead
        | !assetIDRead
        | !quantityRead
        | !startEventRead
        | !startAssetRead
        | !failEventRead
        | !failAssetRead
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
        | !flagsRead
        | !timerStartEventRead
        | !timerAssetIDRead
        | !timerTimeRead
        | !uiOrderRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_description = &stringBuffer[descriptionOffset[s_locale]];
    } else {
        this->m_description = "";
    }

    return true;
}
