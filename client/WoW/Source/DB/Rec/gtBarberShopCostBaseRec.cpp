#include "WoW/Source/DB/Rec/gtBarberShopCostBaseRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

gtBarberShopCostBaseRec::gtBarberShopCostBaseRec() {
}

gtBarberShopCostBaseRec::~gtBarberShopCostBaseRec() {
}

// VERIFY: f99fb6f7 auto
// OFFSET: 0x008AFF80
const char* gtBarberShopCostBaseRec::GetFilename() {
    return "DBFilesClient\\gtBarberShopCostBase.dbc";
}

bool gtBarberShopCostBaseRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t dataRead = SFileReadFile(file, &this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!dataRead) {
        return false;
    }

    return true;
}
