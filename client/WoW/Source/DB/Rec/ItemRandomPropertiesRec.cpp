#include "WoW/Source/DB/Rec/ItemRandomPropertiesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

ItemRandomPropertiesRec::ItemRandomPropertiesRec() {
}

ItemRandomPropertiesRec::~ItemRandomPropertiesRec() {
}

// VERIFY: 16babe00 auto
// OFFSET: 0x008B0700
const char* ItemRandomPropertiesRec::GetFilename() {
    return "DBFilesClient\\ItemRandomProperties.dbc";
}

// VERIFY: c93210f0 auto
// OFFSET: 0x008B0710
bool ItemRandomPropertiesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;
    uint32_t nameLangOffset[16];
    uint32_t nameLangFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t enchantmentRead = SFileReadFile(file, this->m_enchantment, sizeof(this->m_enchantment), nullptr, nullptr, 0);
    int32_t nameLangRead0 = SFileReadFile(file, &nameLangOffset[0], sizeof(nameLangOffset[0]), nullptr, nullptr, 0);
    int32_t nameLangRead1 = SFileReadFile(file, &nameLangOffset[1], sizeof(nameLangOffset[1]), nullptr, nullptr, 0);
    int32_t nameLangRead2 = SFileReadFile(file, &nameLangOffset[2], sizeof(nameLangOffset[2]), nullptr, nullptr, 0);
    int32_t nameLangRead3 = SFileReadFile(file, &nameLangOffset[3], sizeof(nameLangOffset[3]), nullptr, nullptr, 0);
    int32_t nameLangRead4 = SFileReadFile(file, &nameLangOffset[4], sizeof(nameLangOffset[4]), nullptr, nullptr, 0);
    int32_t nameLangRead5 = SFileReadFile(file, &nameLangOffset[5], sizeof(nameLangOffset[5]), nullptr, nullptr, 0);
    int32_t nameLangRead6 = SFileReadFile(file, &nameLangOffset[6], sizeof(nameLangOffset[6]), nullptr, nullptr, 0);
    int32_t nameLangRead7 = SFileReadFile(file, &nameLangOffset[7], sizeof(nameLangOffset[7]), nullptr, nullptr, 0);
    int32_t nameLangRead8 = SFileReadFile(file, &nameLangOffset[8], sizeof(nameLangOffset[8]), nullptr, nullptr, 0);
    int32_t nameLangRead9 = SFileReadFile(file, &nameLangOffset[9], sizeof(nameLangOffset[9]), nullptr, nullptr, 0);
    int32_t nameLangRead10 = SFileReadFile(file, &nameLangOffset[10], sizeof(nameLangOffset[10]), nullptr, nullptr, 0);
    int32_t nameLangRead11 = SFileReadFile(file, &nameLangOffset[11], sizeof(nameLangOffset[11]), nullptr, nullptr, 0);
    int32_t nameLangRead12 = SFileReadFile(file, &nameLangOffset[12], sizeof(nameLangOffset[12]), nullptr, nullptr, 0);
    int32_t nameLangRead13 = SFileReadFile(file, &nameLangOffset[13], sizeof(nameLangOffset[13]), nullptr, nullptr, 0);
    int32_t nameLangRead14 = SFileReadFile(file, &nameLangOffset[14], sizeof(nameLangOffset[14]), nullptr, nullptr, 0);
    int32_t nameLangRead15 = SFileReadFile(file, &nameLangOffset[15], sizeof(nameLangOffset[15]), nullptr, nullptr, 0);
    int32_t nameLangFlagsRead = SFileReadFile(file, &nameLangFlags, sizeof(nameLangFlags), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead
        | !enchantmentRead
        | !nameLangRead0
        | !nameLangRead1
        | !nameLangRead2
        | !nameLangRead3
        | !nameLangRead4
        | !nameLangRead5
        | !nameLangRead6
        | !nameLangRead7
        | !nameLangRead8
        | !nameLangRead9
        | !nameLangRead10
        | !nameLangRead11
        | !nameLangRead12
        | !nameLangRead13
        | !nameLangRead14
        | !nameLangRead15
        | !nameLangFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
        this->m_nameLang = &stringBuffer[nameLangOffset[s_locale]];
    } else {
        this->m_name = "";
        this->m_nameLang = "";
    }

    return true;
}
