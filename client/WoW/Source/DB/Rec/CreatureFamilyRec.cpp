#include "WoW/Source/DB/Rec/CreatureFamilyRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

CreatureFamilyRec::CreatureFamilyRec() {
}

CreatureFamilyRec::~CreatureFamilyRec() {
}

// VERIFY: ce46b941 auto
// OFFSET: 0x008AD420
const char* CreatureFamilyRec::GetFilename() {
    return "DBFilesClient\\CreatureFamily.dbc";
}

// VERIFY: d61c294d auto
// OFFSET: 0x008AD430
bool CreatureFamilyRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset[16];
    uint32_t nameFlags;
    uint32_t iconFileOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t minScaleRead = SFileReadFile(file, &this->m_minScale, sizeof(this->m_minScale), nullptr, nullptr, 0);
    int32_t minScaleLevelRead = SFileReadFile(file, &this->m_minScaleLevel, sizeof(this->m_minScaleLevel), nullptr, nullptr, 0);
    int32_t maxScaleRead = SFileReadFile(file, &this->m_maxScale, sizeof(this->m_maxScale), nullptr, nullptr, 0);
    int32_t maxScaleLevelRead = SFileReadFile(file, &this->m_maxScaleLevel, sizeof(this->m_maxScaleLevel), nullptr, nullptr, 0);
    int32_t skillLineRead = SFileReadFile(file, this->m_skillLine, sizeof(this->m_skillLine), nullptr, nullptr, 0);
    int32_t petFoodMaskRead = SFileReadFile(file, &this->m_petFoodMask, sizeof(this->m_petFoodMask), nullptr, nullptr, 0);
    int32_t petTalentTypeRead = SFileReadFile(file, &this->m_petTalentType, sizeof(this->m_petTalentType), nullptr, nullptr, 0);
    int32_t categoryEnumIDRead = SFileReadFile(file, &this->m_categoryEnumID, sizeof(this->m_categoryEnumID), nullptr, nullptr, 0);
    int32_t nameRead0 = SFileReadFile(file, &nameOffset[0], sizeof(nameOffset[0]), nullptr, nullptr, 0);
    int32_t nameRead1 = SFileReadFile(file, &nameOffset[1], sizeof(nameOffset[1]), nullptr, nullptr, 0);
    int32_t nameRead2 = SFileReadFile(file, &nameOffset[2], sizeof(nameOffset[2]), nullptr, nullptr, 0);
    int32_t nameRead3 = SFileReadFile(file, &nameOffset[3], sizeof(nameOffset[3]), nullptr, nullptr, 0);
    int32_t nameRead4 = SFileReadFile(file, &nameOffset[4], sizeof(nameOffset[4]), nullptr, nullptr, 0);
    int32_t nameRead5 = SFileReadFile(file, &nameOffset[5], sizeof(nameOffset[5]), nullptr, nullptr, 0);
    int32_t nameRead6 = SFileReadFile(file, &nameOffset[6], sizeof(nameOffset[6]), nullptr, nullptr, 0);
    int32_t nameRead7 = SFileReadFile(file, &nameOffset[7], sizeof(nameOffset[7]), nullptr, nullptr, 0);
    int32_t nameRead8 = SFileReadFile(file, &nameOffset[8], sizeof(nameOffset[8]), nullptr, nullptr, 0);
    int32_t nameRead9 = SFileReadFile(file, &nameOffset[9], sizeof(nameOffset[9]), nullptr, nullptr, 0);
    int32_t nameRead10 = SFileReadFile(file, &nameOffset[10], sizeof(nameOffset[10]), nullptr, nullptr, 0);
    int32_t nameRead11 = SFileReadFile(file, &nameOffset[11], sizeof(nameOffset[11]), nullptr, nullptr, 0);
    int32_t nameRead12 = SFileReadFile(file, &nameOffset[12], sizeof(nameOffset[12]), nullptr, nullptr, 0);
    int32_t nameRead13 = SFileReadFile(file, &nameOffset[13], sizeof(nameOffset[13]), nullptr, nullptr, 0);
    int32_t nameRead14 = SFileReadFile(file, &nameOffset[14], sizeof(nameOffset[14]), nullptr, nullptr, 0);
    int32_t nameRead15 = SFileReadFile(file, &nameOffset[15], sizeof(nameOffset[15]), nullptr, nullptr, 0);
    int32_t nameFlagsRead = SFileReadFile(file, &nameFlags, sizeof(nameFlags), nullptr, nullptr, 0);
    int32_t iconFileRead = SFileReadFile(file, &iconFileOffset, sizeof(iconFileOffset), nullptr, nullptr, 0);

    if (!idRead
        | !minScaleRead
        | !minScaleLevelRead
        | !maxScaleRead
        | !maxScaleLevelRead
        | !skillLineRead
        | !petFoodMaskRead
        | !petTalentTypeRead
        | !categoryEnumIDRead
        | !nameRead0
        | !nameRead1
        | !nameRead2
        | !nameRead3
        | !nameRead4
        | !nameRead5
        | !nameRead6
        | !nameRead7
        | !nameRead8
        | !nameRead9
        | !nameRead10
        | !nameRead11
        | !nameRead12
        | !nameRead13
        | !nameRead14
        | !nameRead15
        | !nameFlagsRead
        | !iconFileRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset[s_locale]];
        this->m_iconFile = &stringBuffer[iconFileOffset];
    } else {
        this->m_name = "";
        this->m_iconFile = "";
    }

    return true;
}
