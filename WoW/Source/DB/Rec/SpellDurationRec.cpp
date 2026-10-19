#include "WoW/Source/DB/Rec/SpellDurationRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellDurationRec::SpellDurationRec() {
}

SpellDurationRec::~SpellDurationRec() {
}

// VERIFY: 953ff3e4 auto
// OFFSET: 0x008B7D80
const char* SpellDurationRec::GetFilename() {
    return "DBFilesClient\\SpellDuration.dbc";
}

bool SpellDurationRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t durationRead = SFileReadFile(file, &this->m_duration, sizeof(this->m_duration), nullptr, nullptr, 0);
    int32_t durationPerLevelRead = SFileReadFile(file, &this->m_durationPerLevel, sizeof(this->m_durationPerLevel), nullptr, nullptr, 0);
    int32_t maxDurationRead = SFileReadFile(file, &this->m_maxDuration, sizeof(this->m_maxDuration), nullptr, nullptr, 0);

    if (!idRead
        | !durationRead
        | !durationPerLevelRead
        | !maxDurationRead) {
        return false;
    }

    return true;
}
