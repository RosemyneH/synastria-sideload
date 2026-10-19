#include "WoW/Source/DB/Rec/SkillLineRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

SkillLineRec::SkillLineRec() {
}

SkillLineRec::~SkillLineRec() {
}

// VERIFY: 6b1cd64e auto
// OFFSET: 0x008B5EB0
const char* SkillLineRec::GetFilename() {
    return "DBFilesClient\\SkillLine.dbc";
}

// VERIFY: 8a831634 auto
// OFFSET: 0x008B5EC0
bool SkillLineRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t displayNameOffset[16];
    uint32_t displayNameFlags;
    uint32_t descriptionOffset[16];
    uint32_t descriptionFlags;
    uint32_t alternateVerbOffset[16];
    uint32_t alternateVerbFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t categoryIDRead = SFileReadFile(file, &this->m_categoryID, sizeof(this->m_categoryID), nullptr, nullptr, 0);
    int32_t skillCostsIDRead = SFileReadFile(file, &this->m_skillCostsID, sizeof(this->m_skillCostsID), nullptr, nullptr, 0);
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
    int32_t spellIconIDRead = SFileReadFile(file, &this->m_spellIconID, sizeof(this->m_spellIconID), nullptr, nullptr, 0);
    int32_t alternateVerbRead0 = SFileReadFile(file, &alternateVerbOffset[0], sizeof(alternateVerbOffset[0]), nullptr, nullptr, 0);
    int32_t alternateVerbRead1 = SFileReadFile(file, &alternateVerbOffset[1], sizeof(alternateVerbOffset[1]), nullptr, nullptr, 0);
    int32_t alternateVerbRead2 = SFileReadFile(file, &alternateVerbOffset[2], sizeof(alternateVerbOffset[2]), nullptr, nullptr, 0);
    int32_t alternateVerbRead3 = SFileReadFile(file, &alternateVerbOffset[3], sizeof(alternateVerbOffset[3]), nullptr, nullptr, 0);
    int32_t alternateVerbRead4 = SFileReadFile(file, &alternateVerbOffset[4], sizeof(alternateVerbOffset[4]), nullptr, nullptr, 0);
    int32_t alternateVerbRead5 = SFileReadFile(file, &alternateVerbOffset[5], sizeof(alternateVerbOffset[5]), nullptr, nullptr, 0);
    int32_t alternateVerbRead6 = SFileReadFile(file, &alternateVerbOffset[6], sizeof(alternateVerbOffset[6]), nullptr, nullptr, 0);
    int32_t alternateVerbRead7 = SFileReadFile(file, &alternateVerbOffset[7], sizeof(alternateVerbOffset[7]), nullptr, nullptr, 0);
    int32_t alternateVerbRead8 = SFileReadFile(file, &alternateVerbOffset[8], sizeof(alternateVerbOffset[8]), nullptr, nullptr, 0);
    int32_t alternateVerbRead9 = SFileReadFile(file, &alternateVerbOffset[9], sizeof(alternateVerbOffset[9]), nullptr, nullptr, 0);
    int32_t alternateVerbRead10 = SFileReadFile(file, &alternateVerbOffset[10], sizeof(alternateVerbOffset[10]), nullptr, nullptr, 0);
    int32_t alternateVerbRead11 = SFileReadFile(file, &alternateVerbOffset[11], sizeof(alternateVerbOffset[11]), nullptr, nullptr, 0);
    int32_t alternateVerbRead12 = SFileReadFile(file, &alternateVerbOffset[12], sizeof(alternateVerbOffset[12]), nullptr, nullptr, 0);
    int32_t alternateVerbRead13 = SFileReadFile(file, &alternateVerbOffset[13], sizeof(alternateVerbOffset[13]), nullptr, nullptr, 0);
    int32_t alternateVerbRead14 = SFileReadFile(file, &alternateVerbOffset[14], sizeof(alternateVerbOffset[14]), nullptr, nullptr, 0);
    int32_t alternateVerbRead15 = SFileReadFile(file, &alternateVerbOffset[15], sizeof(alternateVerbOffset[15]), nullptr, nullptr, 0);
    int32_t alternateVerbFlagsRead = SFileReadFile(file, &alternateVerbFlags, sizeof(alternateVerbFlags), nullptr, nullptr, 0);
    int32_t canLinkRead = SFileReadFile(file, &this->m_canLink, sizeof(this->m_canLink), nullptr, nullptr, 0);

    if (!idRead
        | !categoryIDRead
        | !skillCostsIDRead
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
        | !spellIconIDRead
        | !alternateVerbRead0
        | !alternateVerbRead1
        | !alternateVerbRead2
        | !alternateVerbRead3
        | !alternateVerbRead4
        | !alternateVerbRead5
        | !alternateVerbRead6
        | !alternateVerbRead7
        | !alternateVerbRead8
        | !alternateVerbRead9
        | !alternateVerbRead10
        | !alternateVerbRead11
        | !alternateVerbRead12
        | !alternateVerbRead13
        | !alternateVerbRead14
        | !alternateVerbRead15
        | !alternateVerbFlagsRead
        | !canLinkRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_displayName = &stringBuffer[displayNameOffset[s_locale]];
        this->m_description = &stringBuffer[descriptionOffset[s_locale]];
        this->m_alternateVerb = &stringBuffer[alternateVerbOffset[s_locale]];
    } else {
        this->m_displayName = "";
        this->m_description = "";
        this->m_alternateVerb = "";
    }

    return true;
}
