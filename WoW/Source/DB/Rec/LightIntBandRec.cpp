#include "WoW/Source/DB/Rec/LightIntBandRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

LightIntBandRec::LightIntBandRec() {
}

LightIntBandRec::~LightIntBandRec() {
}

// VERIFY: bf18bcf3 auto
// OFFSET: 0x008BE100
const char* LightIntBandRec::GetFilename() {
    return "DBFilesClient\\LightIntBand.dbc";
}

bool LightIntBandRec::Read(SFileImpl* file, const char* stringBuffer) {
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
