#include "WoW/Source/DB/Rec/ServerMessagesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

ServerMessagesRec::ServerMessagesRec() {
}

ServerMessagesRec::~ServerMessagesRec() {
}

// VERIFY: 05d24ae4 auto
// OFFSET: 0x008B5B60
const char* ServerMessagesRec::GetFilename() {
    return "DBFilesClient\\ServerMessages.dbc";
}

bool ServerMessagesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t textOffset[16];
    uint32_t textFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t textRead0 = SFileReadFile(file, &textOffset[0], sizeof(textOffset[0]), nullptr, nullptr, 0);
    int32_t textRead1 = SFileReadFile(file, &textOffset[1], sizeof(textOffset[1]), nullptr, nullptr, 0);
    int32_t textRead2 = SFileReadFile(file, &textOffset[2], sizeof(textOffset[2]), nullptr, nullptr, 0);
    int32_t textRead3 = SFileReadFile(file, &textOffset[3], sizeof(textOffset[3]), nullptr, nullptr, 0);
    int32_t textRead4 = SFileReadFile(file, &textOffset[4], sizeof(textOffset[4]), nullptr, nullptr, 0);
    int32_t textRead5 = SFileReadFile(file, &textOffset[5], sizeof(textOffset[5]), nullptr, nullptr, 0);
    int32_t textRead6 = SFileReadFile(file, &textOffset[6], sizeof(textOffset[6]), nullptr, nullptr, 0);
    int32_t textRead7 = SFileReadFile(file, &textOffset[7], sizeof(textOffset[7]), nullptr, nullptr, 0);
    int32_t textRead8 = SFileReadFile(file, &textOffset[8], sizeof(textOffset[8]), nullptr, nullptr, 0);
    int32_t textRead9 = SFileReadFile(file, &textOffset[9], sizeof(textOffset[9]), nullptr, nullptr, 0);
    int32_t textRead10 = SFileReadFile(file, &textOffset[10], sizeof(textOffset[10]), nullptr, nullptr, 0);
    int32_t textRead11 = SFileReadFile(file, &textOffset[11], sizeof(textOffset[11]), nullptr, nullptr, 0);
    int32_t textRead12 = SFileReadFile(file, &textOffset[12], sizeof(textOffset[12]), nullptr, nullptr, 0);
    int32_t textRead13 = SFileReadFile(file, &textOffset[13], sizeof(textOffset[13]), nullptr, nullptr, 0);
    int32_t textRead14 = SFileReadFile(file, &textOffset[14], sizeof(textOffset[14]), nullptr, nullptr, 0);
    int32_t textRead15 = SFileReadFile(file, &textOffset[15], sizeof(textOffset[15]), nullptr, nullptr, 0);
    int32_t textFlagsRead = SFileReadFile(file, &textFlags, sizeof(textFlags), nullptr, nullptr, 0);

    if (!idRead
        | !textRead0
        | !textRead1
        | !textRead2
        | !textRead3
        | !textRead4
        | !textRead5
        | !textRead6
        | !textRead7
        | !textRead8
        | !textRead9
        | !textRead10
        | !textRead11
        | !textRead12
        | !textRead13
        | !textRead14
        | !textRead15
        | !textFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_text = &stringBuffer[textOffset[s_locale]];
    } else {
        this->m_text = "";
    }

    return true;
}
