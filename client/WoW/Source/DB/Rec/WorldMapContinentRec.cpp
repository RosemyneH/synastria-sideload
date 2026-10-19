#include "WoW/Source/DB/Rec/WorldMapContinentRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

WorldMapContinentRec::WorldMapContinentRec() {
}

WorldMapContinentRec::~WorldMapContinentRec() {
}

// VERIFY: c7e85a4c auto
// OFFSET: 0x008BB9B0
const char* WorldMapContinentRec::GetFilename() {
    return "DBFilesClient\\WorldMapContinent.dbc";
}

// VERIFY: b20e2f6c auto
// OFFSET: 0x008BB9C0
bool WorldMapContinentRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t mapIDRead = SFileReadFile(file, &this->m_mapID, sizeof(this->m_mapID), nullptr, nullptr, 0);
    int32_t leftBoundaryRead = SFileReadFile(file, &this->m_leftBoundary, sizeof(this->m_leftBoundary), nullptr, nullptr, 0);
    int32_t rightBoundaryRead = SFileReadFile(file, &this->m_rightBoundary, sizeof(this->m_rightBoundary), nullptr, nullptr, 0);
    int32_t topBoundaryRead = SFileReadFile(file, &this->m_topBoundary, sizeof(this->m_topBoundary), nullptr, nullptr, 0);
    int32_t bottomBoundaryRead = SFileReadFile(file, &this->m_bottomBoundary, sizeof(this->m_bottomBoundary), nullptr, nullptr, 0);
    int32_t continentRead0 = SFileReadFile(file, &this->m_continentOffset[0], sizeof(this->m_continentOffset[0]), nullptr, nullptr, 0);
    int32_t continentRead1 = SFileReadFile(file, &this->m_continentOffset[1], sizeof(this->m_continentOffset[1]), nullptr, nullptr, 0);
    int32_t scaleRead = SFileReadFile(file, &this->m_scale, sizeof(this->m_scale), nullptr, nullptr, 0);
    int32_t taxiMinRead0 = SFileReadFile(file, &this->m_taxiMin[0], sizeof(this->m_taxiMin[0]), nullptr, nullptr, 0);
    int32_t taxiMinRead1 = SFileReadFile(file, &this->m_taxiMin[1], sizeof(this->m_taxiMin[1]), nullptr, nullptr, 0);
    int32_t taxiMaxRead0 = SFileReadFile(file, &this->m_taxiMax[0], sizeof(this->m_taxiMax[0]), nullptr, nullptr, 0);
    int32_t taxiMaxRead1 = SFileReadFile(file, &this->m_taxiMax[1], sizeof(this->m_taxiMax[1]), nullptr, nullptr, 0);
    int32_t worldMapIDRead = SFileReadFile(file, &this->m_worldMapID, sizeof(this->m_worldMapID), nullptr, nullptr, 0);

    if (!idRead
        | !mapIDRead
        | !leftBoundaryRead
        | !rightBoundaryRead
        | !topBoundaryRead
        | !bottomBoundaryRead
        | !continentRead0
        | !continentRead1
        | !scaleRead
        | !taxiMinRead0
        | !taxiMinRead1
        | !taxiMaxRead0
        | !taxiMaxRead1
        | !worldMapIDRead) {
        return false;
    }

    return true;
}
