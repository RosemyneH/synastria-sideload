#ifndef _FMOD_OUTPUT_OPENAL_H
#define _FMOD_OUTPUT_OPENAL_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL

#include "fmod_systemi.h"
#include "fmod_output_timer.h"
#include "fmod_output_dsound.h"

#include "../../lib/openal/utils/LoadOAL.h"

namespace FMOD
{
    typedef ALenum (*EAXSet)(const GUID*, ALuint, ALuint, ALvoid*, ALuint);
    typedef ALenum (*EAXGet)(const GUID*, ALuint, ALuint, ALvoid*, ALuint);
    
    typedef struct
	{
		bool	used;
		ALuint	sid;
		ALuint *bid;
	} SourceOpenAL;

    class OutputOpenAL : public OutputTimer
    {
        friend class SampleOpenAL;
        friend class ChannelOpenAL;
        friend class ChannelOpenALEAX2;
        friend class ChannelOpenALEAX3;
        friend class ChannelOpenALEAX4;
        friend class ChannelOpenALEAX5;

      private:
        
        OPENALFNTABLE      mOALFnTable;
        bool               mSetupOnce;
        bool               mInitialised;
        bool               mDLLInitialised;

        ALCdevice         *mDevice;
	    ALCcontext        *mContext;        
        int                mNumDrivers;
        char              *mDriverNames[FMOD_OUTPUT_MAXDRIVERS];
        int                mNumBuffers;
        unsigned int       mBufferLength;
        unsigned int       mBufferLengthBytes;
        unsigned int	   mPcmBase;
        char			  *mBufferData;
        int                mNumChannels;
        ChannelOpenAL     *mChannels;
        int	               mNumSources;
        SourceOpenAL	  *mSources;
        unsigned int       mMixerSourceOffset;
        
        int                mRate;
        FMOD_SOUND_FORMAT  mFormat;
        ALuint			   mFormatOAL;
        int                mNumHwChannels;
        int                mOutputChannels;
        REVERB_VERSION     mReverbVersion;
        bool               mMixerReverbDisabled;
        
        EAXSet             mEAXSet;
        EAXGet             mEAXGet;
        // Private methods
        FMOD_RESULT  registerDLL          ();
        FMOD_RESULT  enumerate            ();
        FMOD_RESULT  initEAX              ();
        FMOD_RESULT	 setupSessionEAX5     ();
        FMOD_RESULT  getSpeakerModeEAX5   (FMOD_SPEAKERMODE *speakerMode);
        FMOD_RESULT  lockSlotEAX4         (GUID &slotGUID);
        FMOD_RESULT  lockSlotEAX5         (GUID &slotGUID);
        FMOD_RESULT  setPropertiesEAX2    (const FMOD_REVERB_PROPERTIES *prop);
        FMOD_RESULT  setPropertiesEAX3    (const FMOD_REVERB_PROPERTIES *prop);
        FMOD_RESULT  setPropertiesEAX4    (const FMOD_REVERB_PROPERTIES *prop);
        FMOD_RESULT  setPropertiesEAX5    (const FMOD_REVERB_PROPERTIES *prop);

      public:                      
                                   
        OutputOpenAL();
        static FMOD_OUTPUT_DESCRIPTION_EX *getDescriptionEx();  

        // Public methods
        FMOD_RESULT	 getFreeChannel       (FMOD_MODE mode, ChannelReal **realchannel, int numchannels, int numsoundchannels, int *found);
        FMOD_RESULT  getNumDrivers        (int *numdrivers);
        FMOD_RESULT  getDriverName        (int id, char *name, int namelen);
        FMOD_RESULT  getDriverCapsEx      (int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode, int *num2dchannels, int *num3dchannels, int *totalchannels);
	    FMOD_RESULT  init                 (int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata);
	    FMOD_RESULT  close                ();
        FMOD_RESULT  start                ();
        FMOD_RESULT  stop                 ();                                               
        FMOD_RESULT  getHandle            (void **handle);
	    FMOD_RESULT  update               ();
        FMOD_RESULT	 postMix              ();
	    FMOD_RESULT  getPosition          (unsigned int *pcm);
	    FMOD_RESULT  lock                 (unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
	    FMOD_RESULT  unlock               (void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);
        FMOD_RESULT  createSample         (FMOD_MODE mode, FMOD_CODEC_WAVEFORMAT *waveformat, Sample **sample);
	    FMOD_RESULT  setReverbProperties  (const FMOD_REVERB_PROPERTIES *prop);
        int          getSampleMaxChannels (FMOD_MODE mode, FMOD_SOUND_FORMAT format);

        // Public callbacks
        static FMOD_RESULT F_CALLBACK getNumDriversCallback        (FMOD_OUTPUT_STATE *output, int *numdrivers);
        static FMOD_RESULT F_CALLBACK getDriverNameCallback        (FMOD_OUTPUT_STATE *output, int id, char *name, int namelen);
        static FMOD_RESULT F_CALLBACK getDriverCapsExCallback      (FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode, int *num2dchannels, int *num3dchannels, int *totalchannels);
        static FMOD_RESULT F_CALLBACK initCallback                 (FMOD_OUTPUT_STATE *output, int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata);
        static FMOD_RESULT F_CALLBACK closeCallback                (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK startCallback                (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK stopCallback                 (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK getHandleCallback            (FMOD_OUTPUT_STATE *output, void **handle);
        static FMOD_RESULT F_CALLBACK updateCallback               (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK postMixCallback              (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK getPositionCallback          (FMOD_OUTPUT_STATE *output, unsigned int *pcm);
        static FMOD_RESULT F_CALLBACK lockCallback                 (FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
        static FMOD_RESULT F_CALLBACK unlockCallback               (FMOD_OUTPUT_STATE *output, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);
        static FMOD_RESULT F_CALLBACK createSampleCallback         (FMOD_OUTPUT_STATE *output, FMOD_MODE mode, FMOD_CODEC_WAVEFORMAT *waveformat, Sample **sample);
        static FMOD_RESULT F_CALLBACK setReverbPropertiesCallback  (FMOD_OUTPUT_STATE *output, const FMOD_REVERB_PROPERTIES *prop);
        static int         F_CALLBACK getSampleMaxChannelsCallback (FMOD_OUTPUT_STATE *output, FMOD_MODE mode, FMOD_SOUND_FORMAT format);
    };
}

#endif  /* #ifdef FMOD_SUPPORT_OPENAL */

#endif  /* #ifndef _FMOD_OUTPUT_OPENAL_H */