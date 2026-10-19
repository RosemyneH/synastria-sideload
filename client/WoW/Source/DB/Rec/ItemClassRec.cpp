#include "WoW/Source/DB/Rec/ItemClassRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

ItemClassRec::ItemClassRec() {
}

ItemClassRec::~ItemClassRec() {
}

// VERIFY: 3b5cdbbf auto
// OFFSET: 0x008B02E0
const char* ItemClassRec::GetFilename() {
    return "DBFilesClient\\ItemClass.dbc";
}

bool ItemClassRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t classNameOffset[16];
    uint32_t classNameFlags;

    int32_t classIDRead = SFileReadFile(file, &this->m_classID, sizeof(this->m_classID), nullptr, nullptr, 0);
    int32_t subclassMapIDRead = SFileReadFile(file, &this->m_subclassMapID, sizeof(this->m_subclassMapID), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t classNameRead0 = SFileReadFile(file, &classNameOffset[0], sizeof(classNameOffset[0]), nullptr, nullptr, 0);
    int32_t classNameRead1 = SFileReadFile(file, &classNameOffset[1], sizeof(classNameOffset[1]), nullptr, nullptr, 0);
    int32_t classNameRead2 = SFileReadFile(file, &classNameOffset[2], sizeof(classNameOffset[2]), nullptr, nullptr, 0);
    int32_t classNameRead3 = SFileReadFile(file, &classNameOffset[3], sizeof(classNameOffset[3]), nullptr, nullptr, 0);
    int32_t classNameRead4 = SFileReadFile(file, &classNameOffset[4], sizeof(classNameOffset[4]), nullptr, nullptr, 0);
    int32_t classNameRead5 = SFileReadFile(file, &classNameOffset[5], sizeof(classNameOffset[5]), nullptr, nullptr, 0);
    int32_t classNameRead6 = SFileReadFile(file, &classNameOffset[6], sizeof(classNameOffset[6]), nullptr, nullptr, 0);
    int32_t classNameRead7 = SFileReadFile(file, &classNameOffset[7], sizeof(classNameOffset[7]), nullptr, nullptr, 0);
    int32_t classNameRead8 = SFileReadFile(file, &classNameOffset[8], sizeof(classNameOffset[8]), nullptr, nullptr, 0);
    int32_t classNameRead9 = SFileReadFile(file, &classNameOffset[9], sizeof(classNameOffset[9]), nullptr, nullptr, 0);
    int32_t classNameRead10 = SFileReadFile(file, &classNameOffset[10], sizeof(classNameOffset[10]), nullptr, nullptr, 0);
    int32_t classNameRead11 = SFileReadFile(file, &classNameOffset[11], sizeof(classNameOffset[11]), nullptr, nullptr, 0);
    int32_t classNameRead12 = SFileReadFile(file, &classNameOffset[12], sizeof(classNameOffset[12]), nullptr, nullptr, 0);
    int32_t classNameRead13 = SFileReadFile(file, &classNameOffset[13], sizeof(classNameOffset[13]), nullptr, nullptr, 0);
    int32_t classNameRead14 = SFileReadFile(file, &classNameOffset[14], sizeof(classNameOffset[14]), nullptr, nullptr, 0);
    int32_t classNameRead15 = SFileReadFile(file, &classNameOffset[15], sizeof(classNameOffset[15]), nullptr, nullptr, 0);
    int32_t classNameFlagsRead = SFileReadFile(file, &classNameFlags, sizeof(classNameFlags), nullptr, nullptr, 0);

    if (!classIDRead
        | !subclassMapIDRead
        | !flagsRead
        | !classNameRead0
        | !classNameRead1
        | !classNameRead2
        | !classNameRead3
        | !classNameRead4
        | !classNameRead5
        | !classNameRead6
        | !classNameRead7
        | !classNameRead8
        | !classNameRead9
        | !classNameRead10
        | !classNameRead11
        | !classNameRead12
        | !classNameRead13
        | !classNameRead14
        | !classNameRead15
        | !classNameFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_className = &stringBuffer[classNameOffset[s_locale]];
    } else {
        this->m_className = "";
    }

    return true;
}
