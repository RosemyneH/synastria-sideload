#include "WoW/Source/DB/Rec/WorldMapOverlayRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

WorldMapOverlayRec::WorldMapOverlayRec() {
}

WorldMapOverlayRec::~WorldMapOverlayRec() {
}

// VERIFY: 563e0689 auto
// OFFSET: 0x008BBB90
const char* WorldMapOverlayRec::GetFilename() {
    return "DBFilesClient\\WorldMapOverlay.dbc";
}

// VERIFY: 4b486167 auto
// OFFSET: 0x008BBBA0
bool WorldMapOverlayRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t textureNameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t mapAreaIDRead = SFileReadFile(file, &this->m_mapAreaID, sizeof(this->m_mapAreaID), nullptr, nullptr, 0);
    int32_t areaIDRead = SFileReadFile(file, this->m_areaID, sizeof(this->m_areaID), nullptr, nullptr, 0);
    int32_t mapPointXRead = SFileReadFile(file, &this->m_mapPointX, sizeof(this->m_mapPointX), nullptr, nullptr, 0);
    int32_t mapPointYRead = SFileReadFile(file, &this->m_mapPointY, sizeof(this->m_mapPointY), nullptr, nullptr, 0);
    int32_t textureNameRead = SFileReadFile(file, &textureNameOffset, sizeof(textureNameOffset), nullptr, nullptr, 0);
    int32_t textureWidthRead = SFileReadFile(file, &this->m_textureWidth, sizeof(this->m_textureWidth), nullptr, nullptr, 0);
    int32_t textureHeightRead = SFileReadFile(file, &this->m_textureHeight, sizeof(this->m_textureHeight), nullptr, nullptr, 0);
    int32_t offsetXRead = SFileReadFile(file, &this->m_offsetX, sizeof(this->m_offsetX), nullptr, nullptr, 0);
    int32_t offsetYRead = SFileReadFile(file, &this->m_offsetY, sizeof(this->m_offsetY), nullptr, nullptr, 0);
    int32_t hitRectTopRead = SFileReadFile(file, &this->m_hitRectTop, sizeof(this->m_hitRectTop), nullptr, nullptr, 0);
    int32_t hitRectLeftRead = SFileReadFile(file, &this->m_hitRectLeft, sizeof(this->m_hitRectLeft), nullptr, nullptr, 0);
    int32_t hitRectBottomRead = SFileReadFile(file, &this->m_hitRectBottom, sizeof(this->m_hitRectBottom), nullptr, nullptr, 0);
    int32_t hitRectRightRead = SFileReadFile(file, &this->m_hitRectRight, sizeof(this->m_hitRectRight), nullptr, nullptr, 0);

    if (!idRead
        | !mapAreaIDRead
        | !areaIDRead
        | !mapPointXRead
        | !mapPointYRead
        | !textureNameRead
        | !textureWidthRead
        | !textureHeightRead
        | !offsetXRead
        | !offsetYRead
        | !hitRectTopRead
        | !hitRectLeftRead
        | !hitRectBottomRead
        | !hitRectRightRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_textureName = &stringBuffer[textureNameOffset];
    } else {
        this->m_textureName = "";
    }

    return true;
}
