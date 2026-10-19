#include "WoW/Source/DB/Rec/FileDataRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

FileDataRec::FileDataRec() {
}

FileDataRec::~FileDataRec() {
}

// VERIFY: 9f617f37 auto
// OFFSET: 0x008AF920
const char* FileDataRec::GetFilename() {
    return "DBFilesClient\\FileData.dbc";
}

// VERIFY: 96ff1384 auto
// OFFSET: 0x008AF930
bool FileDataRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t filenameOffset;
    uint32_t filepathOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t filenameRead = SFileReadFile(file, &filenameOffset, sizeof(filenameOffset), nullptr, nullptr, 0);
    int32_t filepathRead = SFileReadFile(file, &filepathOffset, sizeof(filepathOffset), nullptr, nullptr, 0);

    if (!idRead
        | !filenameRead
        | !filepathRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_filename = &stringBuffer[filenameOffset];
        this->m_filepath = &stringBuffer[filepathOffset];
    } else {
        this->m_filename = "";
        this->m_filepath = "";
    }

    return true;
}
