#include "WoW/Source/DB/Rec/SummonPropertiesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SummonPropertiesRec::SummonPropertiesRec() {
}

SummonPropertiesRec::~SummonPropertiesRec() {
}

// VERIFY: e7470c1d auto
// OFFSET: 0x008B9950
const char* SummonPropertiesRec::GetFilename() {
    return "DBFilesClient\\SummonProperties.dbc";
}

bool SummonPropertiesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t controlRead = SFileReadFile(file, &this->m_control, sizeof(this->m_control), nullptr, nullptr, 0);
    int32_t factionRead = SFileReadFile(file, &this->m_faction, sizeof(this->m_faction), nullptr, nullptr, 0);
    int32_t titleRead = SFileReadFile(file, &this->m_title, sizeof(this->m_title), nullptr, nullptr, 0);
    int32_t slotRead = SFileReadFile(file, &this->m_slot, sizeof(this->m_slot), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);

    if (!idRead
        | !controlRead
        | !factionRead
        | !titleRead
        | !slotRead
        | !flagsRead) {
        return false;
    }

    return true;
}
