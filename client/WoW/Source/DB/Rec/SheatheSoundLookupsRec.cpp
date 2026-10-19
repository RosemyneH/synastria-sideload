#include "WoW/Source/DB/Rec/SheatheSoundLookupsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SheatheSoundLookupsRec::SheatheSoundLookupsRec() {
}

SheatheSoundLookupsRec::~SheatheSoundLookupsRec() {
}

// VERIFY: 9271078b auto
// OFFSET: 0x008B5B70
const char* SheatheSoundLookupsRec::GetFilename() {
    return "DBFilesClient\\SheatheSoundLookups.dbc";
}

// VERIFY: f3af1a8b manual
// OFFSET: 0x008B5B80
bool SheatheSoundLookupsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t classIDRead = SFileReadFile(file, &this->m_classID, sizeof(this->m_classID), nullptr, nullptr, 0);
    int32_t subclassIDRead = SFileReadFile(file, &this->m_subclassID, sizeof(this->m_subclassID), nullptr, nullptr, 0);
    int32_t materialRead = SFileReadFile(file, &this->m_material, sizeof(this->m_material), nullptr, nullptr, 0);
    int32_t checkMaterialRead = SFileReadFile(file, &this->m_checkMaterial, sizeof(this->m_checkMaterial), nullptr, nullptr, 0);
    int32_t sheatheSoundRead = SFileReadFile(file, &this->m_sheatheSound, sizeof(this->m_sheatheSound), nullptr, nullptr, 0);
    int32_t unsheatheSoundRead = SFileReadFile(file, &this->m_unsheatheSound, sizeof(this->m_unsheatheSound), nullptr, nullptr, 0);

    if (!idRead
        | !classIDRead
        | !subclassIDRead
        | !materialRead
        | !checkMaterialRead
        | !sheatheSoundRead
        | !unsheatheSoundRead) {
        return false;
    }

    return true;
}
