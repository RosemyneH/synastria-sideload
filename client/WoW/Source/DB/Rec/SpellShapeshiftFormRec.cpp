#include "WoW/Source/DB/Rec/SpellShapeshiftFormRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

SpellShapeshiftFormRec::SpellShapeshiftFormRec() {
}

SpellShapeshiftFormRec::~SpellShapeshiftFormRec() {
}

// VERIFY: 7b442026 auto
// OFFSET: 0x008B8C50
const char* SpellShapeshiftFormRec::GetFilename() {
    return "DBFilesClient\\SpellShapeshiftForm.dbc";
}

// VERIFY: 709c268e auto
// OFFSET: 0x008B8C60
bool SpellShapeshiftFormRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset[16];
    uint32_t nameFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t bonusActionBarRead = SFileReadFile(file, &this->m_bonusActionBar, sizeof(this->m_bonusActionBar), nullptr, nullptr, 0);
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
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t creatureTypeRead = SFileReadFile(file, &this->m_creatureType, sizeof(this->m_creatureType), nullptr, nullptr, 0);
    int32_t attackIconIDRead = SFileReadFile(file, &this->m_attackIconID, sizeof(this->m_attackIconID), nullptr, nullptr, 0);
    int32_t combatRoundTimeRead = SFileReadFile(file, &this->m_combatRoundTime, sizeof(this->m_combatRoundTime), nullptr, nullptr, 0);
    int32_t creatureDisplayIDRead = SFileReadFile(file, this->m_creatureDisplayID, sizeof(this->m_creatureDisplayID), nullptr, nullptr, 0);
    int32_t presetSpellIDRead = SFileReadFile(file, this->m_presetSpellID, sizeof(this->m_presetSpellID), nullptr, nullptr, 0);

    if (!idRead
        | !bonusActionBarRead
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
        | !flagsRead
        | !creatureTypeRead
        | !attackIconIDRead
        | !combatRoundTimeRead
        | !creatureDisplayIDRead
        | !presetSpellIDRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset[s_locale]];
    } else {
        this->m_name = "";
    }

    return true;
}
