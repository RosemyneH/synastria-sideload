#include "WoW/Source/DB/Rec/StationeryRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

StationeryRec::StationeryRec() {
}

StationeryRec::~StationeryRec() {
}

// VERIFY: 4054bdd6 auto
// OFFSET: 0x008B97F0
const char* StationeryRec::GetFilename() {
    return "DBFilesClient\\Stationery.dbc";
}

// VERIFY: bae9ce31 manual
// OFFSET: 0x008B9800
bool StationeryRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t textureOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t itemIDRead = SFileReadFile(file, &this->m_itemID, sizeof(this->m_itemID), nullptr, nullptr, 0);
    int32_t textureRead = SFileReadFile(file, &textureOffset, sizeof(textureOffset), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);

    if (!idRead
        | !itemIDRead
        | !textureRead
        | !flagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_texture = &stringBuffer[textureOffset];
    } else {
        this->m_texture = "";
    }

    return true;
}
