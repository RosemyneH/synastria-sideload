#include "WoW/Source/DB/Rec/QuestXPRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

QuestXPRec::QuestXPRec() {
}

QuestXPRec::~QuestXPRec() {
}

// VERIFY: 711ec0fc auto
// OFFSET: 0x008B5300
const char* QuestXPRec::GetFilename() {
    return "DBFilesClient\\QuestXP.dbc";
}

bool QuestXPRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t difficultyRead = SFileReadFile(file, this->m_difficulty, sizeof(this->m_difficulty), nullptr, nullptr, 0);

    if (!idRead
        | !difficultyRead) {
        return false;
    }

    return true;
}
