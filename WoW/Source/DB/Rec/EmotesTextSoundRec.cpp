#include "WoW/Source/DB/Rec/EmotesTextSoundRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

EmotesTextSoundRec::EmotesTextSoundRec() {
}

EmotesTextSoundRec::~EmotesTextSoundRec() {
}

// VERIFY: 4ddeeb20 auto
// OFFSET: 0x008AEDE0
const char* EmotesTextSoundRec::GetFilename() {
    return "DBFilesClient\\EmotesTextSound.dbc";
}

bool EmotesTextSoundRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t emotesTextIDRead = SFileReadFile(file, &this->m_emotesTextID, sizeof(this->m_emotesTextID), nullptr, nullptr, 0);
    int32_t raceIDRead = SFileReadFile(file, &this->m_raceID, sizeof(this->m_raceID), nullptr, nullptr, 0);
    int32_t sexIDRead = SFileReadFile(file, &this->m_sexID, sizeof(this->m_sexID), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, &this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);

    if (!idRead
        | !emotesTextIDRead
        | !raceIDRead
        | !sexIDRead
        | !soundIDRead) {
        return false;
    }

    return true;
}
