#include "WoW/Source/DB/Rec/WorldMapAreaRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

WorldMapAreaRec::WorldMapAreaRec() {
}

WorldMapAreaRec::~WorldMapAreaRec() {
}

// VERIFY: a38996c2 auto
// OFFSET: 0x008BB800
const char* WorldMapAreaRec::GetFilename() {
    return "DBFilesClient\\WorldMapArea.dbc";
}

// VERIFY: b517c9b5 auto
// OFFSET: 0x008BB810
bool WorldMapAreaRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t areaNameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t mapIDRead = SFileReadFile(file, &this->m_mapID, sizeof(this->m_mapID), nullptr, nullptr, 0);
    int32_t areaIDRead = SFileReadFile(file, &this->m_areaID, sizeof(this->m_areaID), nullptr, nullptr, 0);
    int32_t areaNameRead = SFileReadFile(file, &areaNameOffset, sizeof(areaNameOffset), nullptr, nullptr, 0);
    int32_t locLeftRead = SFileReadFile(file, &this->m_locLeft, sizeof(this->m_locLeft), nullptr, nullptr, 0);
    int32_t locRightRead = SFileReadFile(file, &this->m_locRight, sizeof(this->m_locRight), nullptr, nullptr, 0);
    int32_t locTopRead = SFileReadFile(file, &this->m_locTop, sizeof(this->m_locTop), nullptr, nullptr, 0);
    int32_t locBottomRead = SFileReadFile(file, &this->m_locBottom, sizeof(this->m_locBottom), nullptr, nullptr, 0);
    int32_t displayMapIDRead = SFileReadFile(file, &this->m_displayMapID, sizeof(this->m_displayMapID), nullptr, nullptr, 0);
    int32_t defaultDungeonFloorRead = SFileReadFile(file, &this->m_defaultDungeonFloor, sizeof(this->m_defaultDungeonFloor), nullptr, nullptr, 0);
    int32_t parentWorldMapIDRead = SFileReadFile(file, &this->m_parentWorldMapID, sizeof(this->m_parentWorldMapID), nullptr, nullptr, 0);

    if (!idRead
        | !mapIDRead
        | !areaIDRead
        | !areaNameRead
        | !locLeftRead
        | !locRightRead
        | !locTopRead
        | !locBottomRead
        | !displayMapIDRead
        | !defaultDungeonFloorRead
        | !parentWorldMapIDRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_areaName = &stringBuffer[areaNameOffset];
    } else {
        this->m_areaName = "";
    }

    return true;
}
