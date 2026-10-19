#ifndef WOW_SOURCE_GAME_GAMECLIENT_PASSPROPAGATEFOG_H
#define WOW_SOURCE_GAME_GAMECLIENT_PASSPROPAGATEFOG_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/FFX/FFX.h"
#include "Engine/Source/Tempest/C2Vector.h"

class CGxShader;

class PassPropagateFog : public FFX::Pass {
    public:
    /* 0030 */ uint32_t m_useRect;
    /* 0034 */ CGxShader* m_pixelShaders[2];
    /* 003C */ C2Vector m_params[4];
    /* 005C */ float unk_005C;

    ~PassPropagateFog() override;
    bool Valid() override;
    void Execute() override;

    PassPropagateFog(FFX::Tex** inputs, uint32_t count, FFX::Tex* output, const C2Vector* params, EGxBlend blend, bool unk);
};

#endif
