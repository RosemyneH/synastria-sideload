#include "WoW/Source/DB/Rec/GameObjectArtKitRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

GameObjectArtKitRec::GameObjectArtKitRec() {
}

GameObjectArtKitRec::~GameObjectArtKitRec() {
}

// VERIFY: 36883ae2 auto
// OFFSET: 0x008AFA00
const char* GameObjectArtKitRec::GetFilename() {
    return "DBFilesClient\\GameObjectArtKit.dbc";
}

// VERIFY: a11db924 manual
// OFFSET: 0x008AFA10
bool GameObjectArtKitRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t textureVariationOffset[3];
    uint32_t attachModelOffset[4];

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t textureVariationRead0 = SFileReadFile(file, &textureVariationOffset[0], sizeof(textureVariationOffset[0]), nullptr, nullptr, 0);
    int32_t textureVariationRead1 = SFileReadFile(file, &textureVariationOffset[1], sizeof(textureVariationOffset[1]), nullptr, nullptr, 0);
    int32_t textureVariationRead2 = SFileReadFile(file, &textureVariationOffset[2], sizeof(textureVariationOffset[2]), nullptr, nullptr, 0);
    int32_t attachModelRead0 = SFileReadFile(file, &attachModelOffset[0], sizeof(attachModelOffset[0]), nullptr, nullptr, 0);
    int32_t attachModelRead1 = SFileReadFile(file, &attachModelOffset[1], sizeof(attachModelOffset[1]), nullptr, nullptr, 0);
    int32_t attachModelRead2 = SFileReadFile(file, &attachModelOffset[2], sizeof(attachModelOffset[2]), nullptr, nullptr, 0);
    int32_t attachModelRead3 = SFileReadFile(file, &attachModelOffset[3], sizeof(attachModelOffset[3]), nullptr, nullptr, 0);

    if (!idRead
        | !textureVariationRead0
        | !textureVariationRead1
        | !textureVariationRead2
        | !attachModelRead0
        | !attachModelRead1
        | !attachModelRead2
        | !attachModelRead3) {
        return false;
    }

    if (stringBuffer) {
        this->m_textureVariation[0] = &stringBuffer[textureVariationOffset[0]];
        this->m_textureVariation[1] = &stringBuffer[textureVariationOffset[1]];
        this->m_textureVariation[2] = &stringBuffer[textureVariationOffset[2]];
        this->m_attachModel[0] = &stringBuffer[attachModelOffset[0]];
        this->m_attachModel[1] = &stringBuffer[attachModelOffset[1]];
        this->m_attachModel[2] = &stringBuffer[attachModelOffset[2]];
        this->m_attachModel[3] = &stringBuffer[attachModelOffset[3]];
    } else {
        this->m_textureVariation[0] = "";
        this->m_textureVariation[1] = "";
        this->m_textureVariation[2] = "";
        this->m_attachModel[0] = "";
        this->m_attachModel[1] = "";
        this->m_attachModel[2] = "";
        this->m_attachModel[3] = "";
    }

    return true;
}
