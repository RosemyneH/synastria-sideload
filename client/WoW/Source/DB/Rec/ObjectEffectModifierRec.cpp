#include "WoW/Source/DB/Rec/ObjectEffectModifierRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ObjectEffectModifierRec::ObjectEffectModifierRec() {
}

ObjectEffectModifierRec::~ObjectEffectModifierRec() {
}

// VERIFY: 191fc968 auto
// OFFSET: 0x008BD700
const char* ObjectEffectModifierRec::GetFilename() {
    return "DBFilesClient\\ObjectEffectModifier.dbc";
}

// VERIFY: 0b5f1b1d manual
// OFFSET: 0x008BD710
bool ObjectEffectModifierRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t inputTypeRead = SFileReadFile(file, &this->m_inputType, sizeof(this->m_inputType), nullptr, nullptr, 0);
    int32_t mapTypeRead = SFileReadFile(file, &this->m_mapType, sizeof(this->m_mapType), nullptr, nullptr, 0);
    int32_t outputTypeRead = SFileReadFile(file, &this->m_outputType, sizeof(this->m_outputType), nullptr, nullptr, 0);
    int32_t paramRead = SFileReadFile(file, this->m_param, sizeof(this->m_param), nullptr, nullptr, 0);

    if (!idRead
        | !inputTypeRead
        | !mapTypeRead
        | !outputTypeRead
        | !paramRead) {
        return false;
    }

    return true;
}
