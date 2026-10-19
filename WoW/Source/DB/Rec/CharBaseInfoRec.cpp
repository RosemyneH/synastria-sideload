#include "WoW/Source/DB/Rec/CharBaseInfoRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CharBaseInfoRec::CharBaseInfoRec() {
}

CharBaseInfoRec::~CharBaseInfoRec() {
}

// VERIFY: 40b133d7 auto
// OFFSET: 0x008AADA0
const char* CharBaseInfoRec::GetFilename() {
    return "DBFilesClient\\CharBaseInfo.dbc";
}

// VERIFY: 0ddc5b20 auto
// OFFSET: 0x008AADB0
bool CharBaseInfoRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t raceIDRead = SFileReadFile(file, &this->m_raceID, sizeof(this->m_raceID), nullptr, nullptr, 0);
    int32_t classIDRead = SFileReadFile(file, &this->m_classID, sizeof(this->m_classID), nullptr, nullptr, 0);

    if (!raceIDRead
        | !classIDRead) {
        return false;
    }

    return true;
}
