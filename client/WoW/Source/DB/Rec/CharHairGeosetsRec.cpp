#include "WoW/Source/DB/Rec/CharHairGeosetsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CharHairGeosetsRec::CharHairGeosetsRec() {
}

CharHairGeosetsRec::~CharHairGeosetsRec() {
}

// VERIFY: f6818543 auto
// OFFSET: 0x008AAE00
const char* CharHairGeosetsRec::GetFilename() {
    return "DBFilesClient\\CharHairGeosets.dbc";
}

// VERIFY: ced97cbd manual
// OFFSET: 0x008B5200
bool CharHairGeosetsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t raceIDRead = SFileReadFile(file, &this->m_raceID, sizeof(this->m_raceID), nullptr, nullptr, 0);
    int32_t sexIDRead = SFileReadFile(file, &this->m_sexID, sizeof(this->m_sexID), nullptr, nullptr, 0);
    int32_t variationIDRead = SFileReadFile(file, &this->m_variationID, sizeof(this->m_variationID), nullptr, nullptr, 0);
    int32_t geosetIDRead = SFileReadFile(file, &this->m_geosetID, sizeof(this->m_geosetID), nullptr, nullptr, 0);
    int32_t showscalpRead = SFileReadFile(file, &this->m_showscalp, sizeof(this->m_showscalp), nullptr, nullptr, 0);

    if (!idRead
        | !raceIDRead
        | !sexIDRead
        | !variationIDRead
        | !geosetIDRead
        | !showscalpRead) {
        return false;
    }

    return true;
}
