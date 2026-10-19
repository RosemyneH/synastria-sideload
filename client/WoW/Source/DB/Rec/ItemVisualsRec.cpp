#include "WoW/Source/DB/Rec/ItemVisualsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ItemVisualsRec::ItemVisualsRec() {
}

ItemVisualsRec::~ItemVisualsRec() {
}

// VERIFY: f20ffeed auto
// OFFSET: 0x008B1940
const char* ItemVisualsRec::GetFilename() {
    return "DBFilesClient\\ItemVisuals.dbc";
}

// VERIFY: 8e950615 auto
// OFFSET: 0x008B1950
bool ItemVisualsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t slotRead = SFileReadFile(file, this->m_slot, sizeof(this->m_slot), nullptr, nullptr, 0);

    if (!idRead
        | !slotRead) {
        return false;
    }

    return true;
}
