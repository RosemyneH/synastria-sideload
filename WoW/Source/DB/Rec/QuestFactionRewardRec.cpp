#include "WoW/Source/DB/Rec/QuestFactionRewardRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

QuestFactionRewardRec::QuestFactionRewardRec() {
}

QuestFactionRewardRec::~QuestFactionRewardRec() {
}

// VERIFY: 7c550fa2 auto
// OFFSET: 0x008B52D0
const char* QuestFactionRewardRec::GetFilename() {
    return "DBFilesClient\\QuestFactionReward.dbc";
}

bool QuestFactionRewardRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t difficultyRead = SFileReadFile(file, this->m_difficulty, sizeof(this->m_difficulty), nullptr, nullptr, 0);

    if (!idRead
        | !difficultyRead) {
        return false;
    }

    return true;
}
