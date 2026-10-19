#include "WoW/Source/DB/Rec/PvpDifficultyRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

PvpDifficultyRec::PvpDifficultyRec() {
}

PvpDifficultyRec::~PvpDifficultyRec() {
}

// VERIFY: c1e53e13 auto
// OFFSET: 0x008B51F0
const char* PvpDifficultyRec::GetFilename() {
    return "DBFilesClient\\PvpDifficulty.dbc";
}

bool PvpDifficultyRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t mapIDRead = SFileReadFile(file, &this->m_mapID, sizeof(this->m_mapID), nullptr, nullptr, 0);
    int32_t rangeIndexRead = SFileReadFile(file, &this->m_rangeIndex, sizeof(this->m_rangeIndex), nullptr, nullptr, 0);
    int32_t minLevelRead = SFileReadFile(file, &this->m_minLevel, sizeof(this->m_minLevel), nullptr, nullptr, 0);
    int32_t maxLevelRead = SFileReadFile(file, &this->m_maxLevel, sizeof(this->m_maxLevel), nullptr, nullptr, 0);
    int32_t difficultyRead = SFileReadFile(file, &this->m_difficulty, sizeof(this->m_difficulty), nullptr, nullptr, 0);

    if (!idRead
        | !mapIDRead
        | !rangeIndexRead
        | !minLevelRead
        | !maxLevelRead
        | !difficultyRead) {
        return false;
    }

    return true;
}
