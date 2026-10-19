#include "WoW/Source/DB/Rec/DungeonMapChunkRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

DungeonMapChunkRec::DungeonMapChunkRec() {
}

DungeonMapChunkRec::~DungeonMapChunkRec() {
}

// VERIFY: 70f0f040 auto
// OFFSET: 0x008AED20
const char* DungeonMapChunkRec::GetFilename() {
    return "DBFilesClient\\DungeonMapChunk.dbc";
}

bool DungeonMapChunkRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t mapIDRead = SFileReadFile(file, &this->m_mapID, sizeof(this->m_mapID), nullptr, nullptr, 0);
    int32_t wmogroupIDRead = SFileReadFile(file, &this->m_wmogroupID, sizeof(this->m_wmogroupID), nullptr, nullptr, 0);
    int32_t dungeonMapIDRead = SFileReadFile(file, &this->m_dungeonMapID, sizeof(this->m_dungeonMapID), nullptr, nullptr, 0);
    int32_t minZRead = SFileReadFile(file, &this->m_minZ, sizeof(this->m_minZ), nullptr, nullptr, 0);

    if (!idRead
        | !mapIDRead
        | !wmogroupIDRead
        | !dungeonMapIDRead
        | !minZRead) {
        return false;
    }

    return true;
}
