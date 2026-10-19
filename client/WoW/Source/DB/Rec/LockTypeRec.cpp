#include "WoW/Source/DB/Rec/LockTypeRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

LockTypeRec::LockTypeRec() {
}

LockTypeRec::~LockTypeRec() {
}

// VERIFY: 5d589169 auto
// OFFSET: 0x008B2D70
const char* LockTypeRec::GetFilename() {
    return "DBFilesClient\\LockType.dbc";
}

// VERIFY: edb56d40 auto
// OFFSET: 0x008B2D80
bool LockTypeRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset[16];
    uint32_t nameFlags;
    uint32_t resourceNameOffset[16];
    uint32_t resourceNameFlags;
    uint32_t verbOffset[16];
    uint32_t verbFlags;
    uint32_t cursorNameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
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
    int32_t resourceNameRead0 = SFileReadFile(file, &resourceNameOffset[0], sizeof(resourceNameOffset[0]), nullptr, nullptr, 0);
    int32_t resourceNameRead1 = SFileReadFile(file, &resourceNameOffset[1], sizeof(resourceNameOffset[1]), nullptr, nullptr, 0);
    int32_t resourceNameRead2 = SFileReadFile(file, &resourceNameOffset[2], sizeof(resourceNameOffset[2]), nullptr, nullptr, 0);
    int32_t resourceNameRead3 = SFileReadFile(file, &resourceNameOffset[3], sizeof(resourceNameOffset[3]), nullptr, nullptr, 0);
    int32_t resourceNameRead4 = SFileReadFile(file, &resourceNameOffset[4], sizeof(resourceNameOffset[4]), nullptr, nullptr, 0);
    int32_t resourceNameRead5 = SFileReadFile(file, &resourceNameOffset[5], sizeof(resourceNameOffset[5]), nullptr, nullptr, 0);
    int32_t resourceNameRead6 = SFileReadFile(file, &resourceNameOffset[6], sizeof(resourceNameOffset[6]), nullptr, nullptr, 0);
    int32_t resourceNameRead7 = SFileReadFile(file, &resourceNameOffset[7], sizeof(resourceNameOffset[7]), nullptr, nullptr, 0);
    int32_t resourceNameRead8 = SFileReadFile(file, &resourceNameOffset[8], sizeof(resourceNameOffset[8]), nullptr, nullptr, 0);
    int32_t resourceNameRead9 = SFileReadFile(file, &resourceNameOffset[9], sizeof(resourceNameOffset[9]), nullptr, nullptr, 0);
    int32_t resourceNameRead10 = SFileReadFile(file, &resourceNameOffset[10], sizeof(resourceNameOffset[10]), nullptr, nullptr, 0);
    int32_t resourceNameRead11 = SFileReadFile(file, &resourceNameOffset[11], sizeof(resourceNameOffset[11]), nullptr, nullptr, 0);
    int32_t resourceNameRead12 = SFileReadFile(file, &resourceNameOffset[12], sizeof(resourceNameOffset[12]), nullptr, nullptr, 0);
    int32_t resourceNameRead13 = SFileReadFile(file, &resourceNameOffset[13], sizeof(resourceNameOffset[13]), nullptr, nullptr, 0);
    int32_t resourceNameRead14 = SFileReadFile(file, &resourceNameOffset[14], sizeof(resourceNameOffset[14]), nullptr, nullptr, 0);
    int32_t resourceNameRead15 = SFileReadFile(file, &resourceNameOffset[15], sizeof(resourceNameOffset[15]), nullptr, nullptr, 0);
    int32_t resourceNameFlagsRead = SFileReadFile(file, &resourceNameFlags, sizeof(resourceNameFlags), nullptr, nullptr, 0);
    int32_t verbRead0 = SFileReadFile(file, &verbOffset[0], sizeof(verbOffset[0]), nullptr, nullptr, 0);
    int32_t verbRead1 = SFileReadFile(file, &verbOffset[1], sizeof(verbOffset[1]), nullptr, nullptr, 0);
    int32_t verbRead2 = SFileReadFile(file, &verbOffset[2], sizeof(verbOffset[2]), nullptr, nullptr, 0);
    int32_t verbRead3 = SFileReadFile(file, &verbOffset[3], sizeof(verbOffset[3]), nullptr, nullptr, 0);
    int32_t verbRead4 = SFileReadFile(file, &verbOffset[4], sizeof(verbOffset[4]), nullptr, nullptr, 0);
    int32_t verbRead5 = SFileReadFile(file, &verbOffset[5], sizeof(verbOffset[5]), nullptr, nullptr, 0);
    int32_t verbRead6 = SFileReadFile(file, &verbOffset[6], sizeof(verbOffset[6]), nullptr, nullptr, 0);
    int32_t verbRead7 = SFileReadFile(file, &verbOffset[7], sizeof(verbOffset[7]), nullptr, nullptr, 0);
    int32_t verbRead8 = SFileReadFile(file, &verbOffset[8], sizeof(verbOffset[8]), nullptr, nullptr, 0);
    int32_t verbRead9 = SFileReadFile(file, &verbOffset[9], sizeof(verbOffset[9]), nullptr, nullptr, 0);
    int32_t verbRead10 = SFileReadFile(file, &verbOffset[10], sizeof(verbOffset[10]), nullptr, nullptr, 0);
    int32_t verbRead11 = SFileReadFile(file, &verbOffset[11], sizeof(verbOffset[11]), nullptr, nullptr, 0);
    int32_t verbRead12 = SFileReadFile(file, &verbOffset[12], sizeof(verbOffset[12]), nullptr, nullptr, 0);
    int32_t verbRead13 = SFileReadFile(file, &verbOffset[13], sizeof(verbOffset[13]), nullptr, nullptr, 0);
    int32_t verbRead14 = SFileReadFile(file, &verbOffset[14], sizeof(verbOffset[14]), nullptr, nullptr, 0);
    int32_t verbRead15 = SFileReadFile(file, &verbOffset[15], sizeof(verbOffset[15]), nullptr, nullptr, 0);
    int32_t verbFlagsRead = SFileReadFile(file, &verbFlags, sizeof(verbFlags), nullptr, nullptr, 0);
    int32_t cursorNameRead = SFileReadFile(file, &cursorNameOffset, sizeof(cursorNameOffset), nullptr, nullptr, 0);

    if (!idRead
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
        | !resourceNameRead0
        | !resourceNameRead1
        | !resourceNameRead2
        | !resourceNameRead3
        | !resourceNameRead4
        | !resourceNameRead5
        | !resourceNameRead6
        | !resourceNameRead7
        | !resourceNameRead8
        | !resourceNameRead9
        | !resourceNameRead10
        | !resourceNameRead11
        | !resourceNameRead12
        | !resourceNameRead13
        | !resourceNameRead14
        | !resourceNameRead15
        | !resourceNameFlagsRead
        | !verbRead0
        | !verbRead1
        | !verbRead2
        | !verbRead3
        | !verbRead4
        | !verbRead5
        | !verbRead6
        | !verbRead7
        | !verbRead8
        | !verbRead9
        | !verbRead10
        | !verbRead11
        | !verbRead12
        | !verbRead13
        | !verbRead14
        | !verbRead15
        | !verbFlagsRead
        | !cursorNameRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset[s_locale]];
        this->m_resourceName = &stringBuffer[resourceNameOffset[s_locale]];
        this->m_verb = &stringBuffer[verbOffset[s_locale]];
        this->m_cursorName = &stringBuffer[cursorNameOffset];
    } else {
        this->m_name = "";
        this->m_resourceName = "";
        this->m_verb = "";
        this->m_cursorName = "";
    }

    return true;
}
