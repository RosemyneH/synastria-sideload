#ifndef WOW_SOURCE_WORLDCLIENT_LIQUIDENVIRONMENT_H
#define WOW_SOURCE_WORLDCLIENT_LIQUIDENVIRONMENT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Base/CDataAllocator.h"
#include "WoW/Source/LiquidSystem/Liquid.h"

class CM2Lighting;

namespace Liquid {
    class CClientEnvironment : public IEnvironment {
        public:
        static CClientEnvironment* Create(int32_t customLight);

        /* 0004 */ int32_t m_interiorFog;
        /* 0008 */ int32_t m_customLight;

        virtual void Release();
        virtual void SetInteriorFog(int32_t interiorFog);
        virtual void SelectLights(CM2Lighting* lighting);
    };

    extern CDataAllocator s_clientEnvironmentAllocator;

    void ClearClientEnvironments();
}

#endif
