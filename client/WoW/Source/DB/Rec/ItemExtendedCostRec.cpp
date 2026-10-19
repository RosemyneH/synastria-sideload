#include "WoW/Source/DB/Rec/ItemExtendedCostRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ItemExtendedCostRec::ItemExtendedCostRec() {
}

ItemExtendedCostRec::~ItemExtendedCostRec() {
}

// VERIFY: a3d3f578 auto
// OFFSET: 0x008B0300
const char* ItemExtendedCostRec::GetFilename() {
    return "DBFilesClient\\ItemExtendedCost.dbc";
}

// VERIFY: 3a2ac30c manual
// OFFSET: 0x008B0310
bool ItemExtendedCostRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t honorPointsRead = SFileReadFile(file, &this->m_honorPoints, sizeof(this->m_honorPoints), nullptr, nullptr, 0);
    int32_t arenaPointsRead = SFileReadFile(file, &this->m_arenaPoints, sizeof(this->m_arenaPoints), nullptr, nullptr, 0);
    int32_t arenaBracketRead = SFileReadFile(file, &this->m_arenaBracket, sizeof(this->m_arenaBracket), nullptr, nullptr, 0);
    int32_t itemIDRead = SFileReadFile(file, this->m_itemID, sizeof(this->m_itemID), nullptr, nullptr, 0);
    int32_t itemCountRead = SFileReadFile(file, this->m_itemCount, sizeof(this->m_itemCount), nullptr, nullptr, 0);
    int32_t requiredArenaRatingRead = SFileReadFile(file, &this->m_requiredArenaRating, sizeof(this->m_requiredArenaRating), nullptr, nullptr, 0);
    int32_t itemPurchaseGroupRead = SFileReadFile(file, &this->m_itemPurchaseGroup, sizeof(this->m_itemPurchaseGroup), nullptr, nullptr, 0);

    if (!idRead
        | !honorPointsRead
        | !arenaPointsRead
        | !arenaBracketRead
        | !itemIDRead
        | !itemCountRead
        | !requiredArenaRatingRead
        | !itemPurchaseGroupRead) {
        return false;
    }

    return true;
}
