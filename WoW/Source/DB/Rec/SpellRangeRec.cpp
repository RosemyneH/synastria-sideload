#include "WoW/Source/DB/Rec/SpellRangeRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

SpellRangeRec::SpellRangeRec() {
}

SpellRangeRec::~SpellRangeRec() {
}

// VERIFY: 01d51ea2 auto
// OFFSET: 0x008B8600
const char* SpellRangeRec::GetFilename() {
    return "DBFilesClient\\SpellRange.dbc";
}

// VERIFY: 11964489 auto
// OFFSET: 0x008B8610
bool SpellRangeRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t displayNameOffset[16];
    uint32_t displayNameFlags;
    uint32_t displayNameShortOffset[16];
    uint32_t displayNameShortFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t rangeMinRead = SFileReadFile(file, this->m_rangeMin, sizeof(this->m_rangeMin), nullptr, nullptr, 0);
    int32_t rangeMaxRead = SFileReadFile(file, this->m_rangeMax, sizeof(this->m_rangeMax), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
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
    int32_t displayNameShortRead0 = SFileReadFile(file, &displayNameShortOffset[0], sizeof(displayNameShortOffset[0]), nullptr, nullptr, 0);
    int32_t displayNameShortRead1 = SFileReadFile(file, &displayNameShortOffset[1], sizeof(displayNameShortOffset[1]), nullptr, nullptr, 0);
    int32_t displayNameShortRead2 = SFileReadFile(file, &displayNameShortOffset[2], sizeof(displayNameShortOffset[2]), nullptr, nullptr, 0);
    int32_t displayNameShortRead3 = SFileReadFile(file, &displayNameShortOffset[3], sizeof(displayNameShortOffset[3]), nullptr, nullptr, 0);
    int32_t displayNameShortRead4 = SFileReadFile(file, &displayNameShortOffset[4], sizeof(displayNameShortOffset[4]), nullptr, nullptr, 0);
    int32_t displayNameShortRead5 = SFileReadFile(file, &displayNameShortOffset[5], sizeof(displayNameShortOffset[5]), nullptr, nullptr, 0);
    int32_t displayNameShortRead6 = SFileReadFile(file, &displayNameShortOffset[6], sizeof(displayNameShortOffset[6]), nullptr, nullptr, 0);
    int32_t displayNameShortRead7 = SFileReadFile(file, &displayNameShortOffset[7], sizeof(displayNameShortOffset[7]), nullptr, nullptr, 0);
    int32_t displayNameShortRead8 = SFileReadFile(file, &displayNameShortOffset[8], sizeof(displayNameShortOffset[8]), nullptr, nullptr, 0);
    int32_t displayNameShortRead9 = SFileReadFile(file, &displayNameShortOffset[9], sizeof(displayNameShortOffset[9]), nullptr, nullptr, 0);
    int32_t displayNameShortRead10 = SFileReadFile(file, &displayNameShortOffset[10], sizeof(displayNameShortOffset[10]), nullptr, nullptr, 0);
    int32_t displayNameShortRead11 = SFileReadFile(file, &displayNameShortOffset[11], sizeof(displayNameShortOffset[11]), nullptr, nullptr, 0);
    int32_t displayNameShortRead12 = SFileReadFile(file, &displayNameShortOffset[12], sizeof(displayNameShortOffset[12]), nullptr, nullptr, 0);
    int32_t displayNameShortRead13 = SFileReadFile(file, &displayNameShortOffset[13], sizeof(displayNameShortOffset[13]), nullptr, nullptr, 0);
    int32_t displayNameShortRead14 = SFileReadFile(file, &displayNameShortOffset[14], sizeof(displayNameShortOffset[14]), nullptr, nullptr, 0);
    int32_t displayNameShortRead15 = SFileReadFile(file, &displayNameShortOffset[15], sizeof(displayNameShortOffset[15]), nullptr, nullptr, 0);
    int32_t displayNameShortFlagsRead = SFileReadFile(file, &displayNameShortFlags, sizeof(displayNameShortFlags), nullptr, nullptr, 0);

    if (!idRead
        | !rangeMinRead
        | !rangeMaxRead
        | !flagsRead
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
        | !displayNameShortRead0
        | !displayNameShortRead1
        | !displayNameShortRead2
        | !displayNameShortRead3
        | !displayNameShortRead4
        | !displayNameShortRead5
        | !displayNameShortRead6
        | !displayNameShortRead7
        | !displayNameShortRead8
        | !displayNameShortRead9
        | !displayNameShortRead10
        | !displayNameShortRead11
        | !displayNameShortRead12
        | !displayNameShortRead13
        | !displayNameShortRead14
        | !displayNameShortRead15
        | !displayNameShortFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_displayName = &stringBuffer[displayNameOffset[s_locale]];
        this->m_displayNameShort = &stringBuffer[displayNameShortOffset[s_locale]];
    } else {
        this->m_displayName = "";
        this->m_displayNameShort = "";
    }

    return true;
}
