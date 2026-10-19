#ifndef ENGINE_SOURCE_SHADEREFFECT_TYPES_H
#define ENGINE_SOURCE_SHADEREFFECT_TYPES_H

#include <cstddef>
#include <cstdint>

enum EGxTexOp {
    GxTexOp_Mod = 0,
    GxTexOp_Mod2x = 1,
    GxTexOp_Add = 2,
    GxTexOp_PassThru = 3,
    GxTexOp_Decal = 4,
    GxTexOp_Fade = 5,
    GxTexOps_Last = 6,
};

enum EShadowMode {
    SHADOW_LOWEST = 0,
    SHADOW_LOW = 1,
    SHADOW_MEDIUM = 2,
    SHADOW_MEDIUM_HIGH = 3,
    SHADOW_HIGH = 4,
    SHADOW_VERY_HIGH = 5,
    NUM_SHADOW_MODES = 6,
};

#endif
