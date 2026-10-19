#include <cstdint>
#include <cstring>
#include "Engine/Source/Gx/CGxDevice/CGxDevice.h"
#include "Engine/Source/Gx/CGxTexFlags.h"

// VERIFY: e9d54e21 manual
// OFFSET: 0x00681BE0
CGxTexFlags::CGxTexFlags(EGxTexFilter filter, uint32_t wrapU, uint32_t wrapV, uint32_t forceMipTracking, uint32_t generateMipMaps, uint32_t renderTarget, uint32_t maxAnisotropy, uint32_t bit8, uint32_t bit14, uint32_t bit15) {
    this->m_unk16 = 0;
    this->m_bit31 = 0;
    this->m_filter = filter;
    this->m_wrapU = wrapU;
    this->m_wrapV = wrapV;
    this->m_forceMipTracking = forceMipTracking;
    this->m_generateMipMaps = generateMipMaps;
    this->m_renderTarget = renderTarget;
    this->m_bit8 = bit8;
    this->m_maxAnisotropy = maxAnisotropy < g_theGxDevicePtr->Caps().m_maxTexAnisotropy ? maxAnisotropy : g_theGxDevicePtr->Caps().m_maxTexAnisotropy;
    this->m_bit14 = bit14;
    this->m_bit15 = bit15;
}

// VERIFY: aa1674cd manual
// OFFSET: 0x00683AE0
bool CGxTexFlags::operator!=(const CGxTexFlags& flags) const {
    return memcmp(this, &flags, sizeof(CGxTexFlags)) != 0;
}

// VERIFY: f82cbd59 manual
// OFFSET: 0x004B5820
bool CGxTexFlags::operator==(const CGxTexFlags& flags) const {
    return memcmp(this, &flags, sizeof(CGxTexFlags)) == 0;
}
