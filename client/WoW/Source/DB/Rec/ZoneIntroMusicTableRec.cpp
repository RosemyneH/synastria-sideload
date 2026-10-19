#include "WoW/Source/DB/Rec/ZoneIntroMusicTableRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ZoneIntroMusicTableRec::ZoneIntroMusicTableRec() {
}

ZoneIntroMusicTableRec::~ZoneIntroMusicTableRec() {
}

// VERIFY: 2690f50c auto
// OFFSET: 0x008BCDB0
const char* ZoneIntroMusicTableRec::GetFilename() {
    return "DBFilesClient\\ZoneIntroMusicTable.dbc";
}

// VERIFY: 765af9d6 manual
// OFFSET: 0x008BCDC0
bool ZoneIntroMusicTableRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, &this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);
    int32_t priorityRead = SFileReadFile(file, &this->m_priority, sizeof(this->m_priority), nullptr, nullptr, 0);
    int32_t minDelayMinutesRead = SFileReadFile(file, &this->m_minDelayMinutes, sizeof(this->m_minDelayMinutes), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead
        | !soundIDRead
        | !priorityRead
        | !minDelayMinutesRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
    } else {
        this->m_name = "";
    }

    return true;
}
