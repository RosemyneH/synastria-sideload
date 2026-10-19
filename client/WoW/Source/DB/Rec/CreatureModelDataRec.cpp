#include "WoW/Source/DB/Rec/CreatureModelDataRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CreatureModelDataRec::CreatureModelDataRec() {
}

CreatureModelDataRec::~CreatureModelDataRec() {
}

// VERIFY: 5ebcc290 auto
// OFFSET: 0x008AD7F0
const char* CreatureModelDataRec::GetFilename() {
    return "DBFilesClient\\CreatureModelData.dbc";
}

// VERIFY: e3868028 auto
// OFFSET: 0x008AD800
bool CreatureModelDataRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t modelNameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t modelNameRead = SFileReadFile(file, &modelNameOffset, sizeof(modelNameOffset), nullptr, nullptr, 0);
    int32_t sizeClassRead = SFileReadFile(file, &this->m_sizeClass, sizeof(this->m_sizeClass), nullptr, nullptr, 0);
    int32_t modelScaleRead = SFileReadFile(file, &this->m_modelScale, sizeof(this->m_modelScale), nullptr, nullptr, 0);
    int32_t bloodIDRead = SFileReadFile(file, &this->m_bloodID, sizeof(this->m_bloodID), nullptr, nullptr, 0);
    int32_t footprintTextureIDRead = SFileReadFile(file, &this->m_footprintTextureID, sizeof(this->m_footprintTextureID), nullptr, nullptr, 0);
    int32_t footprintTextureLengthRead = SFileReadFile(file, &this->m_footprintTextureLength, sizeof(this->m_footprintTextureLength), nullptr, nullptr, 0);
    int32_t footprintTextureWidthRead = SFileReadFile(file, &this->m_footprintTextureWidth, sizeof(this->m_footprintTextureWidth), nullptr, nullptr, 0);
    int32_t footprintParticleScaleRead = SFileReadFile(file, &this->m_footprintParticleScale, sizeof(this->m_footprintParticleScale), nullptr, nullptr, 0);
    int32_t foleyMaterialIDRead = SFileReadFile(file, &this->m_foleyMaterialID, sizeof(this->m_foleyMaterialID), nullptr, nullptr, 0);
    int32_t footstepShakeSizeRead = SFileReadFile(file, &this->m_footstepShakeSize, sizeof(this->m_footstepShakeSize), nullptr, nullptr, 0);
    int32_t deathThudShakeSizeRead = SFileReadFile(file, &this->m_deathThudShakeSize, sizeof(this->m_deathThudShakeSize), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, &this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);
    int32_t collisionWidthRead = SFileReadFile(file, &this->m_collisionWidth, sizeof(this->m_collisionWidth), nullptr, nullptr, 0);
    int32_t collisionHeightRead = SFileReadFile(file, &this->m_collisionHeight, sizeof(this->m_collisionHeight), nullptr, nullptr, 0);
    int32_t mountHeightRead = SFileReadFile(file, &this->m_mountHeight, sizeof(this->m_mountHeight), nullptr, nullptr, 0);
    int32_t geoBoxMinXRead = SFileReadFile(file, &this->m_geoBoxMinX, sizeof(this->m_geoBoxMinX), nullptr, nullptr, 0);
    int32_t geoBoxMinYRead = SFileReadFile(file, &this->m_geoBoxMinY, sizeof(this->m_geoBoxMinY), nullptr, nullptr, 0);
    int32_t geoBoxMinZRead = SFileReadFile(file, &this->m_geoBoxMinZ, sizeof(this->m_geoBoxMinZ), nullptr, nullptr, 0);
    int32_t geoBoxMaxXRead = SFileReadFile(file, &this->m_geoBoxMaxX, sizeof(this->m_geoBoxMaxX), nullptr, nullptr, 0);
    int32_t geoBoxMaxYRead = SFileReadFile(file, &this->m_geoBoxMaxY, sizeof(this->m_geoBoxMaxY), nullptr, nullptr, 0);
    int32_t geoBoxMaxZRead = SFileReadFile(file, &this->m_geoBoxMaxZ, sizeof(this->m_geoBoxMaxZ), nullptr, nullptr, 0);
    int32_t worldEffectScaleRead = SFileReadFile(file, &this->m_worldEffectScale, sizeof(this->m_worldEffectScale), nullptr, nullptr, 0);
    int32_t attachedEffectScaleRead = SFileReadFile(file, &this->m_attachedEffectScale, sizeof(this->m_attachedEffectScale), nullptr, nullptr, 0);
    int32_t missileCollisionRadiusRead = SFileReadFile(file, &this->m_missileCollisionRadius, sizeof(this->m_missileCollisionRadius), nullptr, nullptr, 0);
    int32_t missileCollisionPushRead = SFileReadFile(file, &this->m_missileCollisionPush, sizeof(this->m_missileCollisionPush), nullptr, nullptr, 0);
    int32_t missileCollisionRaiseRead = SFileReadFile(file, &this->m_missileCollisionRaise, sizeof(this->m_missileCollisionRaise), nullptr, nullptr, 0);

    if (!idRead
        | !flagsRead
        | !modelNameRead
        | !sizeClassRead
        | !modelScaleRead
        | !bloodIDRead
        | !footprintTextureIDRead
        | !footprintTextureLengthRead
        | !footprintTextureWidthRead
        | !footprintParticleScaleRead
        | !foleyMaterialIDRead
        | !footstepShakeSizeRead
        | !deathThudShakeSizeRead
        | !soundIDRead
        | !collisionWidthRead
        | !collisionHeightRead
        | !mountHeightRead
        | !geoBoxMinXRead
        | !geoBoxMinYRead
        | !geoBoxMinZRead
        | !geoBoxMaxXRead
        | !geoBoxMaxYRead
        | !geoBoxMaxZRead
        | !worldEffectScaleRead
        | !attachedEffectScaleRead
        | !missileCollisionRadiusRead
        | !missileCollisionPushRead
        | !missileCollisionRaiseRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_modelName = &stringBuffer[modelNameOffset];
    } else {
        this->m_modelName = "";
    }

    return true;
}
