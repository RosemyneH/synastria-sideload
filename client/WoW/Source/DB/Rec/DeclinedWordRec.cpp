#include "WoW/Source/DB/Rec/DeclinedWordRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

DeclinedWordRec::DeclinedWordRec() {
}

DeclinedWordRec::~DeclinedWordRec() {
}

// VERIFY: d8469c5c auto
// OFFSET: 0x008AE680
const char* DeclinedWordRec::GetFilename() {
    return "DBFilesClient\\DeclinedWord.dbc";
}

bool DeclinedWordRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t wordOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t wordRead = SFileReadFile(file, &wordOffset, sizeof(wordOffset), nullptr, nullptr, 0);

    if (!idRead
        | !wordRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_word = &stringBuffer[wordOffset];
    } else {
        this->m_word = "";
    }

    return true;
}
