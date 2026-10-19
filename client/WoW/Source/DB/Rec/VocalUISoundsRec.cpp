#include "WoW/Source/DB/Rec/VocalUISoundsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

VocalUISoundsRec::VocalUISoundsRec() {
}

VocalUISoundsRec::~VocalUISoundsRec() {
}

// VERIFY: 7b858b7b auto
// OFFSET: 0x008BB560
const char* VocalUISoundsRec::GetFilename() {
    return "DBFilesClient\\VocalUISounds.dbc";
}

// VERIFY: bb1de7f2 manual
// OFFSET: 0x008BB570
bool VocalUISoundsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t vocalUienumRead = SFileReadFile(file, &this->m_vocalUienum, sizeof(this->m_vocalUienum), nullptr, nullptr, 0);
    int32_t raceIDRead = SFileReadFile(file, &this->m_raceID, sizeof(this->m_raceID), nullptr, nullptr, 0);
    int32_t normalSoundIDRead = SFileReadFile(file, this->m_normalSoundID, sizeof(this->m_normalSoundID), nullptr, nullptr, 0);
    int32_t pissedSoundIDRead = SFileReadFile(file, this->m_pissedSoundID, sizeof(this->m_pissedSoundID), nullptr, nullptr, 0);

    if (!idRead
        | !vocalUienumRead
        | !raceIDRead
        | !normalSoundIDRead
        | !pissedSoundIDRead) {
        return false;
    }

    return true;
}
