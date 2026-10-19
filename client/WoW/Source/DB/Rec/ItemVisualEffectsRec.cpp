#include "WoW/Source/DB/Rec/ItemVisualEffectsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ItemVisualEffectsRec::ItemVisualEffectsRec() {
}

ItemVisualEffectsRec::~ItemVisualEffectsRec() {
}

// VERIFY: dd60d961 auto
// OFFSET: 0x008B1930
const char* ItemVisualEffectsRec::GetFilename() {
    return "DBFilesClient\\ItemVisualEffects.dbc";
}

bool ItemVisualEffectsRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t modelOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t modelRead = SFileReadFile(file, &modelOffset, sizeof(modelOffset), nullptr, nullptr, 0);

    if (!idRead
        | !modelRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_model = &stringBuffer[modelOffset];
    } else {
        this->m_model = "";
    }

    return true;
}
