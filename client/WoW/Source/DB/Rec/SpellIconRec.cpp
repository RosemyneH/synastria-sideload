#include "WoW/Source/DB/Rec/SpellIconRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellIconRec::SpellIconRec() {
}

SpellIconRec::~SpellIconRec() {
}

// VERIFY: 54966b95 auto
// OFFSET: 0x008B7DC0
const char* SpellIconRec::GetFilename() {
    return "DBFilesClient\\SpellIcon.dbc";
}

bool SpellIconRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t textureFilenameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t textureFilenameRead = SFileReadFile(file, &textureFilenameOffset, sizeof(textureFilenameOffset), nullptr, nullptr, 0);

    if (!idRead
        | !textureFilenameRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_textureFilename = &stringBuffer[textureFilenameOffset];
    } else {
        this->m_textureFilename = "";
    }

    return true;
}
