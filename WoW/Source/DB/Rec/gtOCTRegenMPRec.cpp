#include "WoW/Source/DB/Rec/gtOCTRegenMPRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

gtOCTRegenMPRec::gtOCTRegenMPRec() {
}

gtOCTRegenMPRec::~gtOCTRegenMPRec() {
}

// VERIFY: 16ec7509 auto
// OFFSET: 0x008B0030
const char* gtOCTRegenMPRec::GetFilename() {
    return "DBFilesClient\\gtOCTRegenMP.dbc";
}

bool gtOCTRegenMPRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!dataRead) {
        return false;
    }

    return true;
}
