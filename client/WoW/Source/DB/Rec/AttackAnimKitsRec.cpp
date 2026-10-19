#include "WoW/Source/DB/Rec/AttackAnimKitsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

AttackAnimKitsRec::AttackAnimKitsRec() {
}

AttackAnimKitsRec::~AttackAnimKitsRec() {
}

// VERIFY: 8fad52a4 auto
// OFFSET: 0x008AA0F0
const char* AttackAnimKitsRec::GetFilename() {
    return "DBFilesClient\\AttackAnimKits.dbc";
}

// VERIFY: 72abedab manual
// OFFSET: 0x008B8BA0
bool AttackAnimKitsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t itemSubclassIDRead = SFileReadFile(file, &this->m_itemSubclassID, sizeof(this->m_itemSubclassID), nullptr, nullptr, 0);
    int32_t animTypeIDRead = SFileReadFile(file, &this->m_animTypeID, sizeof(this->m_animTypeID), nullptr, nullptr, 0);
    int32_t animFrequencyRead = SFileReadFile(file, &this->m_animFrequency, sizeof(this->m_animFrequency), nullptr, nullptr, 0);
    int32_t whichHandRead = SFileReadFile(file, &this->m_whichHand, sizeof(this->m_whichHand), nullptr, nullptr, 0);

    if (!idRead
        | !itemSubclassIDRead
        | !animTypeIDRead
        | !animFrequencyRead
        | !whichHandRead) {
        return false;
    }

    return true;
}
