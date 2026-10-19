#include "WoW/Source/DB/Rec/gtOCTClassCombatRatingScalarRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

gtOCTClassCombatRatingScalarRec::gtOCTClassCombatRatingScalarRec() {
}

gtOCTClassCombatRatingScalarRec::~gtOCTClassCombatRatingScalarRec() {
}

// VERIFY: 5ff0a7d1 auto
// OFFSET: 0x008B0010
const char* gtOCTClassCombatRatingScalarRec::GetFilename() {
    return "DBFilesClient\\gtOCTClassCombatRatingScalar.dbc";
}

bool gtOCTClassCombatRatingScalarRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!idRead
        | !dataRead) {
        return false;
    }

    return true;
}
