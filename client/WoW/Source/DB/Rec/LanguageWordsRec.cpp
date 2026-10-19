#include "WoW/Source/DB/Rec/LanguageWordsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

LanguageWordsRec::LanguageWordsRec() {
}

LanguageWordsRec::~LanguageWordsRec() {
}

// VERIFY: a6737d50 auto
// OFFSET: 0x008B19A0
const char* LanguageWordsRec::GetFilename() {
    return "DBFilesClient\\LanguageWords.dbc";
}

// VERIFY: 676b08f4 auto
// OFFSET: 0x008B19B0
bool LanguageWordsRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t wordOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t languageIDRead = SFileReadFile(file, &this->m_languageID, sizeof(this->m_languageID), nullptr, nullptr, 0);
    int32_t wordRead = SFileReadFile(file, &wordOffset, sizeof(wordOffset), nullptr, nullptr, 0);

    if (!idRead
        | !languageIDRead
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
