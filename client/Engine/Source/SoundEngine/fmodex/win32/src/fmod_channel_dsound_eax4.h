#ifndef _FMOD_CHANNEL_DSOUND_EAX4_H
#define _FMOD_CHANNEL_DSOUND_EAX4_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_EAX

#include "fmod_channel_dsound.h"

namespace FMOD
{
    class ChannelDSoundEAX4 : public ChannelDSound
    {
      public:
        
        // OFFSET: 0x0093AAC0
        ChannelDSoundEAX4() { };

        FMOD_RESULT  setReverbProperties  (const FMOD_REVERB_CHANNELPROPERTIES *prop);
        FMOD_RESULT  getReverbProperties  (FMOD_REVERB_CHANNELPROPERTIES *prop);
    };
}

#endif /* FMOD_SUPPORT_EAX */

#endif
