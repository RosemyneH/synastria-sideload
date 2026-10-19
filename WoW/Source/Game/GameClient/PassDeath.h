#ifndef WOW_SOURCE_GAME_GAMECLIENT_PASSDEATH_H
#define WOW_SOURCE_GAME_GAMECLIENT_PASSDEATH_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/FFX/FFX.h"

class CGxShader;

class PassDeath : public FFX::Pass {
    public:
    /* 0030 */ int32_t m_useRect;
    /* 0034 */ CGxShader* m_shaders[2];

    virtual ~PassDeath();
    virtual bool Valid();
    virtual void Execute();

    PassDeath(FFX::Tex** inputs, uint32_t count, FFX::Tex* output, EGxBlend blend, bool unk);
};

class PassDeathNVRC : public FFX::Pass {
    public:
    /* 0030 */ CGxShader* m_shader;

    virtual ~PassDeathNVRC();
    virtual bool Valid();
    virtual void Execute();

    PassDeathNVRC(FFX::Tex** inputs, uint32_t count, FFX::Tex* output, EGxBlend blend, bool unk);
};

#endif
