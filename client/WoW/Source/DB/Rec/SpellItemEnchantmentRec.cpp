#include "WoW/Source/DB/Rec/SpellItemEnchantmentRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

SpellItemEnchantmentRec::SpellItemEnchantmentRec() {
}

SpellItemEnchantmentRec::~SpellItemEnchantmentRec() {
}

// VERIFY: 1078e6d6 auto
// OFFSET: 0x008B7DD0
const char* SpellItemEnchantmentRec::GetFilename() {
    return "DBFilesClient\\SpellItemEnchantment.dbc";
}

// VERIFY: cd45cc42 auto
// OFFSET: 0x008B7DE0
bool SpellItemEnchantmentRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset[16];
    uint32_t nameFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t chargesRead = SFileReadFile(file, &this->m_charges, sizeof(this->m_charges), nullptr, nullptr, 0);
    int32_t effectRead = SFileReadFile(file, this->m_effect, sizeof(this->m_effect), nullptr, nullptr, 0);
    int32_t effectPointsMinRead = SFileReadFile(file, this->m_effectPointsMin, sizeof(this->m_effectPointsMin), nullptr, nullptr, 0);
    int32_t effectPointsMaxRead = SFileReadFile(file, this->m_effectPointsMax, sizeof(this->m_effectPointsMax), nullptr, nullptr, 0);
    int32_t effectArgRead = SFileReadFile(file, this->m_effectArg, sizeof(this->m_effectArg), nullptr, nullptr, 0);
    int32_t nameRead0 = SFileReadFile(file, &nameOffset[0], sizeof(nameOffset[0]), nullptr, nullptr, 0);
    int32_t nameRead1 = SFileReadFile(file, &nameOffset[1], sizeof(nameOffset[1]), nullptr, nullptr, 0);
    int32_t nameRead2 = SFileReadFile(file, &nameOffset[2], sizeof(nameOffset[2]), nullptr, nullptr, 0);
    int32_t nameRead3 = SFileReadFile(file, &nameOffset[3], sizeof(nameOffset[3]), nullptr, nullptr, 0);
    int32_t nameRead4 = SFileReadFile(file, &nameOffset[4], sizeof(nameOffset[4]), nullptr, nullptr, 0);
    int32_t nameRead5 = SFileReadFile(file, &nameOffset[5], sizeof(nameOffset[5]), nullptr, nullptr, 0);
    int32_t nameRead6 = SFileReadFile(file, &nameOffset[6], sizeof(nameOffset[6]), nullptr, nullptr, 0);
    int32_t nameRead7 = SFileReadFile(file, &nameOffset[7], sizeof(nameOffset[7]), nullptr, nullptr, 0);
    int32_t nameRead8 = SFileReadFile(file, &nameOffset[8], sizeof(nameOffset[8]), nullptr, nullptr, 0);
    int32_t nameRead9 = SFileReadFile(file, &nameOffset[9], sizeof(nameOffset[9]), nullptr, nullptr, 0);
    int32_t nameRead10 = SFileReadFile(file, &nameOffset[10], sizeof(nameOffset[10]), nullptr, nullptr, 0);
    int32_t nameRead11 = SFileReadFile(file, &nameOffset[11], sizeof(nameOffset[11]), nullptr, nullptr, 0);
    int32_t nameRead12 = SFileReadFile(file, &nameOffset[12], sizeof(nameOffset[12]), nullptr, nullptr, 0);
    int32_t nameRead13 = SFileReadFile(file, &nameOffset[13], sizeof(nameOffset[13]), nullptr, nullptr, 0);
    int32_t nameRead14 = SFileReadFile(file, &nameOffset[14], sizeof(nameOffset[14]), nullptr, nullptr, 0);
    int32_t nameRead15 = SFileReadFile(file, &nameOffset[15], sizeof(nameOffset[15]), nullptr, nullptr, 0);
    int32_t nameFlagsRead = SFileReadFile(file, &nameFlags, sizeof(nameFlags), nullptr, nullptr, 0);
    int32_t itemVisualRead = SFileReadFile(file, &this->m_itemVisual, sizeof(this->m_itemVisual), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t srcItemIDRead = SFileReadFile(file, &this->m_srcItemID, sizeof(this->m_srcItemID), nullptr, nullptr, 0);
    int32_t conditionIDRead = SFileReadFile(file, &this->m_conditionID, sizeof(this->m_conditionID), nullptr, nullptr, 0);
    int32_t requiredSkillIDRead = SFileReadFile(file, &this->m_requiredSkillID, sizeof(this->m_requiredSkillID), nullptr, nullptr, 0);
    int32_t requiredSkillRankRead = SFileReadFile(file, &this->m_requiredSkillRank, sizeof(this->m_requiredSkillRank), nullptr, nullptr, 0);
    int32_t minLevelRead = SFileReadFile(file, &this->m_minLevel, sizeof(this->m_minLevel), nullptr, nullptr, 0);

    if (!idRead
        | !chargesRead
        | !effectRead
        | !effectPointsMinRead
        | !effectPointsMaxRead
        | !effectArgRead
        | !nameRead0
        | !nameRead1
        | !nameRead2
        | !nameRead3
        | !nameRead4
        | !nameRead5
        | !nameRead6
        | !nameRead7
        | !nameRead8
        | !nameRead9
        | !nameRead10
        | !nameRead11
        | !nameRead12
        | !nameRead13
        | !nameRead14
        | !nameRead15
        | !nameFlagsRead
        | !itemVisualRead
        | !flagsRead
        | !srcItemIDRead
        | !conditionIDRead
        | !requiredSkillIDRead
        | !requiredSkillRankRead
        | !minLevelRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset[s_locale]];
    } else {
        this->m_name = "";
    }

    return true;
}
