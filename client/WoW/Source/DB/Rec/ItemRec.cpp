#include "WoW/Source/DB/Rec/ItemRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ItemRec::ItemRec() {
}

ItemRec::~ItemRec() {
}

// VERIFY: e84cbade auto
// OFFSET: 0x008B02C0
const char* ItemRec::GetFilename() {
    return "DBFilesClient\\Item.dbc";
}

bool ItemRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t classIDRead = SFileReadFile(file, &this->m_classID, sizeof(this->m_classID), nullptr, nullptr, 0);
    int32_t subclassIDRead = SFileReadFile(file, &this->m_subclassID, sizeof(this->m_subclassID), nullptr, nullptr, 0);
    int32_t soundOverrideSubclassIDRead = SFileReadFile(file, &this->m_soundOverrideSubclassID, sizeof(this->m_soundOverrideSubclassID), nullptr, nullptr, 0);
    int32_t materialRead = SFileReadFile(file, &this->m_material, sizeof(this->m_material), nullptr, nullptr, 0);
    int32_t displayInfoIDRead = SFileReadFile(file, &this->m_displayInfoID, sizeof(this->m_displayInfoID), nullptr, nullptr, 0);
    int32_t inventoryTypeRead = SFileReadFile(file, &this->m_inventoryType, sizeof(this->m_inventoryType), nullptr, nullptr, 0);
    int32_t sheatheTypeRead = SFileReadFile(file, &this->m_sheatheType, sizeof(this->m_sheatheType), nullptr, nullptr, 0);

    if (!idRead
        | !classIDRead
        | !subclassIDRead
        | !soundOverrideSubclassIDRead
        | !materialRead
        | !displayInfoIDRead
        | !inventoryTypeRead
        | !sheatheTypeRead) {
        return false;
    }

    return true;
}
