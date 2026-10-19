#include "WoW/Source/DB/Rec/gtCombatRatingsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

gtCombatRatingsRec::gtCombatRatingsRec() {
}

gtCombatRatingsRec::~gtCombatRatingsRec() {
}

// VERIFY: 027d8a92 auto
// OFFSET: 0x008AFF90
const char* gtCombatRatingsRec::GetFilename() {
    return "DBFilesClient\\gtCombatRatings.dbc";
}

bool gtCombatRatingsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!dataRead) {
        return false;
    }

    return true;
}
