#include "WoW/Source/DB/Rec/VehicleUIIndicatorRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

VehicleUIIndicatorRec::VehicleUIIndicatorRec() {
}

VehicleUIIndicatorRec::~VehicleUIIndicatorRec() {
}

// VERIFY: 576f72d8 auto
// OFFSET: 0x008A8350
const char* VehicleUIIndicatorRec::GetFilename() {
    return "DBFilesClient\\VehicleUIIndicator.dbc";
}

bool VehicleUIIndicatorRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t backgroundTextureOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t backgroundTextureRead = SFileReadFile(file, &backgroundTextureOffset, sizeof(backgroundTextureOffset), nullptr, nullptr, 0);

    if (!idRead
        | !backgroundTextureRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_backgroundTexture = &stringBuffer[backgroundTextureOffset];
    } else {
        this->m_backgroundTexture = "";
    }

    return true;
}
