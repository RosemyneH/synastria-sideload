#ifndef _FMOD_OUTPUTI_H
#define _FMOD_OUTPUTI_H

#include "fmod_settings.h"

#include "fmod_channelpool.h"
#include "fmod_channelgroupi.h"
#include "fmod_dsp_resampler.h"
#include "fmod_output.h"
#include "fmod_linkedlist.h"
#include "fmod_os_misc.h"
#include "fmod_plugin.h"

#define FMOD_OUTPUT_MAXDRIVERS  32

namespace FMOD
{
    class Sample;
    class MemoryTracker;

    static const FMOD_OUTPUTTYPE FMOD_OUTPUTTYPE_EMULATED = (FMOD_OUTPUTTYPE)-1;
    static const FMOD_OUTPUTTYPE FMOD_OUTPUTTYPE_SOFTWARE = (FMOD_OUTPUTTYPE)-2;

    typedef int         (F_CALLBACK *FMOD_OUTPUT_GETSAMPLEMAXCHANNELS)       (FMOD_OUTPUT_STATE *output, FMOD_MODE mode, FMOD_SOUND_FORMAT format); 
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_GETDRIVERCAPSEXCALLBACK)    (FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_GETDRIVERCAPSEX2CALLBACK)   (FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode, int *num2dchannels, int *num3dchannels, int *totalchannels);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_INITEXCALLBACK)             (FMOD_OUTPUT_STATE *output, int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, FMOD_SPEAKERMODE *speakermode, int dspbufferlength, int dspnumbuffers, int max2dchannels, int max3dchannels, void *extradriverdata);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_STARTCALLBACK)              (FMOD_OUTPUT_STATE *output);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_STOPCALLBACK)               (FMOD_OUTPUT_STATE *output);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_UPDATEFINISHEDCALLBACK)     (FMOD_OUTPUT_STATE *output);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_CREATESAMPLECALLBACK)       (FMOD_OUTPUT_STATE *output, FMOD_MODE mode, FMOD_CODEC_WAVEFORMAT *waveformat, Sample **sample);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_GETSOUNDRAMCALLBACK)        (FMOD_OUTPUT_STATE *output, int *currentalloced, int *maxalloced, int *total);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_RECORDGETDRIVERCAPSCALLBACK)(FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_RECORDSTARTCALLBACK)        (FMOD_OUTPUT_STATE *output, int id, FMOD_SOUND *sound, int loop);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_RECORDSTOPCALLBACK)         (FMOD_OUTPUT_STATE *output);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_RECORDGETPOSITIONCALLBACK)  (FMOD_OUTPUT_STATE *output, unsigned int *pcm);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_RECORDLOCKCALLBACK)         (FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_RECORDUNLOCKCALLBACK)       (FMOD_OUTPUT_STATE *output, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);    
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_SETREVERBCALLBACK)          (FMOD_OUTPUT_STATE *output, const FMOD_REVERB_PROPERTIES *prop);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_GETREVERBCALLBACK)          (FMOD_OUTPUT_STATE *output, FMOD_REVERB_PROPERTIES *prop);
    typedef FMOD_RESULT (F_CALLBACK *FMOD_OUTPUT_POSTMIXCALLBACK)            (FMOD_OUTPUT_STATE *output);
    


    struct FMOD_OUTPUT_DESCRIPTION_EX : public FMOD_OUTPUT_DESCRIPTION, public LinkedListNode
    {
        FMOD_OUTPUTTYPE  mType;        
        int              mSize;
        FMOD_OS_LIBRARY *mModule;

        FMOD_OUTPUT_GETSAMPLEMAXCHANNELS        getsamplemaxchannels;
        FMOD_OUTPUT_GETDRIVERCAPSEXCALLBACK     getdrivercapsex;
        FMOD_OUTPUT_GETDRIVERCAPSEX2CALLBACK    getdrivercapsex2;
        FMOD_OUTPUT_INITEXCALLBACK              initex;
        FMOD_OUTPUT_STARTCALLBACK               start;                 /* [in] Initialization function for the output device to start accepting audio data from the FMOD software mixer.  This is called from System::init. */
        FMOD_OUTPUT_STOPCALLBACK                stop;                  /* [in] Initialization function for the output device to stop accepting audio data from FMOD the software mixer.  This is called from System::close. */
        FMOD_OUTPUT_UPDATEFINISHEDCALLBACK      updatefinished;        /* [in] Update finished function.  PS2 only. */
        FMOD_OUTPUT_CREATESAMPLECALLBACK        createsample;
        FMOD_OUTPUT_GETSOUNDRAMCALLBACK         getsoundram;           /* [in] For information about sound ram usage. This callback is to give System::getSoundRam somethign to return */
		FMOD_OUTPUT_POSTMIXCALLBACK             postmixcallback;       /* [in] Called within mixer update, after the mix has happened, and within SystemI::mDSPCrit. */
        FMOD_OUTPUT_GETNUMDRIVERSCALLBACK       record_getnumdrivers;  /* [in] For recording device enumeration.  This callback is to give System::getRecordNumDrivers somthing to return. */
        FMOD_OUTPUT_GETDRIVERNAMECALLBACK       record_getdrivername;  /* [in] For recording device enumeration.  This callback is to give System::getRecordDriverName somthing to return. */
        FMOD_OUTPUT_RECORDGETDRIVERCAPSCALLBACK record_getdrivercaps;  /* [in] For recording device enumeration.  This callback is to give System::getRecordDriverCaps somthing to return. */
        FMOD_OUTPUT_RECORDSTARTCALLBACK         record_start;          /* [in] Initialization function for the recording device to start accepting audio data from the FMOD software mixer.  This is called from System::recordStart. */
        FMOD_OUTPUT_RECORDSTOPCALLBACK          record_stop;           /* [in] Initialization function for the recording device to stop accepting audio data from FMOD the software mixer.  This is called from System::recordStop. */
        FMOD_OUTPUT_RECORDGETPOSITIONCALLBACK   record_getposition;    /* [in] This is called from the FMOD software mixer thread.  This returns a position value in samples so that FMOD knows where and when to fill its record buffer. */
        FMOD_OUTPUT_RECORDLOCKCALLBACK          record_lock;           /* [in] This is called from the FMOD software mixer thread.  This function provides a pointer to data that FMOD can read from when updating its internal record buffer. */
        FMOD_OUTPUT_RECORDUNLOCKCALLBACK        record_unlock;         /* [in] This is called from the FMOD software mixer thread.  This optional function accepts the data that has been read does whatever post read operation nescessary. */
        FMOD_OUTPUT_SETREVERBCALLBACK           reverb_setproperties;  /* [in] This is called from System::setReverbProperties. */
    };

    class ChannelPool;
    class ChannelReal;
    class SoundI;

    class Output : public Plugin, public FMOD_OUTPUT_STATE
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

        friend class SystemI;
        friend class PluginFactory;
        friend class SoundI;
        friend class ChannelGroupI;

      public:

        ChannelPool             *mChannelPool;

      protected:

        bool                     mEnumerated;
        bool                     mPolling;
        SystemI                 *mSystem;
        ChannelPool             *mChannelPool3D;
        int                      mMixAheadBlocks;
        ChannelGroupI           *mMusicChannelGroup;

        int                      mNum2DChannelsFromCaps;
        int                      mNum3DChannelsFromCaps;
        int                      mTotalChannelsFromCaps;

#ifdef FMOD_SUPPORT_RECORDING
        bool                     mRecording;
        int                      mRecordOffset;
        bool                     mRecordLoop;
        FMOD_SOUND_FORMAT        mRecordFormat;
        unsigned int             mRecordLastCursorPos;
        unsigned int             mRecordBufferLength;
        SoundI                  *mRecordSound;
        int                      mRecordRate;
        bool                     mRecordResample;
        DSPResampler             mRecordResamplerDSP;
        float                   *mRecordTempBuffer;
        unsigned int             mRecordTempBufferLength;

        FMOD_RESULT              recordRead(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
 static FMOD_RESULT F_CALLBACK   recordResamplerReadCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        FMOD_RESULT              recordFill(unsigned int length);
        FMOD_RESULT              recordUpdate();
#endif

      public:

        FMOD_OUTPUT_DESCRIPTION_EX  mDescription;

        Output();

        virtual FMOD_RESULT release();

#ifdef FMOD_SUPPORT_SOFTWARE
        FMOD_RESULT         mix(void *buffer, unsigned int numsamples);
#endif

        virtual FMOD_RESULT getFreeChannel(FMOD_MODE mode, ChannelReal **realchannel, int numchannels, int numsoundchannels, int *found);

#ifdef FMOD_SUPPORT_SOFTWARE
        static FMOD_RESULT F_CALLBACK mixCallback(FMOD_OUTPUT_STATE *output, void *buffer, unsigned int length);
#endif
    };
}

#endif

