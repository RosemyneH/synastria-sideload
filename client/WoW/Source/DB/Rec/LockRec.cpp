#include "WoW/Source/DB/Rec/LockRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

LockRec::LockRec() {
}

LockRec::~LockRec() {
}

// VERIFY: 8487bb5c auto
// OFFSET: 0x008B2CB0
const char* LockRec::GetFilename() {
    return "DBFilesClient\\Lock.dbc";
}

// VERIFY: d57a1635 manual
// OFFSET: 0x008B2CC0
bool LockRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t typeRead = SFileReadFile(file, this->m_type, sizeof(this->m_type), nullptr, nullptr, 0);
    int32_t indexRead = SFileReadFile(file, this->m_index, sizeof(this->m_index), nullptr, nullptr, 0);
    int32_t skillRead = SFileReadFile(file, this->m_skill, sizeof(this->m_skill), nullptr, nullptr, 0);
    int32_t actionRead = SFileReadFile(file, this->m_action, sizeof(this->m_action), nullptr, nullptr, 0);

    if (!idRead
        | !typeRead
        | !indexRead
        | !skillRead
        | !actionRead) {
        return false;
    }

    return true;
}
