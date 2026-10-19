#include "WoW/Source/DB/Rec/SoundFilterRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SoundFilterRec::SoundFilterRec() {
}

SoundFilterRec::~SoundFilterRec() {
}

// VERIFY: 2f1584ef auto
// OFFSET: 0x008BD7E0
const char* SoundFilterRec::GetFilename() {
    return "DBFilesClient\\SoundFilter.dbc";
}

bool SoundFilterRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
    } else {
        this->m_name = "";
    }

    return true;
}
