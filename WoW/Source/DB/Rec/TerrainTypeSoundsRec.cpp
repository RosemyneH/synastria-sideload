#include "WoW/Source/DB/Rec/TerrainTypeSoundsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

TerrainTypeSoundsRec::TerrainTypeSoundsRec() {
}

TerrainTypeSoundsRec::~TerrainTypeSoundsRec() {
}

// VERIFY: e4884ba2 auto
// OFFSET: 0x008BA2B0
const char* TerrainTypeSoundsRec::GetFilename() {
    return "DBFilesClient\\TerrainTypeSounds.dbc";
}

// VERIFY: 3523b9e7 auto
// OFFSET: 0x008AFFD0
bool TerrainTypeSoundsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);

    if (!idRead) {
        return false;
    }

    return true;
}
