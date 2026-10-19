#include "WoW/Source/DB/Rec/ChatChannelsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

ChatChannelsRec::ChatChannelsRec() {
}

ChatChannelsRec::~ChatChannelsRec() {
}

// VERIFY: ed5e8df4 auto
// OFFSET: 0x008AB530
const char* ChatChannelsRec::GetFilename() {
    return "DBFilesClient\\ChatChannels.dbc";
}

// VERIFY: b42ab8cc auto
// OFFSET: 0x008AB540
bool ChatChannelsRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset[16];
    uint32_t nameFlags;
    uint32_t shortcutOffset[16];
    uint32_t shortcutFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t factionGroupRead = SFileReadFile(file, &this->m_factionGroup, sizeof(this->m_factionGroup), nullptr, nullptr, 0);
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
    int32_t shortcutRead0 = SFileReadFile(file, &shortcutOffset[0], sizeof(shortcutOffset[0]), nullptr, nullptr, 0);
    int32_t shortcutRead1 = SFileReadFile(file, &shortcutOffset[1], sizeof(shortcutOffset[1]), nullptr, nullptr, 0);
    int32_t shortcutRead2 = SFileReadFile(file, &shortcutOffset[2], sizeof(shortcutOffset[2]), nullptr, nullptr, 0);
    int32_t shortcutRead3 = SFileReadFile(file, &shortcutOffset[3], sizeof(shortcutOffset[3]), nullptr, nullptr, 0);
    int32_t shortcutRead4 = SFileReadFile(file, &shortcutOffset[4], sizeof(shortcutOffset[4]), nullptr, nullptr, 0);
    int32_t shortcutRead5 = SFileReadFile(file, &shortcutOffset[5], sizeof(shortcutOffset[5]), nullptr, nullptr, 0);
    int32_t shortcutRead6 = SFileReadFile(file, &shortcutOffset[6], sizeof(shortcutOffset[6]), nullptr, nullptr, 0);
    int32_t shortcutRead7 = SFileReadFile(file, &shortcutOffset[7], sizeof(shortcutOffset[7]), nullptr, nullptr, 0);
    int32_t shortcutRead8 = SFileReadFile(file, &shortcutOffset[8], sizeof(shortcutOffset[8]), nullptr, nullptr, 0);
    int32_t shortcutRead9 = SFileReadFile(file, &shortcutOffset[9], sizeof(shortcutOffset[9]), nullptr, nullptr, 0);
    int32_t shortcutRead10 = SFileReadFile(file, &shortcutOffset[10], sizeof(shortcutOffset[10]), nullptr, nullptr, 0);
    int32_t shortcutRead11 = SFileReadFile(file, &shortcutOffset[11], sizeof(shortcutOffset[11]), nullptr, nullptr, 0);
    int32_t shortcutRead12 = SFileReadFile(file, &shortcutOffset[12], sizeof(shortcutOffset[12]), nullptr, nullptr, 0);
    int32_t shortcutRead13 = SFileReadFile(file, &shortcutOffset[13], sizeof(shortcutOffset[13]), nullptr, nullptr, 0);
    int32_t shortcutRead14 = SFileReadFile(file, &shortcutOffset[14], sizeof(shortcutOffset[14]), nullptr, nullptr, 0);
    int32_t shortcutRead15 = SFileReadFile(file, &shortcutOffset[15], sizeof(shortcutOffset[15]), nullptr, nullptr, 0);
    int32_t shortcutFlagsRead = SFileReadFile(file, &shortcutFlags, sizeof(shortcutFlags), nullptr, nullptr, 0);

    if (!idRead
        | !flagsRead
        | !factionGroupRead
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
        | !shortcutRead0
        | !shortcutRead1
        | !shortcutRead2
        | !shortcutRead3
        | !shortcutRead4
        | !shortcutRead5
        | !shortcutRead6
        | !shortcutRead7
        | !shortcutRead8
        | !shortcutRead9
        | !shortcutRead10
        | !shortcutRead11
        | !shortcutRead12
        | !shortcutRead13
        | !shortcutRead14
        | !shortcutRead15
        | !shortcutFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset[s_locale]];
        this->m_shortcut = &stringBuffer[shortcutOffset[s_locale]];
    } else {
        this->m_name = "";
        this->m_shortcut = "";
    }

    return true;
}
