#include "WoW/Source/DB/Rec/QuestSortRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

QuestSortRec::QuestSortRec() {
}

QuestSortRec::~QuestSortRec() {
}

// VERIFY: 087755af auto
// OFFSET: 0x008B52F0
const char* QuestSortRec::GetFilename() {
    return "DBFilesClient\\QuestSort.dbc";
}

bool QuestSortRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t sortNameOffset[16];
    uint32_t sortNameFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t sortNameRead0 = SFileReadFile(file, &sortNameOffset[0], sizeof(sortNameOffset[0]), nullptr, nullptr, 0);
    int32_t sortNameRead1 = SFileReadFile(file, &sortNameOffset[1], sizeof(sortNameOffset[1]), nullptr, nullptr, 0);
    int32_t sortNameRead2 = SFileReadFile(file, &sortNameOffset[2], sizeof(sortNameOffset[2]), nullptr, nullptr, 0);
    int32_t sortNameRead3 = SFileReadFile(file, &sortNameOffset[3], sizeof(sortNameOffset[3]), nullptr, nullptr, 0);
    int32_t sortNameRead4 = SFileReadFile(file, &sortNameOffset[4], sizeof(sortNameOffset[4]), nullptr, nullptr, 0);
    int32_t sortNameRead5 = SFileReadFile(file, &sortNameOffset[5], sizeof(sortNameOffset[5]), nullptr, nullptr, 0);
    int32_t sortNameRead6 = SFileReadFile(file, &sortNameOffset[6], sizeof(sortNameOffset[6]), nullptr, nullptr, 0);
    int32_t sortNameRead7 = SFileReadFile(file, &sortNameOffset[7], sizeof(sortNameOffset[7]), nullptr, nullptr, 0);
    int32_t sortNameRead8 = SFileReadFile(file, &sortNameOffset[8], sizeof(sortNameOffset[8]), nullptr, nullptr, 0);
    int32_t sortNameRead9 = SFileReadFile(file, &sortNameOffset[9], sizeof(sortNameOffset[9]), nullptr, nullptr, 0);
    int32_t sortNameRead10 = SFileReadFile(file, &sortNameOffset[10], sizeof(sortNameOffset[10]), nullptr, nullptr, 0);
    int32_t sortNameRead11 = SFileReadFile(file, &sortNameOffset[11], sizeof(sortNameOffset[11]), nullptr, nullptr, 0);
    int32_t sortNameRead12 = SFileReadFile(file, &sortNameOffset[12], sizeof(sortNameOffset[12]), nullptr, nullptr, 0);
    int32_t sortNameRead13 = SFileReadFile(file, &sortNameOffset[13], sizeof(sortNameOffset[13]), nullptr, nullptr, 0);
    int32_t sortNameRead14 = SFileReadFile(file, &sortNameOffset[14], sizeof(sortNameOffset[14]), nullptr, nullptr, 0);
    int32_t sortNameRead15 = SFileReadFile(file, &sortNameOffset[15], sizeof(sortNameOffset[15]), nullptr, nullptr, 0);
    int32_t sortNameFlagsRead = SFileReadFile(file, &sortNameFlags, sizeof(sortNameFlags), nullptr, nullptr, 0);

    if (!idRead
        | !sortNameRead0
        | !sortNameRead1
        | !sortNameRead2
        | !sortNameRead3
        | !sortNameRead4
        | !sortNameRead5
        | !sortNameRead6
        | !sortNameRead7
        | !sortNameRead8
        | !sortNameRead9
        | !sortNameRead10
        | !sortNameRead11
        | !sortNameRead12
        | !sortNameRead13
        | !sortNameRead14
        | !sortNameRead15
        | !sortNameFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_sortName = &stringBuffer[sortNameOffset[s_locale]];
    } else {
        this->m_sortName = "";
    }

    return true;
}
