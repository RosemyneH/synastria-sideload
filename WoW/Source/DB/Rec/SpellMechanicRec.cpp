#include "WoW/Source/DB/Rec/SpellMechanicRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

SpellMechanicRec::SpellMechanicRec() {
}

SpellMechanicRec::~SpellMechanicRec() {
}

// VERIFY: 065e6e06 auto
// OFFSET: 0x008B82F0
const char* SpellMechanicRec::GetFilename() {
    return "DBFilesClient\\SpellMechanic.dbc";
}

bool SpellMechanicRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t stateNameOffset[16];
    uint32_t stateNameFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t stateNameRead0 = SFileReadFile(file, &stateNameOffset[0], sizeof(stateNameOffset[0]), nullptr, nullptr, 0);
    int32_t stateNameRead1 = SFileReadFile(file, &stateNameOffset[1], sizeof(stateNameOffset[1]), nullptr, nullptr, 0);
    int32_t stateNameRead2 = SFileReadFile(file, &stateNameOffset[2], sizeof(stateNameOffset[2]), nullptr, nullptr, 0);
    int32_t stateNameRead3 = SFileReadFile(file, &stateNameOffset[3], sizeof(stateNameOffset[3]), nullptr, nullptr, 0);
    int32_t stateNameRead4 = SFileReadFile(file, &stateNameOffset[4], sizeof(stateNameOffset[4]), nullptr, nullptr, 0);
    int32_t stateNameRead5 = SFileReadFile(file, &stateNameOffset[5], sizeof(stateNameOffset[5]), nullptr, nullptr, 0);
    int32_t stateNameRead6 = SFileReadFile(file, &stateNameOffset[6], sizeof(stateNameOffset[6]), nullptr, nullptr, 0);
    int32_t stateNameRead7 = SFileReadFile(file, &stateNameOffset[7], sizeof(stateNameOffset[7]), nullptr, nullptr, 0);
    int32_t stateNameRead8 = SFileReadFile(file, &stateNameOffset[8], sizeof(stateNameOffset[8]), nullptr, nullptr, 0);
    int32_t stateNameRead9 = SFileReadFile(file, &stateNameOffset[9], sizeof(stateNameOffset[9]), nullptr, nullptr, 0);
    int32_t stateNameRead10 = SFileReadFile(file, &stateNameOffset[10], sizeof(stateNameOffset[10]), nullptr, nullptr, 0);
    int32_t stateNameRead11 = SFileReadFile(file, &stateNameOffset[11], sizeof(stateNameOffset[11]), nullptr, nullptr, 0);
    int32_t stateNameRead12 = SFileReadFile(file, &stateNameOffset[12], sizeof(stateNameOffset[12]), nullptr, nullptr, 0);
    int32_t stateNameRead13 = SFileReadFile(file, &stateNameOffset[13], sizeof(stateNameOffset[13]), nullptr, nullptr, 0);
    int32_t stateNameRead14 = SFileReadFile(file, &stateNameOffset[14], sizeof(stateNameOffset[14]), nullptr, nullptr, 0);
    int32_t stateNameRead15 = SFileReadFile(file, &stateNameOffset[15], sizeof(stateNameOffset[15]), nullptr, nullptr, 0);
    int32_t stateNameFlagsRead = SFileReadFile(file, &stateNameFlags, sizeof(stateNameFlags), nullptr, nullptr, 0);

    if (!idRead
        | !stateNameRead0
        | !stateNameRead1
        | !stateNameRead2
        | !stateNameRead3
        | !stateNameRead4
        | !stateNameRead5
        | !stateNameRead6
        | !stateNameRead7
        | !stateNameRead8
        | !stateNameRead9
        | !stateNameRead10
        | !stateNameRead11
        | !stateNameRead12
        | !stateNameRead13
        | !stateNameRead14
        | !stateNameRead15
        | !stateNameFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_stateName = &stringBuffer[stateNameOffset[s_locale]];
    } else {
        this->m_stateName = "";
    }

    return true;
}
