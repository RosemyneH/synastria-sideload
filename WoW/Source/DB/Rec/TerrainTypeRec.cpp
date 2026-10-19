#include "WoW/Source/DB/Rec/TerrainTypeRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

TerrainTypeRec::TerrainTypeRec() {
}

TerrainTypeRec::~TerrainTypeRec() {
}

// VERIFY: 0f543dc1 auto
// OFFSET: 0x008BA1A0
const char* TerrainTypeRec::GetFilename() {
    return "DBFilesClient\\TerrainType.dbc";
}

// VERIFY: c0318443 manual
// OFFSET: 0x008BA1B0
bool TerrainTypeRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t terrainDescOffset;

    int32_t terrainIDRead = SFileReadFile(file, &this->m_terrainID, sizeof(this->m_terrainID), nullptr, nullptr, 0);
    int32_t terrainDescRead = SFileReadFile(file, &terrainDescOffset, sizeof(terrainDescOffset), nullptr, nullptr, 0);
    int32_t footstepSprayRunRead = SFileReadFile(file, &this->m_footstepSprayRun, sizeof(this->m_footstepSprayRun), nullptr, nullptr, 0);
    int32_t footstepSprayWalkRead = SFileReadFile(file, &this->m_footstepSprayWalk, sizeof(this->m_footstepSprayWalk), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, &this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);

    if (!terrainIDRead
        | !terrainDescRead
        | !footstepSprayRunRead
        | !footstepSprayWalkRead
        | !soundIDRead
        | !flagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_terrainDesc = &stringBuffer[terrainDescOffset];
    } else {
        this->m_terrainDesc = "";
    }

    return true;
}
