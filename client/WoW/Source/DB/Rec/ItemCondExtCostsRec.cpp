#include "WoW/Source/DB/Rec/ItemCondExtCostsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ItemCondExtCostsRec::ItemCondExtCostsRec() {
}

ItemCondExtCostsRec::~ItemCondExtCostsRec() {
}

// VERIFY: 7b64adfa auto
// OFFSET: 0x008B02F0
const char* ItemCondExtCostsRec::GetFilename() {
    return "DBFilesClient\\ItemCondExtCosts.dbc";
}

bool ItemCondExtCostsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t condExtendedCostRead = SFileReadFile(file, &this->m_condExtendedCost, sizeof(this->m_condExtendedCost), nullptr, nullptr, 0);
    int32_t itemExtendedCostEntryRead = SFileReadFile(file, &this->m_itemExtendedCostEntry, sizeof(this->m_itemExtendedCostEntry), nullptr, nullptr, 0);
    int32_t arenaSeasonRead = SFileReadFile(file, &this->m_arenaSeason, sizeof(this->m_arenaSeason), nullptr, nullptr, 0);

    if (!idRead
        | !condExtendedCostRead
        | !itemExtendedCostEntryRead
        | !arenaSeasonRead) {
        return false;
    }

    return true;
}
