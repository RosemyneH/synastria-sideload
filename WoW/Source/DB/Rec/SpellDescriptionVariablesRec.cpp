#include "WoW/Source/DB/Rec/SpellDescriptionVariablesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellDescriptionVariablesRec::SpellDescriptionVariablesRec() {
}

SpellDescriptionVariablesRec::~SpellDescriptionVariablesRec() {
}

// VERIFY: 4635fe8b auto
// OFFSET: 0x008B7A60
const char* SpellDescriptionVariablesRec::GetFilename() {
    return "DBFilesClient\\SpellDescriptionVariables.dbc";
}

bool SpellDescriptionVariablesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t variablesOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t variablesRead = SFileReadFile(file, &variablesOffset, sizeof(variablesOffset), nullptr, nullptr, 0);

    if (!idRead
        | !variablesRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_variables = &stringBuffer[variablesOffset];
    } else {
        this->m_variables = "";
    }

    return true;
}
