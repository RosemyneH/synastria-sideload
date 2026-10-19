#include "WoW/Source/DB/Rec/SpellVisualKitRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellVisualKitRec::SpellVisualKitRec() {
}

SpellVisualKitRec::~SpellVisualKitRec() {
}

// VERIFY: ce41987d auto
// OFFSET: 0x008B9110
const char* SpellVisualKitRec::GetFilename() {
    return "DBFilesClient\\SpellVisualKit.dbc";
}

// VERIFY: 78045f5c auto
// OFFSET: 0x008B9120
bool SpellVisualKitRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t startAnimIDRead = SFileReadFile(file, &this->m_startAnimID, sizeof(this->m_startAnimID), nullptr, nullptr, 0);
    int32_t animIDRead = SFileReadFile(file, &this->m_animID, sizeof(this->m_animID), nullptr, nullptr, 0);
    int32_t headEffectRead = SFileReadFile(file, &this->m_headEffect, sizeof(this->m_headEffect), nullptr, nullptr, 0);
    int32_t chestEffectRead = SFileReadFile(file, &this->m_chestEffect, sizeof(this->m_chestEffect), nullptr, nullptr, 0);
    int32_t baseEffectRead = SFileReadFile(file, &this->m_baseEffect, sizeof(this->m_baseEffect), nullptr, nullptr, 0);
    int32_t leftHandEffectRead = SFileReadFile(file, &this->m_leftHandEffect, sizeof(this->m_leftHandEffect), nullptr, nullptr, 0);
    int32_t rightHandEffectRead = SFileReadFile(file, &this->m_rightHandEffect, sizeof(this->m_rightHandEffect), nullptr, nullptr, 0);
    int32_t breathEffectRead = SFileReadFile(file, &this->m_breathEffect, sizeof(this->m_breathEffect), nullptr, nullptr, 0);
    int32_t leftWeaponEffectRead = SFileReadFile(file, &this->m_leftWeaponEffect, sizeof(this->m_leftWeaponEffect), nullptr, nullptr, 0);
    int32_t rightWeaponEffectRead = SFileReadFile(file, &this->m_rightWeaponEffect, sizeof(this->m_rightWeaponEffect), nullptr, nullptr, 0);
    int32_t specialEffectRead = SFileReadFile(file, this->m_specialEffect, sizeof(this->m_specialEffect), nullptr, nullptr, 0);
    int32_t worldEffectRead = SFileReadFile(file, &this->m_worldEffect, sizeof(this->m_worldEffect), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, &this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);
    int32_t shakeIDRead = SFileReadFile(file, &this->m_shakeID, sizeof(this->m_shakeID), nullptr, nullptr, 0);
    int32_t charProcRead = SFileReadFile(file, this->m_charProc, sizeof(this->m_charProc), nullptr, nullptr, 0);
    int32_t charParamZeroRead = SFileReadFile(file, this->m_charParamZero, sizeof(this->m_charParamZero), nullptr, nullptr, 0);
    int32_t charParamOneRead = SFileReadFile(file, this->m_charParamOne, sizeof(this->m_charParamOne), nullptr, nullptr, 0);
    int32_t charParamTwoRead = SFileReadFile(file, this->m_charParamTwo, sizeof(this->m_charParamTwo), nullptr, nullptr, 0);
    int32_t charParamThreeRead = SFileReadFile(file, this->m_charParamThree, sizeof(this->m_charParamThree), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);

    if (!idRead
        | !startAnimIDRead
        | !animIDRead
        | !headEffectRead
        | !chestEffectRead
        | !baseEffectRead
        | !leftHandEffectRead
        | !rightHandEffectRead
        | !breathEffectRead
        | !leftWeaponEffectRead
        | !rightWeaponEffectRead
        | !specialEffectRead
        | !worldEffectRead
        | !soundIDRead
        | !shakeIDRead
        | !charProcRead
        | !charParamZeroRead
        | !charParamOneRead
        | !charParamTwoRead
        | !charParamThreeRead
        | !flagsRead) {
        return false;
    }

    return true;
}
