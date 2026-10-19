#include "WoW/Source/DB/Rec/MaterialRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

MaterialRec::MaterialRec() {
}

MaterialRec::~MaterialRec() {
}

// VERIFY: 75774c25 auto
// OFFSET: 0x008B47D0
const char* MaterialRec::GetFilename() {
    return "DBFilesClient\\Material.dbc";
}

bool MaterialRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t foleySoundIDRead = SFileReadFile(file, &this->m_foleySoundID, sizeof(this->m_foleySoundID), nullptr, nullptr, 0);
    int32_t sheatheSoundIDRead = SFileReadFile(file, &this->m_sheatheSoundID, sizeof(this->m_sheatheSoundID), nullptr, nullptr, 0);
    int32_t unsheatheSoundIDRead = SFileReadFile(file, &this->m_unsheatheSoundID, sizeof(this->m_unsheatheSoundID), nullptr, nullptr, 0);

    if (!idRead
        | !flagsRead
        | !foleySoundIDRead
        | !sheatheSoundIDRead
        | !unsheatheSoundIDRead) {
        return false;
    }

    return true;
}
