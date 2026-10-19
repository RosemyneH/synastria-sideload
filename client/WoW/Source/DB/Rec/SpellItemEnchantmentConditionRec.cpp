#include "WoW/Source/DB/Rec/SpellItemEnchantmentConditionRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellItemEnchantmentConditionRec::SpellItemEnchantmentConditionRec() {
}

SpellItemEnchantmentConditionRec::~SpellItemEnchantmentConditionRec() {
}

// VERIFY: b4e0c08d auto
// OFFSET: 0x008B81F0
const char* SpellItemEnchantmentConditionRec::GetFilename() {
    return "DBFilesClient\\SpellItemEnchantmentCondition.dbc";
}

// VERIFY: 24cb0b75 manual
// OFFSET: 0x008B8200
bool SpellItemEnchantmentConditionRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t ltOperandTypeRead = SFileReadFile(file, this->m_ltOperandType, sizeof(this->m_ltOperandType), nullptr, nullptr, 0);
    int32_t ltOperandRead = SFileReadFile(file, this->m_ltOperand, sizeof(this->m_ltOperand), nullptr, nullptr, 0);
    int32_t operatorRead = SFileReadFile(file, this->m_operator, sizeof(this->m_operator), nullptr, nullptr, 0);
    int32_t rtOperandTypeRead = SFileReadFile(file, this->m_rtOperandType, sizeof(this->m_rtOperandType), nullptr, nullptr, 0);
    int32_t rtOperandRead = SFileReadFile(file, this->m_rtOperand, sizeof(this->m_rtOperand), nullptr, nullptr, 0);
    int32_t logicRead = SFileReadFile(file, this->m_logic, sizeof(this->m_logic), nullptr, nullptr, 0);

    if (!idRead
        | !ltOperandTypeRead
        | !ltOperandRead
        | !operatorRead
        | !rtOperandTypeRead
        | !rtOperandRead
        | !logicRead) {
        return false;
    }

    return true;
}
