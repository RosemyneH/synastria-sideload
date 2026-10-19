#include "WoW/Source/DB/Rec/MovieRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

MovieRec::MovieRec() {
}

MovieRec::~MovieRec() {
}

// VERIFY: f4116784 auto
// OFFSET: 0x008B47E0
const char* MovieRec::GetFilename() {
    return "DBFilesClient\\Movie.dbc";
}

bool MovieRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t filenameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t filenameRead = SFileReadFile(file, &filenameOffset, sizeof(filenameOffset), nullptr, nullptr, 0);
    int32_t volumeRead = SFileReadFile(file, &this->m_volume, sizeof(this->m_volume), nullptr, nullptr, 0);

    if (!idRead
        | !filenameRead
        | !volumeRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_filename = &stringBuffer[filenameOffset];
    } else {
        this->m_filename = "";
    }

    return true;
}
