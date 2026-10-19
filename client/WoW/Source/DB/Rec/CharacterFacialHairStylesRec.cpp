#include "WoW/Source/DB/Rec/CharacterFacialHairStylesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CharacterFacialHairStylesRec::CharacterFacialHairStylesRec() {
}

CharacterFacialHairStylesRec::~CharacterFacialHairStylesRec() {
}

// VERIFY: e6d77eb4 auto
// OFFSET: 0x008AB490
const char* CharacterFacialHairStylesRec::GetFilename() {
    return "DBFilesClient\\CharacterFacialHairStyles.dbc";
}

// VERIFY: 4e494991 manual
// OFFSET: 0x008AB4A0
bool CharacterFacialHairStylesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t raceIDRead = SFileReadFile(file, &this->m_raceID, sizeof(this->m_raceID), nullptr, nullptr, 0);
    int32_t sexIDRead = SFileReadFile(file, &this->m_sexID, sizeof(this->m_sexID), nullptr, nullptr, 0);
    int32_t variationIDRead = SFileReadFile(file, &this->m_variationID, sizeof(this->m_variationID), nullptr, nullptr, 0);
    int32_t geosetRead = SFileReadFile(file, this->m_geoset, sizeof(this->m_geoset), nullptr, nullptr, 0);

    if (!raceIDRead
        | !sexIDRead
        | !variationIDRead
        | !geosetRead) {
        return false;
    }

    return true;
}
