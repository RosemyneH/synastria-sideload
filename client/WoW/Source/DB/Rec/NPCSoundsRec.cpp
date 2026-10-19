#include "WoW/Source/DB/Rec/NPCSoundsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

NPCSoundsRec::NPCSoundsRec() {
}

NPCSoundsRec::~NPCSoundsRec() {
}

// VERIFY: 061c330c auto
// OFFSET: 0x008B48E0
const char* NPCSoundsRec::GetFilename() {
    return "DBFilesClient\\NPCSounds.dbc";
}

bool NPCSoundsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);

    if (!idRead
        | !soundIDRead) {
        return false;
    }

    return true;
}
