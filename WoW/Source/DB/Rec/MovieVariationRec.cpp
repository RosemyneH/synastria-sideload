#include "WoW/Source/DB/Rec/MovieVariationRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

MovieVariationRec::MovieVariationRec() {
}

MovieVariationRec::~MovieVariationRec() {
}

// VERIFY: 7dbd6c65 auto
// OFFSET: 0x008B4800
const char* MovieVariationRec::GetFilename() {
    return "DBFilesClient\\MovieVariation.dbc";
}

bool MovieVariationRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t movieIDRead = SFileReadFile(file, &this->m_movieID, sizeof(this->m_movieID), nullptr, nullptr, 0);
    int32_t fileDataIDRead = SFileReadFile(file, &this->m_fileDataID, sizeof(this->m_fileDataID), nullptr, nullptr, 0);

    if (!idRead
        | !movieIDRead
        | !fileDataIDRead) {
        return false;
    }

    return true;
}
