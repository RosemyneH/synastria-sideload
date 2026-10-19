#include "WoW/Source/DB/Rec/ChatProfanityRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ChatProfanityRec::ChatProfanityRec() {
}

ChatProfanityRec::~ChatProfanityRec() {
}

// VERIFY: a7be01b0 auto
// OFFSET: 0x008ABA90
const char* ChatProfanityRec::GetFilename() {
    return "DBFilesClient\\ChatProfanity.dbc";
}

// VERIFY: d01fb327 auto
// OFFSET: 0x008BCB60
bool ChatProfanityRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t textOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t textRead = SFileReadFile(file, &textOffset, sizeof(textOffset), nullptr, nullptr, 0);
    int32_t languageRead = SFileReadFile(file, &this->m_language, sizeof(this->m_language), nullptr, nullptr, 0);

    if (!idRead
        | !textRead
        | !languageRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_text = &stringBuffer[textOffset];
    } else {
        this->m_text = "";
    }

    return true;
}
