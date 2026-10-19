#include "WoW/Source/DB/Rec/ItemGroupSoundsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ItemGroupSoundsRec::ItemGroupSoundsRec() {
}

ItemGroupSoundsRec::~ItemGroupSoundsRec() {
}

// VERIFY: 1707929a auto
// OFFSET: 0x008B0420
const char* ItemGroupSoundsRec::GetFilename() {
    return "DBFilesClient\\ItemGroupSounds.dbc";
}

// VERIFY: 6a580fc0 auto
// OFFSET: 0x008B48F0
bool ItemGroupSoundsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t soundRead = SFileReadFile(file, this->m_sound, sizeof(this->m_sound), nullptr, nullptr, 0);

    if (!idRead
        | !soundRead) {
        return false;
    }

    return true;
}
