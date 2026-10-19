#include "WoW/Source/DB/Rec/NamesProfanityRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

NamesProfanityRec::NamesProfanityRec() {
}

NamesProfanityRec::~NamesProfanityRec() {
}

// VERIFY: ebf8a040 auto
// OFFSET: 0x008BCB50
const char* NamesProfanityRec::GetFilename() {
    return "DBFilesClient\\NamesProfanity.dbc";
}

bool NamesProfanityRec::Read(SFileImpl* file, const char* stringBuffer) {
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
