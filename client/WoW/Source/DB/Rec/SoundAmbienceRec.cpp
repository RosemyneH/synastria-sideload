#include "WoW/Source/DB/Rec/SoundAmbienceRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SoundAmbienceRec::SoundAmbienceRec() {
}

SoundAmbienceRec::~SoundAmbienceRec() {
}

// VERIFY: 586efdd8 auto
// OFFSET: 0x008B67C0
const char* SoundAmbienceRec::GetFilename() {
    return "DBFilesClient\\SoundAmbience.dbc";
}

// VERIFY: c0acb3df auto
// OFFSET: 0x008B67D0
bool SoundAmbienceRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t ambienceIDRead = SFileReadFile(file, this->m_ambienceID, sizeof(this->m_ambienceID), nullptr, nullptr, 0);

    if (!idRead
        | !ambienceIDRead) {
        return false;
    }

    return true;
}
