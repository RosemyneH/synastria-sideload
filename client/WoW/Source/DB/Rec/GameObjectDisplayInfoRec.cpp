#include "WoW/Source/DB/Rec/GameObjectDisplayInfoRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

GameObjectDisplayInfoRec::GameObjectDisplayInfoRec() {
}

GameObjectDisplayInfoRec::~GameObjectDisplayInfoRec() {
}

// VERIFY: 72ed8f7b auto
// OFFSET: 0x008AFB90
const char* GameObjectDisplayInfoRec::GetFilename() {
    return "DBFilesClient\\GameObjectDisplayInfo.dbc";
}

// VERIFY: 5d917f4b auto
// OFFSET: 0x008AFBA0
bool GameObjectDisplayInfoRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t modelNameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t modelNameRead = SFileReadFile(file, &modelNameOffset, sizeof(modelNameOffset), nullptr, nullptr, 0);
    int32_t soundRead = SFileReadFile(file, this->m_sound, sizeof(this->m_sound), nullptr, nullptr, 0);
    int32_t geoBoxMinRead0 = SFileReadFile(file, &this->m_geoBoxMin[0], sizeof(this->m_geoBoxMin[0]), nullptr, nullptr, 0);
    int32_t geoBoxMinRead1 = SFileReadFile(file, &this->m_geoBoxMin[1], sizeof(this->m_geoBoxMin[1]), nullptr, nullptr, 0);
    int32_t geoBoxMinRead2 = SFileReadFile(file, &this->m_geoBoxMin[2], sizeof(this->m_geoBoxMin[2]), nullptr, nullptr, 0);
    int32_t geoBoxMaxRead0 = SFileReadFile(file, &this->m_geoBoxMax[0], sizeof(this->m_geoBoxMax[0]), nullptr, nullptr, 0);
    int32_t geoBoxMaxRead1 = SFileReadFile(file, &this->m_geoBoxMax[1], sizeof(this->m_geoBoxMax[1]), nullptr, nullptr, 0);
    int32_t geoBoxMaxRead2 = SFileReadFile(file, &this->m_geoBoxMax[2], sizeof(this->m_geoBoxMax[2]), nullptr, nullptr, 0);
    int32_t objectEffectPackageIDRead = SFileReadFile(file, &this->m_objectEffectPackageID, sizeof(this->m_objectEffectPackageID), nullptr, nullptr, 0);

    if (!idRead
        | !modelNameRead
        | !soundRead
        | !geoBoxMinRead0
        | !geoBoxMinRead1
        | !geoBoxMinRead2
        | !geoBoxMaxRead0
        | !geoBoxMaxRead1
        | !geoBoxMaxRead2
        | !objectEffectPackageIDRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_modelName = &stringBuffer[modelNameOffset];
    } else {
        this->m_modelName = "";
    }

    return true;
}
