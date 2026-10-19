#ifndef _FMOD_OUTPUT_DSOUND_H
#define _FMOD_OUTPUT_DSOUND_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_DSOUND

//#define FMOD_OUTPUT_DSOUND_USETIMER

#ifdef FMOD_OUTPUT_DSOUND_USETIMER
    #include "fmod_output_timer.h"
#else
    #include "fmod_output_polled.h"
#endif
#include "fmod_os_misc.h"

#ifdef PLATFORM_32BIT
    typedef unsigned long DWORD;
    typedef DWORD DWORD_PTR;
#endif

#include <dsound.h>

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

/*
    Directsound function pointers definitions.
*/
typedef HRESULT (WINAPI * PFN_DSCREATE)		(LPGUID lpguid, LPDIRECTSOUND * ppDS, IUnknown FAR * pUnkOuter);
typedef BOOL	(WINAPI * PFN_DSENUMERATE)	(LPDSENUMCALLBACKA lpDSEnumCallback, LPVOID lpContext);
typedef HRESULT (WINAPI * PFN_DSCREATE8)	(LPCGUID lpcGuidDevice, LPDIRECTSOUND8 * ppDS8, LPUNKNOWN pUnkOuter);
typedef HRESULT (WINAPI * PFN_DSCCREATE)	(LPGUID lpGUID, LPDIRECTSOUNDCAPTURE *lplpDSC,LPUNKNOWN pUnkOuter);
typedef BOOL	(WINAPI * PFN_DSCENUMERATE)	(LPDSENUMCALLBACKA lpDSEnumCallback,LPVOID lpContext);
typedef HRESULT (WINAPI * PFN_DSCCREATE8)	(LPCGUID lpcGUID, LPDIRECTSOUNDCAPTURE8 *lplpDSC, LPUNKNOWN pUnkOuter);

static const FMOD_GUID FMOD_DSPROPSETID_I3DL2_ListenerProperties = { 0xDA0F0520, 0x300A, 0x11D3, { 0x8A, 0x2B, 0x00, 0x60, 0x97, 0x0D, 0xB0, 0x11 } };
static const FMOD_GUID FMOD_DSPROPSETID_I3DL2_BufferProperties   = { 0xDA0F0521, 0x300A, 0x11D3, { 0x8A, 0x2B, 0x00, 0x60, 0x97, 0x0D, 0xB0, 0x11 } };
static const FMOD_GUID FMOD_DSPROPSETID_EAX20_BufferProperties   = { 0x306a6a7,  0xb224, 0x11d2, { 0x99, 0xe5, 0x0,  0x0,  0xe8, 0xd8, 0xc7, 0x22 } };
static const FMOD_GUID FMOD_DSPROPSETID_EAX20_ListenerProperties = { 0x306a6a8,  0xb224, 0x11d2, { 0x99, 0xe5, 0x0,  0x0,  0xe8, 0xd8, 0xc7, 0x22 } };
static const FMOD_GUID FMOD_DSPROPSETID_EAX30_ListenerProperties = { 0xa8fa6882, 0xb476, 0x11d3, { 0xbd, 0xb9, 0x00, 0xc0, 0xf0, 0x2d, 0xdf, 0x87 } };
static const FMOD_GUID FMOD_DSPROPSETID_EAX30_BufferProperties   = { 0xa8fa6881, 0xb476, 0x11d3, { 0xbd, 0xb9, 0x0,  0xc0, 0xf0, 0x2d, 0xdf, 0x87 } };
static const FMOD_GUID FMOD_EAXPROPERTYID_EAX40_Context          = { 0x1d4870ad, 0xdef,  0x43c0, { 0xa4, 0xc,  0x52, 0x36, 0x32, 0x29, 0x63, 0x42 } };
static const FMOD_GUID FMOD_EAXPROPERTYID_EAX40_Source           = { 0x1b86b823, 0x22df, 0x4eae, { 0x8b, 0x3c, 0x12, 0x78, 0xce, 0x54, 0x42, 0x27 } };
static const FMOD_GUID FMOD_EAXPROPERTYID_EAX40_FXSlot0          = { 0xc4d79f1e, 0xf1ac, 0x436b, { 0xa8, 0x1d, 0xa7, 0x38, 0xe7, 0x4,  0x54, 0x69 } };
static const FMOD_GUID FMOD_EAXPROPERTYID_EAX40_FXSlot1          = { 0x8c00e96,  0x74be, 0x4491, { 0x93, 0xaa, 0xe8, 0xad, 0x35, 0xa4, 0x91, 0x17 } };
static const FMOD_GUID FMOD_EAXPROPERTYID_EAX40_FXSlot2          = { 0x1d433b88, 0xf0f6, 0x4637, { 0x91, 0x9f, 0x60, 0xe7, 0xe0, 0x6b, 0x5e, 0xdd } };
static const FMOD_GUID FMOD_EAXPROPERTYID_EAX40_FXSlot3          = { 0xefff08ea, 0xc7d8, 0x44ab, { 0x93, 0xad, 0x6d, 0xbd, 0x5f, 0x91, 0x0,  0x64 } };
static const FMOD_GUID FMOD_EAXPROPERTYID_EAX50_Context			 = { 0x57e13437, 0xb932, 0x4ab2, { 0xb8, 0xbd, 0x52, 0x66, 0xc1, 0xa8, 0x87, 0xee } };
static const FMOD_GUID FMOD_EAXPROPERTYID_EAX50_Source			 = { 0x5edf82f0, 0x24a7, 0x4f38, { 0x8e, 0x64, 0x2f, 0x09, 0xca, 0x05, 0xde, 0xe1 } };
static const FMOD_GUID FMOD_EAXPROPERTYID_EAX50_FXSlot0			 = { 0x91f9590f, 0xc388, 0x407a, { 0x84, 0xb0, 0x1b, 0xae, 0x0e, 0xf7, 0x1a, 0xbc } };
static const FMOD_GUID FMOD_EAXPROPERTYID_EAX50_FXSlot1			 = { 0x8f5f7aca, 0x9608, 0x4965, { 0x81, 0x37, 0x82, 0x13, 0xc7, 0xb9, 0xd9, 0xde } };
static const FMOD_GUID FMOD_EAXPROPERTYID_EAX50_FXSlot2			 = { 0x3c0f5252, 0x9834, 0x46f0, { 0xa1, 0xd8, 0x5b, 0x95, 0xc4, 0xa0, 0x0a, 0x30 } };
static const FMOD_GUID FMOD_EAXPROPERTYID_EAX50_FXSlot3			 = { 0xe2eb0eaa, 0xe806, 0x45e7, { 0x9f, 0x86, 0x06, 0xc1, 0x57, 0x1a, 0x6f, 0xa3 } };
static const FMOD_GUID FMOD_EAX_NULL_GUID                        = { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
static const FMOD_GUID FMOD_EAX_REVERB_EFFECT                    = { 0xcf95c8f,  0xa3cc, 0x4849, { 0xb0, 0xb6, 0x83, 0x2e, 0xcc, 0x18, 0x22, 0xdf } };
static const FMOD_GUID FMOD_IID_IKsPropertySet                   = { 0x31efac30, 0x515c, 0x11d0, { 0xa9, 0xaa, 0x00, 0xaa, 0x00, 0x61, 0xbe, 0x93 } };


namespace FMOD
{
    class ChannelDSound;
    class SampleDSound;
    class Sound;

    typedef enum
    {
        REVERB_VERSION_NONE,
        REVERB_VERSION_EAX5,
        REVERB_VERSION_EAX4,
        REVERB_VERSION_EAX3,
        REVERB_VERSION_EAX2,
        REVERB_VERSION_I3DL2
    } REVERB_VERSION;

#ifdef FMOD_OUTPUT_DSOUND_USETIMER
    class OutputDSound : public OutputTimer
#else
    class OutputDSound : public OutputPolled
#endif
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

        friend class SampleDSound;
        friend class ChannelDSound;
        friend class ChannelDSoundEAX2;
        friend class ChannelDSoundEAX3;
        friend class ChannelDSoundEAX4;
        friend class ChannelDSoundI3DL2;

      private:

        bool                        mDLLInitialized;
        bool                        mCoInitialized;
        bool                        mGlobalFocus;
        bool                        mNeedToCommit;
        FMOD_INITFLAGS              mInitFlags;
                                   
        int                         mDirectXVersion;
        HMODULE                     mDSoundModule;
        HMODULE                     mDSound3DModule;

        PFN_DSCREATE8               mDirectSoundCreate;
        PFN_DSENUMERATE             mDirectSoundEnumerate;
        IDirectSound8              *mDirectSound;
        IDirectSound3DListener     *mDirectSoundListener;

        PFN_DSCENUMERATE	        mDirectSoundCaptureEnumerate;
        PFN_DSCCREATE8              mDirectSoundCaptureCreate;
#ifdef FMOD_SUPPORT_RECORDING
        IDirectSoundCapture8       *mDirectSoundCapture;
        IDirectSoundCaptureBuffer  *mDirectSoundCaptureBuffer;
#endif

        REVERB_VERSION              mReverbVersion;
        unsigned int                mFeaturesReverb;
        IKsPropertySet             *mBufferReverb;
        SampleDSound               *mSampleReverb;
                                   
        bool                        mUseSoftware2DBuffers;
        bool                        mUseSoftware3DBuffers;
        float                       mMinFrequency;
        float                       mMaxFrequency;
        unsigned int                mBufferLength;
        unsigned int                mNumBuffers;

        float                       mLastDopplerScale;
        float                       mLastDistancerScale;
        float                       mLastRolloffrScale;

        FMOD_SPEAKERMODE            mSpeakerMode;
                                   
        ChannelDSound              *mChannel2D;
        ChannelDSound              *mChannel3D;
        Sample                     *mSample;

        unsigned int                mBufferMemoryCurrent;
        unsigned int                mBufferMemoryMax;
                                   
        FMOD_RESULT                 registerDLL();

        bool                        initI3DL2();
        FMOD_RESULT                 closeI3DL2();
        FMOD_RESULT                 setPropertiesI3DL2(const FMOD_REVERB_PROPERTIES *prop);
        FMOD_RESULT                 getPropertiesI3DL2(FMOD_REVERB_PROPERTIES *prop);
        bool                        querySupportI3DL2(unsigned int ulQuery);

        bool                        initEAX2();
        FMOD_RESULT                 closeEAX2();
        FMOD_RESULT                 setPropertiesEAX2(const FMOD_REVERB_PROPERTIES *prop);
        FMOD_RESULT	                getPropertiesEAX2(FMOD_REVERB_PROPERTIES *prop);
        bool                        querySupportEAX2(unsigned int ulQuery);

        bool                        initEAX3();
        FMOD_RESULT                 closeEAX3();
        FMOD_RESULT                 setPropertiesEAX3(const FMOD_REVERB_PROPERTIES *prop);
        FMOD_RESULT	                getPropertiesEAX3(FMOD_REVERB_PROPERTIES *prop);
        bool                        querySupportEAX3(unsigned int ulQuery);

        bool                        initEAX4();
        FMOD_RESULT                 closeEAX4();
        FMOD_RESULT                 setPropertiesEAX4(const FMOD_REVERB_PROPERTIES *prop);
        FMOD_RESULT                 getPropertiesEAX4(FMOD_REVERB_PROPERTIES *prop);
        bool                        querySupportEAX4(FMOD_GUID guid, unsigned long ulProperty);

                                 
      public:                      
                                  
        int                         mNumDrivers;                            /* Public for C callback access */
        char                       *mDriverName[FMOD_OUTPUT_MAXDRIVERS];    /* Public for C callback access */
        GUID                        mGUID[FMOD_OUTPUT_MAXDRIVERS];

        static FMOD_OUTPUT_DESCRIPTION_EX *getDescriptionEx();  

        FMOD_RESULT              enumerate();
        FMOD_RESULT              getNumDrivers(int *numdrivers);
        FMOD_RESULT              getDriverName(int id, char *name, int namelen);
        FMOD_RESULT              getDriverCapsEx(int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode, int *num2dchannels, int *num3dchannels, int *totalchannels);
	    FMOD_RESULT              init(int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, FMOD_SPEAKERMODE *speakermode, int dspbufferlength, int dspnumbuffers, int max2dchannels, int max3dchannels, void *extradriverdata);
	    FMOD_RESULT              close();
        FMOD_RESULT              getHandle(void **handle);
	    FMOD_RESULT              update();
        FMOD_RESULT              createSample(FMOD_MODE mode, FMOD_CODEC_WAVEFORMAT *waveformat, Sample **sample);
        FMOD_RESULT              getSoundRAM(int *currentalloced, int *maxalloced, int *total);
	    FMOD_RESULT              setReverbProperties(const FMOD_REVERB_PROPERTIES *prop);
	    FMOD_RESULT              getReverbProperties(FMOD_REVERB_PROPERTIES *prop);
        FMOD_RESULT              start();
        FMOD_RESULT              stop();                                       
	    FMOD_RESULT              getPosition(unsigned int *pcm);
	    FMOD_RESULT              lock(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
	    FMOD_RESULT              unlock(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);
        int                      getSampleMaxChannels(FMOD_MODE mode, FMOD_SOUND_FORMAT format);

#ifdef FMOD_SUPPORT_RECORDING
        int                      mRecordNumDrivers;                            /* Public for C callback access */
        char                    *mRecordDriverName[FMOD_OUTPUT_MAXDRIVERS];    /* Public for C callback access */
        GUID                     mRecordGUID[FMOD_OUTPUT_MAXDRIVERS];

        FMOD_RESULT              recordEnumerate();
        FMOD_RESULT              recordGetNumDrivers(int *numdrivers);
        FMOD_RESULT              recordGetDriverInfo(int id, char *name, int namelen);
        FMOD_RESULT              recordStart        (int id, Sound *sound, bool loop);
        FMOD_RESULT              recordStop         ();
        FMOD_RESULT              recordGetPosition  (unsigned int *pcm);
        FMOD_RESULT              recordLock         (unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
        FMOD_RESULT              recordUnlock       (void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);
#endif

        static FMOD_RESULT F_CALLBACK getNumDriversCallback          (FMOD_OUTPUT_STATE *output, int *numdrivers);
        static FMOD_RESULT F_CALLBACK getDriverNameCallback          (FMOD_OUTPUT_STATE *output, int id, char *name, int namelen);
        static FMOD_RESULT F_CALLBACK getDriverCapsExCallback        (FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode, int *num2dchannels, int *num3dchannels, int *totalchannels);
        static FMOD_RESULT F_CALLBACK initExCallback                 (FMOD_OUTPUT_STATE *output, int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, FMOD_SPEAKERMODE *speakermode, int dspbufferlength, int dspnumbuffers, int max2dchannels, int max3dchannels, void *extradriverdata);
        static FMOD_RESULT F_CALLBACK closeCallback                  (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK startCallback                  (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK stopCallback                   (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK updateCallback                 (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK getHandleCallback              (FMOD_OUTPUT_STATE *output, void **handle);
        static FMOD_RESULT F_CALLBACK getPositionCallback            (FMOD_OUTPUT_STATE *output, unsigned int *pcm);
        static FMOD_RESULT F_CALLBACK lockCallback                   (FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
        static FMOD_RESULT F_CALLBACK unlockCallback                 (FMOD_OUTPUT_STATE *output, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);
        static FMOD_RESULT F_CALLBACK createSampleCallback           (FMOD_OUTPUT_STATE *output, FMOD_MODE mode, FMOD_CODEC_WAVEFORMAT *waveformat, Sample **sample);
        static FMOD_RESULT F_CALLBACK getSoundRAMCallback            (FMOD_OUTPUT_STATE *output, int *currentalloced, int *maxalloced, int *total);
        static FMOD_RESULT F_CALLBACK setReverbPropertiesCallback    (FMOD_OUTPUT_STATE *output, const FMOD_REVERB_PROPERTIES *prop);
        static int         F_CALLBACK getSampleMaxChannelsCallback   (FMOD_OUTPUT_STATE *output, FMOD_MODE mode, FMOD_SOUND_FORMAT format);

#ifdef FMOD_SUPPORT_RECORDING
        static FMOD_RESULT F_CALLBACK recordGetNumDriversCallback    (FMOD_OUTPUT_STATE *output, int *numdrivers);
        static FMOD_RESULT F_CALLBACK recordGetDriverInfoCallback    (FMOD_OUTPUT_STATE *output, int id, char *name, int namelen);
        static FMOD_RESULT F_CALLBACK recordStartCallback            (FMOD_OUTPUT_STATE *output, int id, FMOD_SOUND *sound, int loop);
        static FMOD_RESULT F_CALLBACK recordStopCallback             (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK recordGetPositionCallback      (FMOD_OUTPUT_STATE *output, unsigned int *pcm);
        static FMOD_RESULT F_CALLBACK recordLockCallback             (FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
        static FMOD_RESULT F_CALLBACK recordUnlockCallback           (FMOD_OUTPUT_STATE *output, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);
#endif


    };
}

#endif /* FMOD_SUPPORT_DSOUND */

#endif