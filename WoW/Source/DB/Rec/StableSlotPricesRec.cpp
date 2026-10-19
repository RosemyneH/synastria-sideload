#include "WoW/Source/DB/Rec/StableSlotPricesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

StableSlotPricesRec::StableSlotPricesRec() {
}

StableSlotPricesRec::~StableSlotPricesRec() {
}

// VERIFY: 07c61246 auto
// OFFSET: 0x008B97E0
const char* StableSlotPricesRec::GetFilename() {
    return "DBFilesClient\\StableSlotPrices.dbc";
}

bool StableSlotPricesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t costRead = SFileReadFile(file, &this->m_cost, sizeof(this->m_cost), nullptr, nullptr, 0);

    if (!idRead
        | !costRead) {
        return false;
    }

    return true;
}
