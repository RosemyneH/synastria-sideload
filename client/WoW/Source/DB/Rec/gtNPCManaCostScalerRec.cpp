#include "WoW/Source/DB/Rec/gtNPCManaCostScalerRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

gtNPCManaCostScalerRec::gtNPCManaCostScalerRec() {
}

gtNPCManaCostScalerRec::~gtNPCManaCostScalerRec() {
}

// VERIFY: a51e1e9c auto
// OFFSET: 0x008B0000
const char* gtNPCManaCostScalerRec::GetFilename() {
    return "DBFilesClient\\gtNPCManaCostScaler.dbc";
}

bool gtNPCManaCostScalerRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!dataRead) {
        return false;
    }

    return true;
}
