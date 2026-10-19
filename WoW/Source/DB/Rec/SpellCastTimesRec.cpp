#include "WoW/Source/DB/Rec/SpellCastTimesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellCastTimesRec::SpellCastTimesRec() {
}

SpellCastTimesRec::~SpellCastTimesRec() {
}

// VERIFY: 6a927c5b auto
// OFFSET: 0x008B72A0
const char* SpellCastTimesRec::GetFilename() {
    return "DBFilesClient\\SpellCastTimes.dbc";
}

bool SpellCastTimesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t baseRead = SFileReadFile(file, &this->m_base, sizeof(this->m_base), nullptr, nullptr, 0);
    int32_t perLevelRead = SFileReadFile(file, &this->m_perLevel, sizeof(this->m_perLevel), nullptr, nullptr, 0);
    int32_t minimumRead = SFileReadFile(file, &this->m_minimum, sizeof(this->m_minimum), nullptr, nullptr, 0);

    if (!idRead
        | !baseRead
        | !perLevelRead
        | !minimumRead) {
        return false;
    }

    return true;
}
