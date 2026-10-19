#ifndef _FMOD_CHANNEL_DSOUND_EAX2_H
#define _FMOD_CHANNEL_DSOUND_EAX2_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_EAX

#include "fmod_channel_dsound.h"

namespace FMOD
{
    class ChannelDSoundEAX2 : public ChannelDSound
    {   
      public:
        
        // OFFSET: 0x0093AA80
        ChannelDSoundEAX2() {};

        FMOD_RESULT  setReverbProperties  (const FMOD_REVERB_CHANNELPROPERTIES *prop);
        FMOD_RESULT  getReverbProperties  (FMOD_REVERB_CHANNELPROPERTIES *prop);
    };
}

#endif /* FMOD_SUPPORT_EAX */

#endif
