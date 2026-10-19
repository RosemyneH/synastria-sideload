#include "WoW/Source/DB/Rec/SoundWaterTypeRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SoundWaterTypeRec::SoundWaterTypeRec() {
}

SoundWaterTypeRec::~SoundWaterTypeRec() {
}

// VERIFY: 2e16a9ad auto
// OFFSET: 0x008B7280
const char* SoundWaterTypeRec::GetFilename() {
    return "DBFilesClient\\SoundWaterType.dbc";
}

bool SoundWaterTypeRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t soundTypeRead = SFileReadFile(file, &this->m_soundType, sizeof(this->m_soundType), nullptr, nullptr, 0);
    int32_t soundSubtypeRead = SFileReadFile(file, &this->m_soundSubtype, sizeof(this->m_soundSubtype), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, &this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);

    if (!idRead
        | !soundTypeRead
        | !soundSubtypeRead
        | !soundIDRead) {
        return false;
    }

    return true;
}
