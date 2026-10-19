#ifndef _FMOD_CHANNEL_REALMANUAL3D_H
#define _FMOD_CHANNEL_REALMANUAL3D_H

#include "fmod_settings.h"

#include "fmod_channel_real.h"

namespace FMOD
{
    class ChannelRealManual3D : public ChannelReal
    {
        friend class ChannelSoftware;

        float           mAngleToListener;

      public:

        ChannelRealManual3D();

        FMOD_RESULT     set2DFreqVolumePanFor3D();
    };
}

#endif
