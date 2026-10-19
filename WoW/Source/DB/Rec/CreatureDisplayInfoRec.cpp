#include "WoW/Source/DB/Rec/CreatureDisplayInfoRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CreatureDisplayInfoRec::CreatureDisplayInfoRec() {
}

CreatureDisplayInfoRec::~CreatureDisplayInfoRec() {
}

// VERIFY: 818e431a auto
// OFFSET: 0x008AD1C0
const char* CreatureDisplayInfoRec::GetFilename() {
    return "DBFilesClient\\CreatureDisplayInfo.dbc";
}

// VERIFY: 2c948a2c auto
// OFFSET: 0x008AD1D0
bool CreatureDisplayInfoRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t textureVariationOffset[3];
    uint32_t portraitTextureNameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t modelIDRead = SFileReadFile(file, &this->m_modelID, sizeof(this->m_modelID), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, &this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);
    int32_t extendedDisplayInfoIDRead = SFileReadFile(file, &this->m_extendedDisplayInfoID, sizeof(this->m_extendedDisplayInfoID), nullptr, nullptr, 0);
    int32_t creatureModelScaleRead = SFileReadFile(file, &this->m_creatureModelScale, sizeof(this->m_creatureModelScale), nullptr, nullptr, 0);
    int32_t creatureModelAlphaRead = SFileReadFile(file, &this->m_creatureModelAlpha, sizeof(this->m_creatureModelAlpha), nullptr, nullptr, 0);
    int32_t textureVariationRead0 = SFileReadFile(file, &textureVariationOffset[0], sizeof(textureVariationOffset[0]), nullptr, nullptr, 0);
    int32_t textureVariationRead1 = SFileReadFile(file, &textureVariationOffset[1], sizeof(textureVariationOffset[1]), nullptr, nullptr, 0);
    int32_t textureVariationRead2 = SFileReadFile(file, &textureVariationOffset[2], sizeof(textureVariationOffset[2]), nullptr, nullptr, 0);
    int32_t portraitTextureNameRead = SFileReadFile(file, &portraitTextureNameOffset, sizeof(portraitTextureNameOffset), nullptr, nullptr, 0);
    int32_t sizeClassRead = SFileReadFile(file, &this->m_sizeClass, sizeof(this->m_sizeClass), nullptr, nullptr, 0);
    int32_t bloodIDRead = SFileReadFile(file, &this->m_bloodID, sizeof(this->m_bloodID), nullptr, nullptr, 0);
    int32_t npcsoundIDRead = SFileReadFile(file, &this->m_npcsoundID, sizeof(this->m_npcsoundID), nullptr, nullptr, 0);
    int32_t particleColorIDRead = SFileReadFile(file, &this->m_particleColorID, sizeof(this->m_particleColorID), nullptr, nullptr, 0);
    int32_t creatureGeosetDataRead = SFileReadFile(file, &this->m_creatureGeosetData, sizeof(this->m_creatureGeosetData), nullptr, nullptr, 0);
    int32_t objectEffectPackageIDRead = SFileReadFile(file, &this->m_objectEffectPackageID, sizeof(this->m_objectEffectPackageID), nullptr, nullptr, 0);

    if (!idRead
        | !modelIDRead
        | !soundIDRead
        | !extendedDisplayInfoIDRead
        | !creatureModelScaleRead
        | !creatureModelAlphaRead
        | !textureVariationRead0
        | !textureVariationRead1
        | !textureVariationRead2
        | !portraitTextureNameRead
        | !sizeClassRead
        | !bloodIDRead
        | !npcsoundIDRead
        | !particleColorIDRead
        | !creatureGeosetDataRead
        | !objectEffectPackageIDRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_textureVariation[0] = &stringBuffer[textureVariationOffset[0]];
        this->m_textureVariation[1] = &stringBuffer[textureVariationOffset[1]];
        this->m_textureVariation[2] = &stringBuffer[textureVariationOffset[2]];
        this->m_portraitTextureName = &stringBuffer[portraitTextureNameOffset];
    } else {
        this->m_textureVariation[0] = "";
        this->m_textureVariation[1] = "";
        this->m_textureVariation[2] = "";
        this->m_portraitTextureName = "";
    }

    return true;
}
