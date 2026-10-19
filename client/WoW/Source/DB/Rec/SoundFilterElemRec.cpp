#include "WoW/Source/DB/Rec/SoundFilterElemRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SoundFilterElemRec::SoundFilterElemRec() {
}

SoundFilterElemRec::~SoundFilterElemRec() {
}

// VERIFY: d8e57a84 auto
// OFFSET: 0x008BD7F0
const char* SoundFilterElemRec::GetFilename() {
    return "DBFilesClient\\SoundFilterElem.dbc";
}

// VERIFY: 6839f692 manual
// OFFSET: 0x008BD800
bool SoundFilterElemRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t soundFilterIDRead = SFileReadFile(file, &this->m_soundFilterID, sizeof(this->m_soundFilterID), nullptr, nullptr, 0);
    int32_t orderIndexRead = SFileReadFile(file, &this->m_orderIndex, sizeof(this->m_orderIndex), nullptr, nullptr, 0);
    int32_t filterTypeRead = SFileReadFile(file, &this->m_filterType, sizeof(this->m_filterType), nullptr, nullptr, 0);
    int32_t paramsRead = SFileReadFile(file, this->m_params, sizeof(this->m_params), nullptr, nullptr, 0);

    if (!idRead
        | !soundFilterIDRead
        | !orderIndexRead
        | !filterTypeRead
        | !paramsRead) {
        return false;
    }

    return true;
}
