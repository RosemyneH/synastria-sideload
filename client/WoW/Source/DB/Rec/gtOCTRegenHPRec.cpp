#include "WoW/Source/DB/Rec/gtOCTRegenHPRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

gtOCTRegenHPRec::gtOCTRegenHPRec() {
}

gtOCTRegenHPRec::~gtOCTRegenHPRec() {
}

// VERIFY: 9f30403d auto
// OFFSET: 0x008B0020
const char* gtOCTRegenHPRec::GetFilename() {
    return "DBFilesClient\\gtOCTRegenHP.dbc";
}

bool gtOCTRegenHPRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!dataRead) {
        return false;
    }

    return true;
}
