#include "WoW/Source/DB/Rec/HolidaysRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

HolidaysRec::HolidaysRec() {
}

HolidaysRec::~HolidaysRec() {
}

// VERIFY: edb14f24 auto
// OFFSET: 0x008B00E0
const char* HolidaysRec::GetFilename() {
    return "DBFilesClient\\Holidays.dbc";
}

// VERIFY: 3a1dc65c auto
// OFFSET: 0x008B00F0
bool HolidaysRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t textureFileNameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t durationRead = SFileReadFile(file, this->m_duration, sizeof(this->m_duration), nullptr, nullptr, 0);
    int32_t dateRead = SFileReadFile(file, this->m_date, sizeof(this->m_date), nullptr, nullptr, 0);
    int32_t regionRead = SFileReadFile(file, &this->m_region, sizeof(this->m_region), nullptr, nullptr, 0);
    int32_t loopingRead = SFileReadFile(file, &this->m_looping, sizeof(this->m_looping), nullptr, nullptr, 0);
    int32_t calendarFlagsRead = SFileReadFile(file, this->m_calendarFlags, sizeof(this->m_calendarFlags), nullptr, nullptr, 0);
    int32_t holidayNameIDRead = SFileReadFile(file, &this->m_holidayNameID, sizeof(this->m_holidayNameID), nullptr, nullptr, 0);
    int32_t holidayDescriptionIDRead = SFileReadFile(file, &this->m_holidayDescriptionID, sizeof(this->m_holidayDescriptionID), nullptr, nullptr, 0);
    int32_t textureFileNameRead = SFileReadFile(file, &textureFileNameOffset, sizeof(textureFileNameOffset), nullptr, nullptr, 0);
    int32_t priorityRead = SFileReadFile(file, &this->m_priority, sizeof(this->m_priority), nullptr, nullptr, 0);
    int32_t calendarFilterTypeRead = SFileReadFile(file, &this->m_calendarFilterType, sizeof(this->m_calendarFilterType), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);

    if (!idRead
        | !durationRead
        | !dateRead
        | !regionRead
        | !loopingRead
        | !calendarFlagsRead
        | !holidayNameIDRead
        | !holidayDescriptionIDRead
        | !textureFileNameRead
        | !priorityRead
        | !calendarFilterTypeRead
        | !flagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_textureFileName = &stringBuffer[textureFileNameOffset];
    } else {
        this->m_textureFileName = "";
    }

    return true;
}
