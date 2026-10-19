#ifndef ENGINE_SOURCE_GX_CGXLIGHT_H
#define ENGINE_SOURCE_GX_CGXLIGHT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C3Vector.h"
#include "Engine/Source/Tempest/C4Vector.h"

class CGxLight {
    public:
    /* 0000 */ uint32_t m_flags;
    /* 0004 */ C3Vector m_dir;
    /* 0010 */ C3Vector m_ambientColor;
    /* 001C */ C3Vector m_dirColor;
    /* 0028 */ C3Vector m_specularColor;
    /* 0034 */ float m_constantAttenuation;
    /* 0038 */ float m_linearAttenuation;
    /* 003C */ float m_quadraticAttenuation;

    CGxLight();
};

class CGxApiLight {
    public:
    /* 0000 */ C4Vector m_dir;
    /* 0010 */ C3Vector m_ambColor;
    /* 001C */ C3Vector m_dirColor;
    /* 0028 */ C3Vector m_specColor;
    /* 0034 */ float m_constantAttenuation;
    /* 0038 */ float m_linearAttenuation;
    /* 003C */ float m_quadraticAttenuation;
    /* 0040 */ int32_t m_enable;
    /* 0044 */ uint16_t m_flags;
    /* 0046 */ uint16_t unk_0046;

    CGxApiLight();
    CGxApiLight& operator=(const CGxLight& light);
};

#endif
