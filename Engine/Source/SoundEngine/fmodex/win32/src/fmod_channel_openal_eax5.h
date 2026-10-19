#ifndef _FMOD_CHANNEL_OPENAL_EAX5_H
#define _FMOD_CHANNEL_OPENAL_EAX5_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL
#ifdef FMOD_SUPPORT_EAX

#include "fmod_channel_openal.h"

namespace FMOD
{
    class ChannelOpenALEAX5 : public ChannelOpenAL
    {
      public:
        
        // OFFSET: 0x0092A320
        ChannelOpenALEAX5() {};

		FMOD_RESULT	 setupChannel         ();

		FMOD_RESULT  setSpeakerMix        (float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright);
        FMOD_RESULT  setSpeakerLevels     (int speaker, float *levels, int numlevels);

        FMOD_RESULT  setReverbProperties  (const FMOD_REVERB_CHANNELPROPERTIES *prop);
        FMOD_RESULT  getReverbProperties  (FMOD_REVERB_CHANNELPROPERTIES *prop);
    };
}

#endif /* FMOD_SUPPORT_EAX */
#endif /* FMOD_SUPPORT_OPENAL */

#endif /* _FMOD_CHANNEL_OPENAL_EAX5_H */
