#include "WoW/Source/DB/Rec/QuestInfoRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

QuestInfoRec::QuestInfoRec() {
}

QuestInfoRec::~QuestInfoRec() {
}

// VERIFY: c32e0167 auto
// OFFSET: 0x008B52E0
const char* QuestInfoRec::GetFilename() {
    return "DBFilesClient\\QuestInfo.dbc";
}

bool QuestInfoRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t infoNameOffset[16];
    uint32_t infoNameFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t infoNameRead0 = SFileReadFile(file, &infoNameOffset[0], sizeof(infoNameOffset[0]), nullptr, nullptr, 0);
    int32_t infoNameRead1 = SFileReadFile(file, &infoNameOffset[1], sizeof(infoNameOffset[1]), nullptr, nullptr, 0);
    int32_t infoNameRead2 = SFileReadFile(file, &infoNameOffset[2], sizeof(infoNameOffset[2]), nullptr, nullptr, 0);
    int32_t infoNameRead3 = SFileReadFile(file, &infoNameOffset[3], sizeof(infoNameOffset[3]), nullptr, nullptr, 0);
    int32_t infoNameRead4 = SFileReadFile(file, &infoNameOffset[4], sizeof(infoNameOffset[4]), nullptr, nullptr, 0);
    int32_t infoNameRead5 = SFileReadFile(file, &infoNameOffset[5], sizeof(infoNameOffset[5]), nullptr, nullptr, 0);
    int32_t infoNameRead6 = SFileReadFile(file, &infoNameOffset[6], sizeof(infoNameOffset[6]), nullptr, nullptr, 0);
    int32_t infoNameRead7 = SFileReadFile(file, &infoNameOffset[7], sizeof(infoNameOffset[7]), nullptr, nullptr, 0);
    int32_t infoNameRead8 = SFileReadFile(file, &infoNameOffset[8], sizeof(infoNameOffset[8]), nullptr, nullptr, 0);
    int32_t infoNameRead9 = SFileReadFile(file, &infoNameOffset[9], sizeof(infoNameOffset[9]), nullptr, nullptr, 0);
    int32_t infoNameRead10 = SFileReadFile(file, &infoNameOffset[10], sizeof(infoNameOffset[10]), nullptr, nullptr, 0);
    int32_t infoNameRead11 = SFileReadFile(file, &infoNameOffset[11], sizeof(infoNameOffset[11]), nullptr, nullptr, 0);
    int32_t infoNameRead12 = SFileReadFile(file, &infoNameOffset[12], sizeof(infoNameOffset[12]), nullptr, nullptr, 0);
    int32_t infoNameRead13 = SFileReadFile(file, &infoNameOffset[13], sizeof(infoNameOffset[13]), nullptr, nullptr, 0);
    int32_t infoNameRead14 = SFileReadFile(file, &infoNameOffset[14], sizeof(infoNameOffset[14]), nullptr, nullptr, 0);
    int32_t infoNameRead15 = SFileReadFile(file, &infoNameOffset[15], sizeof(infoNameOffset[15]), nullptr, nullptr, 0);
    int32_t infoNameFlagsRead = SFileReadFile(file, &infoNameFlags, sizeof(infoNameFlags), nullptr, nullptr, 0);

    if (!idRead
        | !infoNameRead0
        | !infoNameRead1
        | !infoNameRead2
        | !infoNameRead3
        | !infoNameRead4
        | !infoNameRead5
        | !infoNameRead6
        | !infoNameRead7
        | !infoNameRead8
        | !infoNameRead9
        | !infoNameRead10
        | !infoNameRead11
        | !infoNameRead12
        | !infoNameRead13
        | !infoNameRead14
        | !infoNameRead15
        | !infoNameFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_infoName = &stringBuffer[infoNameOffset[s_locale]];
    } else {
        this->m_infoName = "";
    }

    return true;
}
