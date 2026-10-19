#include "WoW/Source/DB/Rec/DurabilityQualityRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

DurabilityQualityRec::DurabilityQualityRec() {
}

DurabilityQualityRec::~DurabilityQualityRec() {
}

// VERIFY: e635c809 auto
// OFFSET: 0x008AEDB0
const char* DurabilityQualityRec::GetFilename() {
    return "DBFilesClient\\DurabilityQuality.dbc";
}

bool DurabilityQualityRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!idRead
        | !dataRead) {
        return false;
    }

    return true;
}
