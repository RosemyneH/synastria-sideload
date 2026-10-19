#include "WoW/Source/DB/Rec/BankBagSlotPricesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

BankBagSlotPricesRec::BankBagSlotPricesRec() {
}

BankBagSlotPricesRec::~BankBagSlotPricesRec() {
}

// VERIFY: 6052eb86 auto
// OFFSET: 0x008AA120
const char* BankBagSlotPricesRec::GetFilename() {
    return "DBFilesClient\\BankBagSlotPrices.dbc";
}

// VERIFY: b107146c auto
// OFFSET: 0x008B7350
bool BankBagSlotPricesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t costRead = SFileReadFile(file, &this->m_cost, sizeof(this->m_cost), nullptr, nullptr, 0);

    if (!idRead
        | !costRead) {
        return false;
    }

    return true;
}
