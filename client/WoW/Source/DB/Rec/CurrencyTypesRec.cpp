#include "WoW/Source/DB/Rec/CurrencyTypesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CurrencyTypesRec::CurrencyTypesRec() {
}

CurrencyTypesRec::~CurrencyTypesRec() {
}

// VERIFY: d8e8249d auto
// OFFSET: 0x008AE2E0
const char* CurrencyTypesRec::GetFilename() {
    return "DBFilesClient\\CurrencyTypes.dbc";
}

bool CurrencyTypesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t itemIDRead = SFileReadFile(file, &this->m_itemID, sizeof(this->m_itemID), nullptr, nullptr, 0);
    int32_t categoryIDRead = SFileReadFile(file, &this->m_categoryID, sizeof(this->m_categoryID), nullptr, nullptr, 0);
    int32_t bitIndexRead = SFileReadFile(file, &this->m_bitIndex, sizeof(this->m_bitIndex), nullptr, nullptr, 0);

    if (!idRead
        | !itemIDRead
        | !categoryIDRead
        | !bitIndexRead) {
        return false;
    }

    return true;
}
