#include "WoW/Source/DB/Rec/BarberShopStyleRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

BarberShopStyleRec::BarberShopStyleRec() {
}

BarberShopStyleRec::~BarberShopStyleRec() {
}

// VERIFY: ee4cd2b0 auto
// OFFSET: 0x008AA140
const char* BarberShopStyleRec::GetFilename() {
    return "DBFilesClient\\BarberShopStyle.dbc";
}

// VERIFY: 8a1ae8d0 auto
// OFFSET: 0x008AA150
bool BarberShopStyleRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t displayNameOffset[16];
    uint32_t displayNameFlags;
    uint32_t descriptionOffset[16];
    uint32_t descriptionFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t typeRead = SFileReadFile(file, &this->m_type, sizeof(this->m_type), nullptr, nullptr, 0);
    int32_t displayNameRead0 = SFileReadFile(file, &displayNameOffset[0], sizeof(displayNameOffset[0]), nullptr, nullptr, 0);
    int32_t displayNameRead1 = SFileReadFile(file, &displayNameOffset[1], sizeof(displayNameOffset[1]), nullptr, nullptr, 0);
    int32_t displayNameRead2 = SFileReadFile(file, &displayNameOffset[2], sizeof(displayNameOffset[2]), nullptr, nullptr, 0);
    int32_t displayNameRead3 = SFileReadFile(file, &displayNameOffset[3], sizeof(displayNameOffset[3]), nullptr, nullptr, 0);
    int32_t displayNameRead4 = SFileReadFile(file, &displayNameOffset[4], sizeof(displayNameOffset[4]), nullptr, nullptr, 0);
    int32_t displayNameRead5 = SFileReadFile(file, &displayNameOffset[5], sizeof(displayNameOffset[5]), nullptr, nullptr, 0);
    int32_t displayNameRead6 = SFileReadFile(file, &displayNameOffset[6], sizeof(displayNameOffset[6]), nullptr, nullptr, 0);
    int32_t displayNameRead7 = SFileReadFile(file, &displayNameOffset[7], sizeof(displayNameOffset[7]), nullptr, nullptr, 0);
    int32_t displayNameRead8 = SFileReadFile(file, &displayNameOffset[8], sizeof(displayNameOffset[8]), nullptr, nullptr, 0);
    int32_t displayNameRead9 = SFileReadFile(file, &displayNameOffset[9], sizeof(displayNameOffset[9]), nullptr, nullptr, 0);
    int32_t displayNameRead10 = SFileReadFile(file, &displayNameOffset[10], sizeof(displayNameOffset[10]), nullptr, nullptr, 0);
    int32_t displayNameRead11 = SFileReadFile(file, &displayNameOffset[11], sizeof(displayNameOffset[11]), nullptr, nullptr, 0);
    int32_t displayNameRead12 = SFileReadFile(file, &displayNameOffset[12], sizeof(displayNameOffset[12]), nullptr, nullptr, 0);
    int32_t displayNameRead13 = SFileReadFile(file, &displayNameOffset[13], sizeof(displayNameOffset[13]), nullptr, nullptr, 0);
    int32_t displayNameRead14 = SFileReadFile(file, &displayNameOffset[14], sizeof(displayNameOffset[14]), nullptr, nullptr, 0);
    int32_t displayNameRead15 = SFileReadFile(file, &displayNameOffset[15], sizeof(displayNameOffset[15]), nullptr, nullptr, 0);
    int32_t displayNameFlagsRead = SFileReadFile(file, &displayNameFlags, sizeof(displayNameFlags), nullptr, nullptr, 0);
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
    int32_t costModifierRead = SFileReadFile(file, &this->m_costModifier, sizeof(this->m_costModifier), nullptr, nullptr, 0);
    int32_t raceRead = SFileReadFile(file, &this->m_race, sizeof(this->m_race), nullptr, nullptr, 0);
    int32_t sexRead = SFileReadFile(file, &this->m_sex, sizeof(this->m_sex), nullptr, nullptr, 0);
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!idRead
        | !typeRead
        | !displayNameRead0
        | !displayNameRead1
        | !displayNameRead2
        | !displayNameRead3
        | !displayNameRead4
        | !displayNameRead5
        | !displayNameRead6
        | !displayNameRead7
        | !displayNameRead8
        | !displayNameRead9
        | !displayNameRead10
        | !displayNameRead11
        | !displayNameRead12
        | !displayNameRead13
        | !displayNameRead14
        | !displayNameRead15
        | !displayNameFlagsRead
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
        | !costModifierRead
        | !raceRead
        | !sexRead
        | !dataRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_displayName = &stringBuffer[displayNameOffset[s_locale]];
        this->m_description = &stringBuffer[descriptionOffset[s_locale]];
    } else {
        this->m_displayName = "";
        this->m_description = "";
    }

    return true;
}
