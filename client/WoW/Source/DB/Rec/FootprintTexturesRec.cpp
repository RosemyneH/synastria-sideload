#include "WoW/Source/DB/Rec/FootprintTexturesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

FootprintTexturesRec::FootprintTexturesRec() {
}

FootprintTexturesRec::~FootprintTexturesRec() {
}

// VERIFY: cff07304 auto
// OFFSET: 0x008AF9E0
const char* FootprintTexturesRec::GetFilename() {
    return "DBFilesClient\\FootprintTextures.dbc";
}

bool FootprintTexturesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t footstepFilenameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t footstepFilenameRead = SFileReadFile(file, &footstepFilenameOffset, sizeof(footstepFilenameOffset), nullptr, nullptr, 0);

    if (!idRead
        | !footstepFilenameRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_footstepFilename = &stringBuffer[footstepFilenameOffset];
    } else {
        this->m_footstepFilename = "";
    }

    return true;
}
