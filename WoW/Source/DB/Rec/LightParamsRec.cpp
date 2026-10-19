#include "WoW/Source/DB/Rec/LightParamsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

LightParamsRec::LightParamsRec() {
}

LightParamsRec::~LightParamsRec() {
}

// VERIFY: 4c6cf0ab auto
// OFFSET: 0x008BDFD0
const char* LightParamsRec::GetFilename() {
    return "DBFilesClient\\LightParams.dbc";
}

// VERIFY: 4ceb7b5a manual
// OFFSET: 0x008BD0C0
bool LightParamsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t highlightSkyRead = SFileReadFile(file, &this->m_highlightSky, sizeof(this->m_highlightSky), nullptr, nullptr, 0);
    int32_t lightSkyboxIDRead = SFileReadFile(file, &this->m_lightSkyboxID, sizeof(this->m_lightSkyboxID), nullptr, nullptr, 0);
    int32_t cloudTypeIDRead = SFileReadFile(file, &this->m_cloudTypeID, sizeof(this->m_cloudTypeID), nullptr, nullptr, 0);
    int32_t glowRead = SFileReadFile(file, &this->m_glow, sizeof(this->m_glow), nullptr, nullptr, 0);
    int32_t waterShallowAlphaRead = SFileReadFile(file, &this->m_waterShallowAlpha, sizeof(this->m_waterShallowAlpha), nullptr, nullptr, 0);
    int32_t waterDeepAlphaRead = SFileReadFile(file, &this->m_waterDeepAlpha, sizeof(this->m_waterDeepAlpha), nullptr, nullptr, 0);
    int32_t oceanShallowAlphaRead = SFileReadFile(file, &this->m_oceanShallowAlpha, sizeof(this->m_oceanShallowAlpha), nullptr, nullptr, 0);
    int32_t oceanDeepAlphaRead = SFileReadFile(file, &this->m_oceanDeepAlpha, sizeof(this->m_oceanDeepAlpha), nullptr, nullptr, 0);

    if (!idRead
        | !highlightSkyRead
        | !lightSkyboxIDRead
        | !cloudTypeIDRead
        | !glowRead
        | !waterShallowAlphaRead
        | !waterDeepAlphaRead
        | !oceanShallowAlphaRead
        | !oceanDeepAlphaRead) {
        return false;
    }

    return true;
}
