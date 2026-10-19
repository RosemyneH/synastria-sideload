#ifndef ENGINE_SOURCE_MODEL2_M2MODELLIGHT_H
#define ENGINE_SOURCE_MODEL2_M2MODELLIGHT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Model2/M2Light.h"
#include "Engine/Source/Model2/M2Model.h"
#include "Engine/Source/Tempest/C3Vector.h"

class C3Vector;

class M2ModelLight {
    public:
    /* 0000 */ M2ModelTrack<C3Vector> m_ambientColorTrack;
    /* 0014 */ M2ModelTrack<float> m_ambientIntensityTrack;
    /* 0020 */ M2ModelTrack<C3Vector> m_diffuseColorTrack;
    /* 0034 */ M2ModelTrack<float> m_diffuseIntensityTrack;
    /* 0040 */ M2ModelTrack<float> m_attenuationStartTrack;
    /* 004C */ M2ModelTrack<float> m_attenuationEndTrack;
    /* 0058 */ M2ModelTrack<uint8_t> m_visibilityTrack;
    /* 0064 */ uint32_t m_enabled;
    /* 0068 */ CM2Light m_light;

    M2ModelLight();
};

#endif
