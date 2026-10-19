#include "WoW/Source/DB/Rec/CharStartOutfitRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CharStartOutfitRec::CharStartOutfitRec() {
}

CharStartOutfitRec::~CharStartOutfitRec() {
}

// VERIFY: ea2a9491 auto
// OFFSET: 0x008AAE10
const char* CharStartOutfitRec::GetFilename() {
    return "DBFilesClient\\CharStartOutfit.dbc";
}

// VERIFY: 72658a34 manual
// OFFSET: 0x008AAE20
bool CharStartOutfitRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t raceIDRead = SFileReadFile(file, &this->m_raceID, sizeof(this->m_raceID), nullptr, nullptr, 0);
    int32_t classIDRead = SFileReadFile(file, &this->m_classID, sizeof(this->m_classID), nullptr, nullptr, 0);
    int32_t sexIDRead = SFileReadFile(file, &this->m_sexID, sizeof(this->m_sexID), nullptr, nullptr, 0);
    int32_t outfitIDRead = SFileReadFile(file, &this->m_outfitID, sizeof(this->m_outfitID), nullptr, nullptr, 0);
    int32_t itemIDRead = SFileReadFile(file, this->m_itemID, sizeof(this->m_itemID), nullptr, nullptr, 0);
    int32_t displayItemIDRead = SFileReadFile(file, this->m_displayItemID, sizeof(this->m_displayItemID), nullptr, nullptr, 0);
    int32_t inventoryTypeRead = SFileReadFile(file, this->m_inventoryType, sizeof(this->m_inventoryType), nullptr, nullptr, 0);

    if (!idRead
        | !raceIDRead
        | !classIDRead
        | !sexIDRead
        | !outfitIDRead
        | !itemIDRead
        | !displayItemIDRead
        | !inventoryTypeRead) {
        return false;
    }

    return true;
}
