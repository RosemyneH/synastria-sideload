#include "WoW/Source/DB/Rec/PowerDisplayRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

PowerDisplayRec::PowerDisplayRec() {
}

PowerDisplayRec::~PowerDisplayRec() {
}

// VERIFY: ba658dad auto
// OFFSET: 0x008B50E0
const char* PowerDisplayRec::GetFilename() {
    return "DBFilesClient\\PowerDisplay.dbc";
}

// VERIFY: 14710959 manual
// OFFSET: 0x008B50F0
bool PowerDisplayRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t globalStringBaseTagOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t actualTypeRead = SFileReadFile(file, &this->m_actualType, sizeof(this->m_actualType), nullptr, nullptr, 0);
    int32_t globalStringBaseTagRead = SFileReadFile(file, &globalStringBaseTagOffset, sizeof(globalStringBaseTagOffset), nullptr, nullptr, 0);
    int32_t redRead = SFileReadFile(file, &this->m_red, sizeof(this->m_red), nullptr, nullptr, 0);
    int32_t greenRead = SFileReadFile(file, &this->m_green, sizeof(this->m_green), nullptr, nullptr, 0);
    int32_t blueRead = SFileReadFile(file, &this->m_blue, sizeof(this->m_blue), nullptr, nullptr, 0);

    if (!idRead
        | !actualTypeRead
        | !globalStringBaseTagRead
        | !redRead
        | !greenRead
        | !blueRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_globalStringBaseTag = &stringBuffer[globalStringBaseTagOffset];
    } else {
        this->m_globalStringBaseTag = "";
    }

    return true;
}
