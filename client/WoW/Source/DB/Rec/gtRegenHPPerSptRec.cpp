#include "WoW/Source/DB/Rec/gtRegenHPPerSptRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

gtRegenHPPerSptRec::gtRegenHPPerSptRec() {
}

gtRegenHPPerSptRec::~gtRegenHPPerSptRec() {
}

// VERIFY: 5b46113a auto
// OFFSET: 0x008B0040
const char* gtRegenHPPerSptRec::GetFilename() {
    return "DBFilesClient\\gtRegenHPPerSpt.dbc";
}

bool gtRegenHPPerSptRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!dataRead) {
        return false;
    }

    return true;
}
