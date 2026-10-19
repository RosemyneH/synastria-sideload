#include "WoW/Source/DB/Rec/SpellDifficultyRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellDifficultyRec::SpellDifficultyRec() {
}

SpellDifficultyRec::~SpellDifficultyRec() {
}

// VERIFY: c498320d auto
// OFFSET: 0x008B7A70
const char* SpellDifficultyRec::GetFilename() {
    return "DBFilesClient\\SpellDifficulty.dbc";
}

bool SpellDifficultyRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t difficultySpellIDRead = SFileReadFile(file, this->m_difficultySpellID, sizeof(this->m_difficultySpellID), nullptr, nullptr, 0);

    if (!idRead
        | !difficultySpellIDRead) {
        return false;
    }

    return true;
}
