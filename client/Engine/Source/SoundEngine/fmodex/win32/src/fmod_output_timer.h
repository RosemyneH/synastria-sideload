#ifndef _FMOD_OUTPUT_TIMER_H
#define _FMOD_OUTPUT_TIMER_H

#include "fmod_settings.h"

#include "fmod_outputi.h"
#include "fmod_codec_wav.h"

namespace FMOD
{
    class OutputTimer : public Output
    {
      private:
        
        float                     mMixerTimerPeriod;
        unsigned int              mMixerTimerID;
        bool                      mPlaying;
        FMOD_OS_CRITICALSECTION  *mMixerCrit;

        FMOD_RESULT               timerFunc();
        
        static void CALLBACK      timerFuncCallback(UINT uTimerID, UINT uMsg, UINT_PTR dwUser, UINT_PTR dw1, UINT_PTR dw2);

      protected:
        
        int                       mFillBlock;

      public:
        
        FMOD_RESULT               start();
        FMOD_RESULT               stop();
    };
}

#endif

