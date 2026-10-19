#include "WoW/Source/DB/Rec/ScalingStatValuesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ScalingStatValuesRec::ScalingStatValuesRec() {
}

ScalingStatValuesRec::~ScalingStatValuesRec() {
}

// VERIFY: ded484ae auto
// OFFSET: 0x008B5720
const char* ScalingStatValuesRec::GetFilename() {
    return "DBFilesClient\\ScalingStatValues.dbc";
}

// VERIFY: 8b66a250 auto
// OFFSET: 0x008B5730
bool ScalingStatValuesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t charlevelRead = SFileReadFile(file, &this->m_charlevel, sizeof(this->m_charlevel), nullptr, nullptr, 0);
    int32_t shoulderBudgetRead = SFileReadFile(file, &this->m_shoulderBudget, sizeof(this->m_shoulderBudget), nullptr, nullptr, 0);
    int32_t trinketBudgetRead = SFileReadFile(file, &this->m_trinketBudget, sizeof(this->m_trinketBudget), nullptr, nullptr, 0);
    int32_t weaponBudget1hRead = SFileReadFile(file, &this->m_weaponBudget1h, sizeof(this->m_weaponBudget1h), nullptr, nullptr, 0);
    int32_t rangedBudgetRead = SFileReadFile(file, &this->m_rangedBudget, sizeof(this->m_rangedBudget), nullptr, nullptr, 0);
    int32_t clothShoulderArmorRead = SFileReadFile(file, &this->m_clothShoulderArmor, sizeof(this->m_clothShoulderArmor), nullptr, nullptr, 0);
    int32_t leatherShoulderArmorRead = SFileReadFile(file, &this->m_leatherShoulderArmor, sizeof(this->m_leatherShoulderArmor), nullptr, nullptr, 0);
    int32_t mailShoulderArmorRead = SFileReadFile(file, &this->m_mailShoulderArmor, sizeof(this->m_mailShoulderArmor), nullptr, nullptr, 0);
    int32_t plateShoulderArmorRead = SFileReadFile(file, &this->m_plateShoulderArmor, sizeof(this->m_plateShoulderArmor), nullptr, nullptr, 0);
    int32_t weaponDps1hRead = SFileReadFile(file, &this->m_weaponDps1h, sizeof(this->m_weaponDps1h), nullptr, nullptr, 0);
    int32_t weaponDps2hRead = SFileReadFile(file, &this->m_weaponDps2h, sizeof(this->m_weaponDps2h), nullptr, nullptr, 0);
    int32_t spellcasterDps1hRead = SFileReadFile(file, &this->m_spellcasterDps1h, sizeof(this->m_spellcasterDps1h), nullptr, nullptr, 0);
    int32_t spellcasterDps2hRead = SFileReadFile(file, &this->m_spellcasterDps2h, sizeof(this->m_spellcasterDps2h), nullptr, nullptr, 0);
    int32_t rangedDpsRead = SFileReadFile(file, &this->m_rangedDps, sizeof(this->m_rangedDps), nullptr, nullptr, 0);
    int32_t wandDpsRead = SFileReadFile(file, &this->m_wandDps, sizeof(this->m_wandDps), nullptr, nullptr, 0);
    int32_t spellPowerRead = SFileReadFile(file, &this->m_spellPower, sizeof(this->m_spellPower), nullptr, nullptr, 0);
    int32_t primaryBudgetRead = SFileReadFile(file, &this->m_primaryBudget, sizeof(this->m_primaryBudget), nullptr, nullptr, 0);
    int32_t tertiaryBudgetRead = SFileReadFile(file, &this->m_tertiaryBudget, sizeof(this->m_tertiaryBudget), nullptr, nullptr, 0);
    int32_t clothCloakArmorRead = SFileReadFile(file, &this->m_clothCloakArmor, sizeof(this->m_clothCloakArmor), nullptr, nullptr, 0);
    int32_t clothChestArmorRead = SFileReadFile(file, &this->m_clothChestArmor, sizeof(this->m_clothChestArmor), nullptr, nullptr, 0);
    int32_t leatherChestArmorRead = SFileReadFile(file, &this->m_leatherChestArmor, sizeof(this->m_leatherChestArmor), nullptr, nullptr, 0);
    int32_t mailChestArmorRead = SFileReadFile(file, &this->m_mailChestArmor, sizeof(this->m_mailChestArmor), nullptr, nullptr, 0);
    int32_t plateChestArmorRead = SFileReadFile(file, &this->m_plateChestArmor, sizeof(this->m_plateChestArmor), nullptr, nullptr, 0);

    if (!idRead
        | !charlevelRead
        | !shoulderBudgetRead
        | !trinketBudgetRead
        | !weaponBudget1hRead
        | !rangedBudgetRead
        | !clothShoulderArmorRead
        | !leatherShoulderArmorRead
        | !mailShoulderArmorRead
        | !plateShoulderArmorRead
        | !weaponDps1hRead
        | !weaponDps2hRead
        | !spellcasterDps1hRead
        | !spellcasterDps2hRead
        | !rangedDpsRead
        | !wandDpsRead
        | !spellPowerRead
        | !primaryBudgetRead
        | !tertiaryBudgetRead
        | !clothCloakArmorRead
        | !clothChestArmorRead
        | !leatherChestArmorRead
        | !mailChestArmorRead
        | !plateChestArmorRead) {
        return false;
    }

    return true;
}
