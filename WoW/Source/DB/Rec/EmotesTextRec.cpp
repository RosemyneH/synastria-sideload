#include "WoW/Source/DB/Rec/EmotesTextRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

EmotesTextRec::EmotesTextRec() {
}

EmotesTextRec::~EmotesTextRec() {
}

// VERIFY: 81d856a5 auto
// OFFSET: 0x008AEDF0
const char* EmotesTextRec::GetFilename() {
    return "DBFilesClient\\EmotesText.dbc";
}

// VERIFY: 893cc732 manual
// OFFSET: 0x008AEE00
bool EmotesTextRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t emoteIDRead = SFileReadFile(file, &this->m_emoteID, sizeof(this->m_emoteID), nullptr, nullptr, 0);
    int32_t emoteTextRead = SFileReadFile(file, this->m_emoteText, sizeof(this->m_emoteText), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead
        | !emoteIDRead
        | !emoteTextRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
    } else {
        this->m_name = "";
    }

    return true;
}
