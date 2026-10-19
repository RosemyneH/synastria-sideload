#include "WoW/Source/DB/Rec/DeclinedWordCasesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

DeclinedWordCasesRec::DeclinedWordCasesRec() {
}

DeclinedWordCasesRec::~DeclinedWordCasesRec() {
}

// VERIFY: 1a1f1d88 auto
// OFFSET: 0x008AE690
const char* DeclinedWordCasesRec::GetFilename() {
    return "DBFilesClient\\DeclinedWordCases.dbc";
}

// VERIFY: 61b6354a manual
// OFFSET: 0x008AE6A0
bool DeclinedWordCasesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t declinedWordOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t declinedWordIDRead = SFileReadFile(file, &this->m_declinedWordID, sizeof(this->m_declinedWordID), nullptr, nullptr, 0);
    int32_t caseIndexRead = SFileReadFile(file, &this->m_caseIndex, sizeof(this->m_caseIndex), nullptr, nullptr, 0);
    int32_t declinedWordRead = SFileReadFile(file, &declinedWordOffset, sizeof(declinedWordOffset), nullptr, nullptr, 0);

    if (!idRead
        | !declinedWordIDRead
        | !caseIndexRead
        | !declinedWordRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_declinedWord = &stringBuffer[declinedWordOffset];
    } else {
        this->m_declinedWord = "";
    }

    return true;
}
