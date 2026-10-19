#ifndef _FMOD_CHANNELPOOL_H
#define _FMOD_CHANNELPOOL_H

#include "fmod_settings.h"
#include "fmod.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    class ChannelReal;
    class DSPI;
    class Output;
    class Sound;
    class SystemI;

    class ChannelPool
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

        friend class ChannelReal;

      protected:
        
        int             mNumChannels;
        int             mChannelsUsed;
        SystemI        *mSystem;
        Output         *mOutput;
       
      public:

        ChannelReal   **mChannel;
       
        ChannelPool();

        FMOD_RESULT     init(SystemI *system, Output *output, int numchannels);
        FMOD_RESULT     release();
        FMOD_RESULT     find(int id, Sound *sound, ChannelReal **channel, int excludeid, bool stopstreamable);
        FMOD_RESULT     allocateChannel(ChannelReal **realchannel, int index, int numchannels, int *found);
        FMOD_RESULT     getNumChannels(int *numchannels);
	    FMOD_RESULT     getChannelsUsed(int *numchannels);
        FMOD_RESULT     setChannel(int index, ChannelReal *channel, DSPI *dspmixtarget = 0);
        FMOD_RESULT     getChannel(int index, ChannelReal **channel);
    };
}

#endif
