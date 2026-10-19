#include "WoW/Source/DB/Rec/VehicleUIIndSeatRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

VehicleUIIndSeatRec::VehicleUIIndSeatRec() {
}

VehicleUIIndSeatRec::~VehicleUIIndSeatRec() {
}

// VERIFY: 1d16e1a0 auto
// OFFSET: 0x008BB550
const char* VehicleUIIndSeatRec::GetFilename() {
    return "DBFilesClient\\VehicleUIIndSeat.dbc";
}

bool VehicleUIIndSeatRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t vehicleUiindicatorIDRead = SFileReadFile(file, &this->m_vehicleUiindicatorID, sizeof(this->m_vehicleUiindicatorID), nullptr, nullptr, 0);
    int32_t virtualSeatIndexRead = SFileReadFile(file, &this->m_virtualSeatIndex, sizeof(this->m_virtualSeatIndex), nullptr, nullptr, 0);
    int32_t xposRead = SFileReadFile(file, &this->m_xpos, sizeof(this->m_xpos), nullptr, nullptr, 0);
    int32_t yposRead = SFileReadFile(file, &this->m_ypos, sizeof(this->m_ypos), nullptr, nullptr, 0);

    if (!idRead
        | !vehicleUiindicatorIDRead
        | !virtualSeatIndexRead
        | !xposRead
        | !yposRead) {
        return false;
    }

    return true;
}
