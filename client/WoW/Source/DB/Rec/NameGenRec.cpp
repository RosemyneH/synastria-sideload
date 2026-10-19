#include "WoW/Source/DB/Rec/NameGenRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

NameGenRec::NameGenRec() {
}

NameGenRec::~NameGenRec() {
}

// VERIFY: fb9f5723 auto
// OFFSET: 0x008B4810
const char* NameGenRec::GetFilename() {
    return "DBFilesClient\\NameGen.dbc";
}

// VERIFY: 2ec8bb08 manual
// OFFSET: 0x008B4820
bool NameGenRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t raceIDRead = SFileReadFile(file, &this->m_raceID, sizeof(this->m_raceID), nullptr, nullptr, 0);
    int32_t sexRead = SFileReadFile(file, &this->m_sex, sizeof(this->m_sex), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead
        | !raceIDRead
        | !sexRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
    } else {
        this->m_name = "";
    }

    return true;
}
