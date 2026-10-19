#include "WoW/Source/DB/Rec/SpellMissileRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellMissileRec::SpellMissileRec() {
}

SpellMissileRec::~SpellMissileRec() {
}

// VERIFY: 72fad47e auto
// OFFSET: 0x008B8300
const char* SpellMissileRec::GetFilename() {
    return "DBFilesClient\\SpellMissile.dbc";
}

// VERIFY: d6152bc1 auto
// OFFSET: 0x008B8310
bool SpellMissileRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t defaultPitchMinRead = SFileReadFile(file, &this->m_defaultPitchMin, sizeof(this->m_defaultPitchMin), nullptr, nullptr, 0);
    int32_t defaultPitchMaxRead = SFileReadFile(file, &this->m_defaultPitchMax, sizeof(this->m_defaultPitchMax), nullptr, nullptr, 0);
    int32_t defaultSpeedMinRead = SFileReadFile(file, &this->m_defaultSpeedMin, sizeof(this->m_defaultSpeedMin), nullptr, nullptr, 0);
    int32_t defaultSpeedMaxRead = SFileReadFile(file, &this->m_defaultSpeedMax, sizeof(this->m_defaultSpeedMax), nullptr, nullptr, 0);
    int32_t randomizeFacingMinRead = SFileReadFile(file, &this->m_randomizeFacingMin, sizeof(this->m_randomizeFacingMin), nullptr, nullptr, 0);
    int32_t randomizeFacingMaxRead = SFileReadFile(file, &this->m_randomizeFacingMax, sizeof(this->m_randomizeFacingMax), nullptr, nullptr, 0);
    int32_t randomizePitchMinRead = SFileReadFile(file, &this->m_randomizePitchMin, sizeof(this->m_randomizePitchMin), nullptr, nullptr, 0);
    int32_t randomizePitchMaxRead = SFileReadFile(file, &this->m_randomizePitchMax, sizeof(this->m_randomizePitchMax), nullptr, nullptr, 0);
    int32_t randomizeSpeedMinRead = SFileReadFile(file, &this->m_randomizeSpeedMin, sizeof(this->m_randomizeSpeedMin), nullptr, nullptr, 0);
    int32_t randomizeSpeedMaxRead = SFileReadFile(file, &this->m_randomizeSpeedMax, sizeof(this->m_randomizeSpeedMax), nullptr, nullptr, 0);
    int32_t gravityRead = SFileReadFile(file, &this->m_gravity, sizeof(this->m_gravity), nullptr, nullptr, 0);
    int32_t maxDurationRead = SFileReadFile(file, &this->m_maxDuration, sizeof(this->m_maxDuration), nullptr, nullptr, 0);
    int32_t collisionRadiusRead = SFileReadFile(file, &this->m_collisionRadius, sizeof(this->m_collisionRadius), nullptr, nullptr, 0);

    if (!idRead
        | !flagsRead
        | !defaultPitchMinRead
        | !defaultPitchMaxRead
        | !defaultSpeedMinRead
        | !defaultSpeedMaxRead
        | !randomizeFacingMinRead
        | !randomizeFacingMaxRead
        | !randomizePitchMinRead
        | !randomizePitchMaxRead
        | !randomizeSpeedMinRead
        | !randomizeSpeedMaxRead
        | !gravityRead
        | !maxDurationRead
        | !collisionRadiusRead) {
        return false;
    }

    return true;
}
