#include "WoW/Source/DB/Rec/LightFloatBandRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

LightFloatBandRec::LightFloatBandRec() {
}

LightFloatBandRec::~LightFloatBandRec() {
}

// VERIFY: 9c915bb5 auto
// OFFSET: 0x008BE1A0
const char* LightFloatBandRec::GetFilename() {
    return "DBFilesClient\\LightFloatBand.dbc";
}

// VERIFY: 77794b45 manual
// OFFSET: 0x008BE110
bool LightFloatBandRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t numRead = SFileReadFile(file, &this->m_num, sizeof(this->m_num), nullptr, nullptr, 0);
    int32_t timeRead = SFileReadFile(file, this->m_time, sizeof(this->m_time), nullptr, nullptr, 0);
    int32_t dataRead = SFileReadFile(file, this->m_data, sizeof(this->m_data), nullptr, nullptr, 0);

    if (!idRead
        | !numRead
        | !timeRead
        | !dataRead) {
        return false;
    }

    return true;
}
