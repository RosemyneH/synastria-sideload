#include "WoW/Source/DB/Rec/LoadingScreensRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

LoadingScreensRec::LoadingScreensRec() {
}

LoadingScreensRec::~LoadingScreensRec() {
}

// VERIFY: c61accdb auto
// OFFSET: 0x008B2B10
const char* LoadingScreensRec::GetFilename() {
    return "DBFilesClient\\LoadingScreens.dbc";
}

// VERIFY: d86dd03f manual
// OFFSET: 0x008B2B20
bool LoadingScreensRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;
    uint32_t fileNameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t fileNameRead = SFileReadFile(file, &fileNameOffset, sizeof(fileNameOffset), nullptr, nullptr, 0);
    int32_t hasWideScreenRead = SFileReadFile(file, &this->m_hasWideScreen, sizeof(this->m_hasWideScreen), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead
        | !fileNameRead
        | !hasWideScreenRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
        this->m_fileName = &stringBuffer[fileNameOffset];
    } else {
        this->m_name = "";
        this->m_fileName = "";
    }

    return true;
}
