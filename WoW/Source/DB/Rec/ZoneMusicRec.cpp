#include "WoW/Source/DB/Rec/ZoneMusicRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ZoneMusicRec::ZoneMusicRec() {
}

ZoneMusicRec::~ZoneMusicRec() {
}

// VERIFY: 2aba63b3 auto
// OFFSET: 0x008BCEA0
const char* ZoneMusicRec::GetFilename() {
    return "DBFilesClient\\ZoneMusic.dbc";
}

// VERIFY: 9c300d4b manual
// OFFSET: 0x008BCEB0
bool ZoneMusicRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t setNameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t setNameRead = SFileReadFile(file, &setNameOffset, sizeof(setNameOffset), nullptr, nullptr, 0);
    int32_t silenceIntervalMinRead = SFileReadFile(file, this->m_silenceIntervalMin, sizeof(this->m_silenceIntervalMin), nullptr, nullptr, 0);
    int32_t silenceIntervalMaxRead = SFileReadFile(file, this->m_silenceIntervalMax, sizeof(this->m_silenceIntervalMax), nullptr, nullptr, 0);
    int32_t soundsRead = SFileReadFile(file, this->m_sounds, sizeof(this->m_sounds), nullptr, nullptr, 0);

    if (!idRead
        | !setNameRead
        | !silenceIntervalMinRead
        | !silenceIntervalMaxRead
        | !soundsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_setName = &stringBuffer[setNameOffset];
    } else {
        this->m_setName = "";
    }

    return true;
}
