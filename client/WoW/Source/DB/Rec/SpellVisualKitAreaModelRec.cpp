#include "WoW/Source/DB/Rec/SpellVisualKitAreaModelRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellVisualKitAreaModelRec::SpellVisualKitAreaModelRec() {
}

SpellVisualKitAreaModelRec::~SpellVisualKitAreaModelRec() {
}

// VERIFY: 29e2b380 auto
// OFFSET: 0x008B93C0
const char* SpellVisualKitAreaModelRec::GetFilename() {
    return "DBFilesClient\\SpellVisualKitAreaModel.dbc";
}

bool SpellVisualKitAreaModelRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t enumIDRead = SFileReadFile(file, &this->m_enumID, sizeof(this->m_enumID), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead
        | !enumIDRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
    } else {
        this->m_name = "";
    }

    return true;
}
