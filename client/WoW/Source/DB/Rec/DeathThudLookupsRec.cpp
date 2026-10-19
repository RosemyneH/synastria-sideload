#include "WoW/Source/DB/Rec/DeathThudLookupsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

DeathThudLookupsRec::DeathThudLookupsRec() {
}

DeathThudLookupsRec::~DeathThudLookupsRec() {
}

// VERIFY: 9abd5533 auto
// OFFSET: 0x008AE670
const char* DeathThudLookupsRec::GetFilename() {
    return "DBFilesClient\\DeathThudLookups.dbc";
}

bool DeathThudLookupsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t sizeClassRead = SFileReadFile(file, &this->m_sizeClass, sizeof(this->m_sizeClass), nullptr, nullptr, 0);
    int32_t terrainTypeSoundIDRead = SFileReadFile(file, &this->m_terrainTypeSoundID, sizeof(this->m_terrainTypeSoundID), nullptr, nullptr, 0);
    int32_t soundEntryIDRead = SFileReadFile(file, &this->m_soundEntryID, sizeof(this->m_soundEntryID), nullptr, nullptr, 0);
    int32_t soundEntryIdwaterRead = SFileReadFile(file, &this->m_soundEntryIdwater, sizeof(this->m_soundEntryIdwater), nullptr, nullptr, 0);

    if (!idRead
        | !sizeClassRead
        | !terrainTypeSoundIDRead
        | !soundEntryIDRead
        | !soundEntryIdwaterRead) {
        return false;
    }

    return true;
}
