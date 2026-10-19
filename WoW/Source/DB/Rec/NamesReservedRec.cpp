#include "WoW/Source/DB/Rec/NamesReservedRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

NamesReservedRec::NamesReservedRec() {
}

NamesReservedRec::~NamesReservedRec() {
}

// VERIFY: f5e3e3c2 auto
// OFFSET: 0x008BCC00
const char* NamesReservedRec::GetFilename() {
    return "DBFilesClient\\NamesReserved.dbc";
}

bool NamesReservedRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t languageRead = SFileReadFile(file, &this->m_language, sizeof(this->m_language), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead
        | !languageRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
    } else {
        this->m_name = "";
    }

    return true;
}
