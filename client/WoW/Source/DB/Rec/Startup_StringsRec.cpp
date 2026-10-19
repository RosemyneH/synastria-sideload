#include "WoW/Source/DB/Rec/Startup_StringsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

Startup_StringsRec::Startup_StringsRec() {
}

Startup_StringsRec::~Startup_StringsRec() {
}

// VERIFY: d5cac60d auto
// OFFSET: 0x00464090
const char* Startup_StringsRec::GetFilename() {
    return "DBFilesClient\\Startup_Strings.dbc";
}

// VERIFY: 33e52feb auto
// OFFSET: 0x004640A0
bool Startup_StringsRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;
    uint32_t messageOffset[16];
    uint32_t messageFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t messageRead0 = SFileReadFile(file, &messageOffset[0], sizeof(messageOffset[0]), nullptr, nullptr, 0);
    int32_t messageRead1 = SFileReadFile(file, &messageOffset[1], sizeof(messageOffset[1]), nullptr, nullptr, 0);
    int32_t messageRead2 = SFileReadFile(file, &messageOffset[2], sizeof(messageOffset[2]), nullptr, nullptr, 0);
    int32_t messageRead3 = SFileReadFile(file, &messageOffset[3], sizeof(messageOffset[3]), nullptr, nullptr, 0);
    int32_t messageRead4 = SFileReadFile(file, &messageOffset[4], sizeof(messageOffset[4]), nullptr, nullptr, 0);
    int32_t messageRead5 = SFileReadFile(file, &messageOffset[5], sizeof(messageOffset[5]), nullptr, nullptr, 0);
    int32_t messageRead6 = SFileReadFile(file, &messageOffset[6], sizeof(messageOffset[6]), nullptr, nullptr, 0);
    int32_t messageRead7 = SFileReadFile(file, &messageOffset[7], sizeof(messageOffset[7]), nullptr, nullptr, 0);
    int32_t messageRead8 = SFileReadFile(file, &messageOffset[8], sizeof(messageOffset[8]), nullptr, nullptr, 0);
    int32_t messageRead9 = SFileReadFile(file, &messageOffset[9], sizeof(messageOffset[9]), nullptr, nullptr, 0);
    int32_t messageRead10 = SFileReadFile(file, &messageOffset[10], sizeof(messageOffset[10]), nullptr, nullptr, 0);
    int32_t messageRead11 = SFileReadFile(file, &messageOffset[11], sizeof(messageOffset[11]), nullptr, nullptr, 0);
    int32_t messageRead12 = SFileReadFile(file, &messageOffset[12], sizeof(messageOffset[12]), nullptr, nullptr, 0);
    int32_t messageRead13 = SFileReadFile(file, &messageOffset[13], sizeof(messageOffset[13]), nullptr, nullptr, 0);
    int32_t messageRead14 = SFileReadFile(file, &messageOffset[14], sizeof(messageOffset[14]), nullptr, nullptr, 0);
    int32_t messageRead15 = SFileReadFile(file, &messageOffset[15], sizeof(messageOffset[15]), nullptr, nullptr, 0);
    int32_t messageFlagsRead = SFileReadFile(file, &messageFlags, sizeof(messageFlags), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead
        | !messageRead0
        | !messageRead1
        | !messageRead2
        | !messageRead3
        | !messageRead4
        | !messageRead5
        | !messageRead6
        | !messageRead7
        | !messageRead8
        | !messageRead9
        | !messageRead10
        | !messageRead11
        | !messageRead12
        | !messageRead13
        | !messageRead14
        | !messageRead15
        | !messageFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
        this->m_message = &stringBuffer[messageOffset[s_locale]];
    } else {
        this->m_name = "";
        this->m_message = "";
    }

    return true;
}
