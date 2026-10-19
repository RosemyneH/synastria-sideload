#include "WoW/Source/DB/Rec/gtRegenMPPerSptRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

gtRegenMPPerSptRec::gtRegenMPPerSptRec() {
}

gtRegenMPPerSptRec::~gtRegenMPPerSptRec() {
}

// VERIFY: f9483f77 auto
// OFFSET: 0x008B0050
const char* gtRegenMPPerSptRec::GetFilename() {
    return "DBFilesClient\\gtRegenMPPerSpt.dbc";
}

bool gtRegenMPPerSptRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!dataRead) {
        return false;
    }

    return true;
}
