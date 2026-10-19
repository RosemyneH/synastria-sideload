#include "WoW/Source/DB/Rec/MovieFileDataRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

MovieFileDataRec::MovieFileDataRec() {
}

MovieFileDataRec::~MovieFileDataRec() {
}

// VERIFY: 56b5a5dd auto
// OFFSET: 0x008B47F0
const char* MovieFileDataRec::GetFilename() {
    return "DBFilesClient\\MovieFileData.dbc";
}

bool MovieFileDataRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t resolutionRead = SFileReadFile(file, &this->m_resolution, sizeof(this->m_resolution), nullptr, nullptr, 0);

    if (!idRead
        | !resolutionRead) {
        return false;
    }

    return true;
}
