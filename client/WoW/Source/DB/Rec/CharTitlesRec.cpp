#include "WoW/Source/DB/Rec/CharTitlesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

CharTitlesRec::CharTitlesRec() {
}

CharTitlesRec::~CharTitlesRec() {
}

// VERIFY: e4946604 auto
// OFFSET: 0x008AAF30
const char* CharTitlesRec::GetFilename() {
    return "DBFilesClient\\CharTitles.dbc";
}

// VERIFY: 470cc4ce auto
// OFFSET: 0x008AAF40
bool CharTitlesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset[16];
    uint32_t nameFlags;
    uint32_t name1Offset[16];
    uint32_t name1Flags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t conditionIDRead = SFileReadFile(file, &this->m_conditionID, sizeof(this->m_conditionID), nullptr, nullptr, 0);
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
    int32_t name1Read0 = SFileReadFile(file, &name1Offset[0], sizeof(name1Offset[0]), nullptr, nullptr, 0);
    int32_t name1Read1 = SFileReadFile(file, &name1Offset[1], sizeof(name1Offset[1]), nullptr, nullptr, 0);
    int32_t name1Read2 = SFileReadFile(file, &name1Offset[2], sizeof(name1Offset[2]), nullptr, nullptr, 0);
    int32_t name1Read3 = SFileReadFile(file, &name1Offset[3], sizeof(name1Offset[3]), nullptr, nullptr, 0);
    int32_t name1Read4 = SFileReadFile(file, &name1Offset[4], sizeof(name1Offset[4]), nullptr, nullptr, 0);
    int32_t name1Read5 = SFileReadFile(file, &name1Offset[5], sizeof(name1Offset[5]), nullptr, nullptr, 0);
    int32_t name1Read6 = SFileReadFile(file, &name1Offset[6], sizeof(name1Offset[6]), nullptr, nullptr, 0);
    int32_t name1Read7 = SFileReadFile(file, &name1Offset[7], sizeof(name1Offset[7]), nullptr, nullptr, 0);
    int32_t name1Read8 = SFileReadFile(file, &name1Offset[8], sizeof(name1Offset[8]), nullptr, nullptr, 0);
    int32_t name1Read9 = SFileReadFile(file, &name1Offset[9], sizeof(name1Offset[9]), nullptr, nullptr, 0);
    int32_t name1Read10 = SFileReadFile(file, &name1Offset[10], sizeof(name1Offset[10]), nullptr, nullptr, 0);
    int32_t name1Read11 = SFileReadFile(file, &name1Offset[11], sizeof(name1Offset[11]), nullptr, nullptr, 0);
    int32_t name1Read12 = SFileReadFile(file, &name1Offset[12], sizeof(name1Offset[12]), nullptr, nullptr, 0);
    int32_t name1Read13 = SFileReadFile(file, &name1Offset[13], sizeof(name1Offset[13]), nullptr, nullptr, 0);
    int32_t name1Read14 = SFileReadFile(file, &name1Offset[14], sizeof(name1Offset[14]), nullptr, nullptr, 0);
    int32_t name1Read15 = SFileReadFile(file, &name1Offset[15], sizeof(name1Offset[15]), nullptr, nullptr, 0);
    int32_t name1FlagsRead = SFileReadFile(file, &name1Flags, sizeof(name1Flags), nullptr, nullptr, 0);
    int32_t maskIDRead = SFileReadFile(file, &this->m_maskID, sizeof(this->m_maskID), nullptr, nullptr, 0);

    if (!idRead
        | !conditionIDRead
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
        | !name1Read0
        | !name1Read1
        | !name1Read2
        | !name1Read3
        | !name1Read4
        | !name1Read5
        | !name1Read6
        | !name1Read7
        | !name1Read8
        | !name1Read9
        | !name1Read10
        | !name1Read11
        | !name1Read12
        | !name1Read13
        | !name1Read14
        | !name1Read15
        | !name1FlagsRead
        | !maskIDRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset[s_locale]];
        this->m_name1 = &stringBuffer[name1Offset[s_locale]];
    } else {
        this->m_name = "";
        this->m_name1 = "";
    }

    return true;
}
