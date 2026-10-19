#include "WoW/Source/DB/Rec/SpamMessagesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpamMessagesRec::SpamMessagesRec() {
}

SpamMessagesRec::~SpamMessagesRec() {
}

// VERIFY: 63561ae2 auto
// OFFSET: 0x008B7290
const char* SpamMessagesRec::GetFilename() {
    return "DBFilesClient\\SpamMessages.dbc";
}

bool SpamMessagesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t textOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t textRead = SFileReadFile(file, &textOffset, sizeof(textOffset), nullptr, nullptr, 0);

    if (!idRead
        | !textRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_text = &stringBuffer[textOffset];
    } else {
        this->m_text = "";
    }

    return true;
}
