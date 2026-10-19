#include "WoW/Source/DB/Rec/AnimationDataRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

AnimationDataRec::AnimationDataRec() {
}

AnimationDataRec::~AnimationDataRec() {
}

// VERIFY: f23fb774 auto
// OFFSET: 0x008A93D0
const char* AnimationDataRec::GetFilename() {
    return "DBFilesClient\\AnimationData.dbc";
}

// VERIFY: 2ead5348 manual
// OFFSET: 0x008A93E0
bool AnimationDataRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t weaponflagsRead = SFileReadFile(file, &this->m_weaponflags, sizeof(this->m_weaponflags), nullptr, nullptr, 0);
    int32_t bodyflagsRead = SFileReadFile(file, &this->m_bodyflags, sizeof(this->m_bodyflags), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t fallbackRead = SFileReadFile(file, &this->m_fallback, sizeof(this->m_fallback), nullptr, nullptr, 0);
    int32_t behaviorIDRead = SFileReadFile(file, &this->m_behaviorID, sizeof(this->m_behaviorID), nullptr, nullptr, 0);
    int32_t behaviorTierRead = SFileReadFile(file, &this->m_behaviorTier, sizeof(this->m_behaviorTier), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead
        | !weaponflagsRead
        | !bodyflagsRead
        | !flagsRead
        | !fallbackRead
        | !behaviorIDRead
        | !behaviorTierRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
    } else {
        this->m_name = "";
    }

    return true;
}
