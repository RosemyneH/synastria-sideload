#ifndef ENGINE_SOURCE_GX_CGXGAMMARAMP_H
#define ENGINE_SOURCE_GX_CGXGAMMARAMP_H

#include <cstddef>
#include <cstdint>

class CGxGammaRamp {
    public:
    /* 0000 */ uint16_t m_red [256];
    /* 0200 */ uint16_t m_green [256];
    /* 0400 */ uint16_t m_blue [256];

    void Set(float gamma);
};

#endif
