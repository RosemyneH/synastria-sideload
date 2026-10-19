#include "WoW/Source/DB/Rec/VideoHardwareRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

VideoHardwareRec::VideoHardwareRec() {
}

VideoHardwareRec::~VideoHardwareRec() {
}

// VERIFY: f82d0fbc auto
// OFFSET: 0x008BDC90
const char* VideoHardwareRec::GetFilename() {
    return "DBFilesClient\\VideoHardware.dbc";
}

// VERIFY: aa364a07 auto
// OFFSET: 0x008BDCA0
bool VideoHardwareRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t oglOverridesOffset;
    uint32_t d3DOverridesOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t vendorIDRead = SFileReadFile(file, &this->m_vendorID, sizeof(this->m_vendorID), nullptr, nullptr, 0);
    int32_t deviceIDRead = SFileReadFile(file, &this->m_deviceID, sizeof(this->m_deviceID), nullptr, nullptr, 0);
    int32_t farclipIdxRead = SFileReadFile(file, &this->m_farclipIdx, sizeof(this->m_farclipIdx), nullptr, nullptr, 0);
    int32_t terrainLoddistIdxRead = SFileReadFile(file, &this->m_terrainLoddistIdx, sizeof(this->m_terrainLoddistIdx), nullptr, nullptr, 0);
    int32_t terrainShadowLodRead = SFileReadFile(file, &this->m_terrainShadowLod, sizeof(this->m_terrainShadowLod), nullptr, nullptr, 0);
    int32_t detailDoodadDensityIdxRead = SFileReadFile(file, &this->m_detailDoodadDensityIdx, sizeof(this->m_detailDoodadDensityIdx), nullptr, nullptr, 0);
    int32_t detailDoodadAlphaRead = SFileReadFile(file, &this->m_detailDoodadAlpha, sizeof(this->m_detailDoodadAlpha), nullptr, nullptr, 0);
    int32_t animatingDoodadIdxRead = SFileReadFile(file, &this->m_animatingDoodadIdx, sizeof(this->m_animatingDoodadIdx), nullptr, nullptr, 0);
    int32_t trilinearRead = SFileReadFile(file, &this->m_trilinear, sizeof(this->m_trilinear), nullptr, nullptr, 0);
    int32_t numLightsRead = SFileReadFile(file, &this->m_numLights, sizeof(this->m_numLights), nullptr, nullptr, 0);
    int32_t specularityRead = SFileReadFile(file, &this->m_specularity, sizeof(this->m_specularity), nullptr, nullptr, 0);
    int32_t waterLodidxRead = SFileReadFile(file, &this->m_waterLodidx, sizeof(this->m_waterLodidx), nullptr, nullptr, 0);
    int32_t particleDensityIdxRead = SFileReadFile(file, &this->m_particleDensityIdx, sizeof(this->m_particleDensityIdx), nullptr, nullptr, 0);
    int32_t unitDrawDistIdxRead = SFileReadFile(file, &this->m_unitDrawDistIdx, sizeof(this->m_unitDrawDistIdx), nullptr, nullptr, 0);
    int32_t smallCullDistIdxRead = SFileReadFile(file, &this->m_smallCullDistIdx, sizeof(this->m_smallCullDistIdx), nullptr, nullptr, 0);
    int32_t resolutionIdxRead = SFileReadFile(file, &this->m_resolutionIdx, sizeof(this->m_resolutionIdx), nullptr, nullptr, 0);
    int32_t baseMipLevelRead = SFileReadFile(file, &this->m_baseMipLevel, sizeof(this->m_baseMipLevel), nullptr, nullptr, 0);
    int32_t oglOverridesRead = SFileReadFile(file, &oglOverridesOffset, sizeof(oglOverridesOffset), nullptr, nullptr, 0);
    int32_t d3DOverridesRead = SFileReadFile(file, &d3DOverridesOffset, sizeof(d3DOverridesOffset), nullptr, nullptr, 0);
    int32_t fixLagRead = SFileReadFile(file, &this->m_fixLag, sizeof(this->m_fixLag), nullptr, nullptr, 0);
    int32_t multisampleRead = SFileReadFile(file, &this->m_multisample, sizeof(this->m_multisample), nullptr, nullptr, 0);
    int32_t atlasdisableRead = SFileReadFile(file, &this->m_atlasdisable, sizeof(this->m_atlasdisable), nullptr, nullptr, 0);

    if (!idRead
        | !vendorIDRead
        | !deviceIDRead
        | !farclipIdxRead
        | !terrainLoddistIdxRead
        | !terrainShadowLodRead
        | !detailDoodadDensityIdxRead
        | !detailDoodadAlphaRead
        | !animatingDoodadIdxRead
        | !trilinearRead
        | !numLightsRead
        | !specularityRead
        | !waterLodidxRead
        | !particleDensityIdxRead
        | !unitDrawDistIdxRead
        | !smallCullDistIdxRead
        | !resolutionIdxRead
        | !baseMipLevelRead
        | !oglOverridesRead
        | !d3DOverridesRead
        | !fixLagRead
        | !multisampleRead
        | !atlasdisableRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_oglOverrides = &stringBuffer[oglOverridesOffset];
        this->m_d3DOverrides = &stringBuffer[d3DOverridesOffset];
    } else {
        this->m_oglOverrides = "";
        this->m_d3DOverrides = "";
    }

    return true;
}
