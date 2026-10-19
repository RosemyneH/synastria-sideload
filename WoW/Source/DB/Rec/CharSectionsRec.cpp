#include "WoW/Source/DB/Rec/CharSectionsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CharSectionsRec::CharSectionsRec() {
}

CharSectionsRec::~CharSectionsRec() {
}

// VERIFY: 96d2e822 auto
// OFFSET: 0x008BCC10
const char* CharSectionsRec::GetFilename() {
    return "DBFilesClient\\CharSections.dbc";
}

// VERIFY: e4364a8f manual
// OFFSET: 0x008BCC20
bool CharSectionsRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t textureNameOffset[3];

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t raceIDRead = SFileReadFile(file, &this->m_raceID, sizeof(this->m_raceID), nullptr, nullptr, 0);
    int32_t sexIDRead = SFileReadFile(file, &this->m_sexID, sizeof(this->m_sexID), nullptr, nullptr, 0);
    int32_t baseSectionRead = SFileReadFile(file, &this->m_baseSection, sizeof(this->m_baseSection), nullptr, nullptr, 0);
    int32_t textureNameRead0 = SFileReadFile(file, &textureNameOffset[0], sizeof(textureNameOffset[0]), nullptr, nullptr, 0);
    int32_t textureNameRead1 = SFileReadFile(file, &textureNameOffset[1], sizeof(textureNameOffset[1]), nullptr, nullptr, 0);
    int32_t textureNameRead2 = SFileReadFile(file, &textureNameOffset[2], sizeof(textureNameOffset[2]), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t variationIndexRead = SFileReadFile(file, &this->m_variationIndex, sizeof(this->m_variationIndex), nullptr, nullptr, 0);
    int32_t colorIndexRead = SFileReadFile(file, &this->m_colorIndex, sizeof(this->m_colorIndex), nullptr, nullptr, 0);

    if (!idRead
        | !raceIDRead
        | !sexIDRead
        | !baseSectionRead
        | !textureNameRead0
        | !textureNameRead1
        | !textureNameRead2
        | !flagsRead
        | !variationIndexRead
        | !colorIndexRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_textureName[0] = &stringBuffer[textureNameOffset[0]];
        this->m_textureName[1] = &stringBuffer[textureNameOffset[1]];
        this->m_textureName[2] = &stringBuffer[textureNameOffset[2]];
    } else {
        this->m_textureName[0] = "";
        this->m_textureName[1] = "";
        this->m_textureName[2] = "";
    }

    return true;
}
