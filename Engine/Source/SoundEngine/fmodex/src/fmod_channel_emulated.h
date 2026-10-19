#ifndef _FMOD_CHANNEL_EMULATED_H
#define _FMOD_CHANNEL_EMULATED_H

#include "fmod_settings.h"

#include "fmod_channel_real.h"
#include "fmod_dsp_filter.h"

namespace FMOD
{
    class ChannelEmulated : public ChannelReal
    {
      private:

#ifdef FMOD_SUPPORT_SOFTWARE
        DSPFilter  mDSPHeadMemory;
        DSPI      *mDSPHead;
#endif

      public:

        ChannelEmulated();

        FMOD_RESULT  init(int index, SystemI *system, Output *output, DSPI *dspmixtarget);
        FMOD_RESULT  close();
        FMOD_RESULT  alloc();
        FMOD_RESULT  update(int delta);
        FMOD_RESULT  stop(bool ignoreenddelay, bool resetstate);
        FMOD_RESULT  stop(bool ignoreenddelay, bool resetstate, bool force);
        FMOD_RESULT  setSpeakerLevels(int speaker, float *levels, int numlevels);
        FMOD_RESULT  isVirtual(bool *isvirtual);
        FMOD_RESULT  getDSPHead(DSPI **dsp);
    };
}

#endif

