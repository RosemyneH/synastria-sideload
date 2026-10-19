#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_DSOUND

#include "fmod.h"
#include "fmod_channelpool.h"
#include "fmod_codec_wav.h"
#include "fmod_debug.h"
#include "fmod_file.h"
#include "fmod_file_disk.h"
#include "fmod_channel_dsound.h"
#include "fmod_channel_dsound_eax2.h"
#include "fmod_channel_dsound_eax3.h"
#include "fmod_channel_dsound_eax4.h"
#include "fmod_channel_dsound_i3dl2.h"
#include "fmod_output_dsound.h"
#include "fmod_sample_dsound.h"
#include "fmod_speakermap.h"
#include "fmod_systemi.h"
#include "fmod_os_misc.h"

#include <dsound.h>

namespace FMOD
{

//#define USE_SOFTWAREBUFFERS
//#define DISABLE_SOFTWAREBUFFERS
#define USE_HARDWARE2DBUFFERS

static const FMOD_GUID IID_IDirectSound3DListener           = { 0x279AFA84, 0x4981, 0x11CE, { 0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60} };
static const FMOD_GUID IID_IDirectSound3DBuffer             = { 0x279AFA86, 0x4981, 0x11CE, { 0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60} };
static const FMOD_GUID _KSDATAFORMAT_SUBTYPE_PCM            = { 0x00000001, 0x0000, 0x0010, { 0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71} };
static const FMOD_GUID _KSDATAFORMAT_SUBTYPE_IEEE_FLOAT     = { 0x00000003, 0x0000, 0x0010, { 0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71} };

static const FMOD_GUID DS3DALG_NO_VIRTUALIZATION            = { 0xc241333f, 0x1c1b, 0x11d2, { 0x94,0xf5,0x00,0xc0,0x4f,0xc2,0x8a,0xca} };
static const FMOD_GUID DS3DALG_HRTF_LIGHT                   = { 0xc2413342, 0x1c1b, 0x11d2, { 0x94,0xf5,0x00,0xc0,0x4f,0xc2,0x8a,0xca} };
static const FMOD_GUID DS3DALG_HRTF_FULL                    = { 0xc2413340, 0x1c1b, 0x11d2, { 0x94,0xf5,0x00,0xc0,0x4f,0xc2,0x8a,0xca} };


FMOD_OUTPUT_DESCRIPTION_EX dsoundoutput;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetOutputDescription is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_OUTPUT_DESCRIPTION_EX * F_API FMODGetOutputDescriptionEx()
    {
        return OutputDSound::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: fbc12a87 manual
// OFFSET: 0x0093CBD0
FMOD_OUTPUT_DESCRIPTION_EX *OutputDSound::getDescriptionEx()
{
    FMOD_memset(&dsoundoutput, 0, sizeof(FMOD_OUTPUT_DESCRIPTION_EX));

    dsoundoutput.name                   = "FMOD DirectSound Output";
    dsoundoutput.version                = 0x00010100;
    dsoundoutput.polling                = true;
    dsoundoutput.getnumdrivers          = &OutputDSound::getNumDriversCallback;
    dsoundoutput.getdrivername          = &OutputDSound::getDriverNameCallback;
    dsoundoutput.close                  = &OutputDSound::closeCallback;
    dsoundoutput.start                  = &OutputDSound::startCallback;
    dsoundoutput.stop                   = &OutputDSound::stopCallback;
    dsoundoutput.update                 = &OutputDSound::updateCallback;
    dsoundoutput.gethandle              = &OutputDSound::getHandleCallback;
    dsoundoutput.getposition            = &OutputDSound::getPositionCallback;
    dsoundoutput.lock                   = &OutputDSound::lockCallback;
    dsoundoutput.unlock                 = &OutputDSound::unlockCallback;

    /*
        Private members
    */
    dsoundoutput.getsamplemaxchannels   = &OutputDSound::getSampleMaxChannelsCallback;
    dsoundoutput.getdrivercapsex2       = &OutputDSound::getDriverCapsExCallback;
    dsoundoutput.initex                 = &OutputDSound::initExCallback;
    dsoundoutput.createsample           = &OutputDSound::createSampleCallback;
    dsoundoutput.getsoundram            = &OutputDSound::getSoundRAMCallback;
#ifdef FMOD_SUPPORT_RECORDING
    dsoundoutput.record_getnumdrivers   = &OutputDSound::recordGetNumDriversCallback;
    dsoundoutput.record_getdrivername   = &OutputDSound::recordGetDriverInfoCallback;
    dsoundoutput.record_start           = &OutputDSound::recordStartCallback;
    dsoundoutput.record_stop            = &OutputDSound::recordStopCallback;
    dsoundoutput.record_getposition     = &OutputDSound::recordGetPositionCallback;
    dsoundoutput.record_lock            = &OutputDSound::recordLockCallback;
    dsoundoutput.record_unlock          = &OutputDSound::recordUnlockCallback;
#endif
    dsoundoutput.reverb_setproperties   = &OutputDSound::setReverbPropertiesCallback;


    dsoundoutput.mType                  = FMOD_OUTPUTTYPE_DSOUND;
    dsoundoutput.mSize                  = sizeof(OutputDSound);

    return &dsoundoutput;
}


/*
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]
	This has to be done to selectively load different functions because of different DLL versions.

	[SEE_ALSO]
*/
// VERIFY: 4d1336df manual
// OFFSET: 0x0093AAE0
FMOD_RESULT OutputDSound::registerDLL()
{
    char szPath[512];
    char szFile[512];
    HMODULE	ddrawmodule = 0;

    if (mDLLInitialized)
    {
        return FMOD_OK;
    }

    Plugin::init(); /* Set up gGlobal - for debug / file / memory access by this plugin. */

	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::registerDLL", "Register DLL\n"));

    if (!mDSoundModule)
    {
        mDSoundModule = LoadLibrary("dsound.dll");
    }
	
    if (mInitFlags & (FMOD_INIT_DSOUND_HRTFNONE | FMOD_INIT_DSOUND_HRTFLIGHT | FMOD_INIT_DSOUND_HRTFFULL))
    {
	    mDSound3DModule = LoadLibrary("dsound3d.dll");
    }

    mDirectXVersion = 3;  /* Start off thinking it is dx3, now try to find dx8 then dx9 */

    /*
        Query dsound.dll for ds interfaces.
    */
    mDirectSoundCreate = (PFN_DSCREATE8)GetProcAddress(mDSoundModule,"DirectSoundCreate8");
    if (mDirectSoundCreate)
    {
        mDirectXVersion = 8;
    }
    else
    {
        mDirectSoundCreate = (PFN_DSCREATE8)GetProcAddress(mDSoundModule,"DirectSoundCreate");
        if (!mDirectSoundCreate)
        {
            return FMOD_ERR_OUTPUT_INIT;
        }        
    }

    mDirectSoundEnumerate = (PFN_DSENUMERATE)GetProcAddress(mDSoundModule,"DirectSoundEnumerateA");

    if (mDirectXVersion >= 8 && GetSystemDirectory( szPath, MAX_PATH ) != 0 )
    {
        FMOD_RESULT result;
        DiskFile    diskfp;
        File        *fp = &diskfp;

        FMOD_strcpy( szFile, szPath );
        FMOD_strcat( szFile, "\\d3d9.dll");

        result = fp->open(szFile, 0);
        if (result == FMOD_OK)
        {
            fp->close();
            FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::registerDLL", "Detected DIRECTX 9\n"));
            mDirectXVersion = 9;
        }
        else
        {
            FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::registerDLL", "Detected DIRECTX 8\n"));
        }
    }
    else
    {
		FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::registerDLL", "Detected DIRECTX %d\n", mDirectXVersion));
    }

    mDirectSoundCaptureCreate = (PFN_DSCCREATE8)GetProcAddress(mDSoundModule, "DirectSoundCaptureCreate8");
    if (mDirectSoundCaptureCreate)
    {
        mDirectSoundCaptureCreate = (PFN_DSCCREATE8)GetProcAddress(mDSoundModule, "DirectSoundCaptureCreate");
    }

	mDirectSoundCaptureEnumerate = (PFN_DSCENUMERATE)GetProcAddress(mDSoundModule, "DirectSoundCaptureEnumerateA");

	mDLLInitialized = true;

    return FMOD_OK;
}


/*
	[DESCRIPTION]
    Callback to enumerate each found output driver

	[PARAMETERS]
 
	[RETURN_VALUE]
    TRUE

	[REMARKS]
    The first enumerated driver is skipped as it has no GUID, (it's a duplicate
    anyway), the next driver enumerated is always the default, therefore it will be
    in element 0 of FMODs driver list as required.

	[SEE_ALSO]
*/
// VERIFY: ffc8a9dc manual
// OFFSET: 0x0093ACA0
BOOL CALLBACK FMOD_Output_DSound_EnumProc(LPGUID lpGUID, LPCSTR lpszDesc, LPCSTR lpszDrvName, LPVOID lpContext)
{
    FMOD::OutputDSound *outputdsound = (FMOD::OutputDSound *)lpContext;

	if (outputdsound->mNumDrivers >= FMOD_OUTPUT_MAXDRIVERS)
	{
        return TRUE;
    }

    if (!lpGUID)
    {
        return TRUE;
    }

    outputdsound->mDriverName[outputdsound->mNumDrivers] = (char *)FMOD_Memory_Calloc(FMOD_strlen(lpszDesc) + 1);
    if (!outputdsound->mDriverName[outputdsound->mNumDrivers])
    {
        return TRUE;
    }

    strncpy(outputdsound->mDriverName[outputdsound->mNumDrivers], lpszDesc, FMOD_strlen(lpszDesc));

    FMOD_memcpy(&outputdsound->mGUID[outputdsound->mNumDrivers], lpGUID, sizeof(GUID));

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "FMOD_Output_DSound_EnumProc", "Enumerating \"%s\"\n", lpszDesc));

    outputdsound->mNumDrivers++;

	return TRUE;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: b960cff2 auto
// OFFSET: 0x0093AD80
FMOD_RESULT OutputDSound::enumerate()
{
    FMOD_RESULT result;

    result = registerDLL();
    if (result != FMOD_OK)
    {
        return result;
    }

    for (int i = 0; i < mNumDrivers; i++)
    {
         if (mDriverName[i])
         {
             FMOD_Memory_Free(mDriverName[i]);
             mDriverName[i] = NULL;
         }
    }
    mNumDrivers = 0;

    if (mDirectSoundEnumerate)
    {
		(*mDirectSoundEnumerate)(FMOD_Output_DSound_EnumProc, (void *)this);
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputDSound::getNumDrivers(int *numdrivers)
{
    FMOD_RESULT result;

    if (!numdrivers)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = enumerate();
    if (result != FMOD_OK)
    {
        return result;
    }

    *numdrivers = mNumDrivers;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: eacd5b91 manual
// OFFSET: 0x0093AE00
FMOD_RESULT OutputDSound::getDriverName(int driver, char *name, int namelen)
{
    FMOD_RESULT result;

    if (!name || namelen < 1)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = enumerate();
    if (result != FMOD_OK)
    {
        return result;
    }

    if (driver < 0 || driver >= mNumDrivers)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    FMOD_strncpy(name, mDriverName[driver], namelen - 1);

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 7fc6c1de manual
// OFFSET: 0x0093BEA0
FMOD_RESULT OutputDSound::getDriverCapsEx(int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode, int *num2dchannels, int *num3dchannels, int *totalchannels)
{
    FMOD_RESULT result;
	DSCAPS      dscaps;
    HRESULT     hr;

    Plugin::init(); /* Set up gGlobal - for debug / file / memory access by this plugin. */
    
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::getDriverCapsEx", "Register DLL\n"));

	result = registerDLL();
    if (result != FMOD_OK)
    {
        return result;
    }

	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::getDriverCapsEx", "Enumerate Drivers\n"));

	result = enumerate();
    if (result != FMOD_OK)
    {
        return result;
    }

	if (!mNumDrivers)
	{
		FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputDSound::getDriverCapsEx", "No sound devices!\n"));
        close();
		return FMOD_ERR_OUTPUT_INIT;
	}

	if (!mCoInitialized)
	{
		FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::getDriverCapsEx", "CoInitialize\n"));
		CoInitialize(0);
		mCoInitialized = true;
	}

    /*
	    CREATE DIRECTSOUND OBJECT
	*/
	hr = !DS_OK;
	if (mDirectSoundCreate)
	{
		FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::getDriverCapsEx", "DirectSoundCreate8 : id = %d\n", id));

		if (id >= 0)
        {
			hr = (*mDirectSoundCreate)(&mGUID[id], (LPDIRECTSOUND8 *)&mDirectSound, 0);
        }
		else 
        {
			hr = (*mDirectSoundCreate)(0, (LPDIRECTSOUND8 *)&mDirectSound, 0);
        }
	}
	if (hr != DS_OK)
	{
        close();
		return FMOD_ERR_OUTPUT_INIT;
	}

	/*
	    PROCESS DIRECTSOUND CAPS FOR THIS CARD
	*/
	FMOD_memset(&dscaps, 0, sizeof(DSCAPS));
	dscaps.dwSize = sizeof(DSCAPS);

	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::getDriverCapsEx", "GetCaps\n"));

	hr = mDirectSound->GetCaps(&dscaps);
	if (hr != DS_OK)
	{
        close();
		return FMOD_ERR_OUTPUT_INIT;
	}

    if (minfrequency)
    {
        *minfrequency = dscaps.dwMinSecondarySampleRate;
    }
    if (maxfrequency)
    {
        *maxfrequency = dscaps.dwMaxSecondarySampleRate;
    }

    #ifdef USE_SOFTWAREBUFFERS
    *num2dchannels = 0; 
    #else
    *num2dchannels = dscaps.dwFreeHwMixingStreamingBuffers;    
    #endif

	if (!*num2dchannels)
    {
#ifdef DISABLE_SOFTWAREBUFFERS
	    *num2dchannels = 0;
#else
	    *num2dchannels = 32;
#endif
    }

	if (dscaps.dwMaxHw3DAllBuffers == 0 || dscaps.dwMaxHw3DAllBuffers > 200 || (dscaps.dwFlags & DSCAPS_EMULDRIVER))	/* > 200 has gotta be a lie (some drivers report 256 hw channels then crash on the dx voice manager code.. reject them) Also reject cards that have emulated drivers. */
	{
		FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::getDriverCapsEx", "Bad Caps or emulated driver.  Reverting back to software\n"));
    	*num3dchannels = 0;
	}
    else
    {
    	*num3dchannels = dscaps.dwFreeHw3DStreamingBuffers;
    }

    if (dscaps.dwFlags & DSCAPS_EMULDRIVER)
    {
        *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_HARDWARE_EMULATED);
    }

	if (*num3dchannels)
    {
        *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_HARDWARE);
    }
    else
    {
        *minfrequency = DSBFREQUENCY_MIN;
        *maxfrequency = DSBFREQUENCY_MAX;
    }

    *totalchannels = *num2dchannels + *num3dchannels;

    if (mDirectXVersion >= 9)
    {
        *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_OUTPUT_MULTICHANNEL);
        *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_OUTPUT_FORMAT_PCM8);
        *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_OUTPUT_FORMAT_PCM16);
        *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_OUTPUT_FORMAT_PCM24);
        *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_OUTPUT_FORMAT_PCM32);
        *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_OUTPUT_FORMAT_PCMFLOAT);
    }

    /*
        Get hadware reverb caps
    */
    if (*num3dchannels)
    {
        #if defined(FMOD_SUPPORT_EAX)
        if (initEAX4())
        {
            *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_REVERB_EAX4);
            mReverbVersion = REVERB_VERSION_EAX4;
            closeEAX4();
        }
        else 
        {
            closeEAX4();

            if (initEAX3())
            {
                *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_REVERB_EAX3);
                mReverbVersion = REVERB_VERSION_EAX3;
                closeEAX3();
            }
            else 
            {
                closeEAX3();

                if (initEAX2())
                {
                    *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_REVERB_EAX2);
                    mReverbVersion = REVERB_VERSION_EAX2;
                }

                closeEAX2();
            }
        }
        #endif

        #if defined(FMOD_SUPPORT_I3DL2)
        if (mReverbVersion != REVERB_VERSION_EAX4 && mReverbVersion != REVERB_VERSION_EAX3 && mReverbVersion != REVERB_VERSION_EAX2)
        {
            if (initI3DL2())
            {
                *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_REVERB_I3DL2);
                mReverbVersion = REVERB_VERSION_I3DL2;
                closeI3DL2();
            }
            else
            {
                closeI3DL2();
            }
        }
        #endif
    }


    /*
        Find out the control panel speaker mode.
    */
    {
        DWORD speakermode = 0;

        hr= mDirectSound->GetSpeakerConfig(&speakermode);


        switch ((WORD)speakermode)
        {
            case DSSPEAKER_DIRECTOUT:
            {
                break;
            }
            case DSSPEAKER_HEADPHONE:
            {
                *controlpanelspeakermode = FMOD_SPEAKERMODE_STEREO;
                break;
            }
            case DSSPEAKER_MONO:
            {
                *controlpanelspeakermode = FMOD_SPEAKERMODE_MONO;
                break;
            }
            case DSSPEAKER_STEREO:
            {
                *controlpanelspeakermode = FMOD_SPEAKERMODE_STEREO;
                break;
            }
            case DSSPEAKER_QUAD:
            {
                *controlpanelspeakermode = FMOD_SPEAKERMODE_QUAD;
                break;
            }
            case DSSPEAKER_SURROUND:
            {
                *controlpanelspeakermode = FMOD_SPEAKERMODE_SURROUND;
                break;
            }
            case DSSPEAKER_5POINT1:
            {
                *controlpanelspeakermode = FMOD_SPEAKERMODE_5POINT1;
                break;
            }
            case DSSPEAKER_7POINT1:
            case 8:
            {
                *controlpanelspeakermode = FMOD_SPEAKERMODE_7POINT1;
                break;
            }
        }
    }


    close();
    
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 0b10a9fa manual
// OFFSET: 0x0093C180
FMOD_RESULT OutputDSound::init(int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, FMOD_SPEAKERMODE *speakermode, int dspbufferlength, int dspnumbuffers, int max2dchannels, int max3dchannels, void *extradriverdata)
{
    FMOD_RESULT           result;
	HRESULT               hr;
	DSBUFFERDESC          dsbdesc; 
	DSCAPS                dscaps;
    GUID                  guid;
	WAVEFORMATEX          pcmwf;
	LPDIRECTSOUNDBUFFER	  primary = 0;
    int                   num2dchannels = 0;
    int                   num3dchannels = 0;

    mNeedToCommit   = false;
    mFeaturesReverb = 0;
    mBufferReverb   = 0;
    mSampleReverb   = 0;
    mReverbVersion  = REVERB_VERSION_NONE;


	result = registerDLL();
    if (result != FMOD_OK)
    {
        return result;
    }

	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::init", "Enumerate Drivers\n"));

	result = enumerate();
    if (result != FMOD_OK)
    {
        return result;
    }

	if (!mNumDrivers)
	{
		FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputDSound::init", "No sound devices!\n"));
        close();
		return FMOD_ERR_OUTPUT_INIT;
	}

	if (!mCoInitialized)
	{
		FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::init", "CoInitialize\n"));
		CoInitialize(0);
		mCoInitialized = true;
	}

    mInitFlags = flags;

    /*
	    CREATE DIRECTSOUND OBJECT
	*/
	hr = !DS_OK;
	if (mDirectSoundCreate)
	{
		FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::init", "DirectSoundCreate8 : mSelectedDriver = %d\n", selecteddriver));
		hr = (*mDirectSoundCreate)(&mGUID[selecteddriver], (LPDIRECTSOUND8 *)&mDirectSound, 0);
	}
	if (hr != DS_OK)
	{
        close();
		return FMOD_ERR_OUTPUT_INIT;
	}

	/*
	    SET COOPERATIVE LEVEL
	*/
    mGlobalFocus = false;
	if (!extradriverdata)
    {
        mGlobalFocus = true;
		extradriverdata = GetDesktopWindow();   /* As it is global focus, desktop is the safest bet and we don't lose sound anywhere. */
    }

	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::init", "SetCooperativeLevel\n"));

	hr = mDirectSound->SetCooperativeLevel((HWND)extradriverdata, DSSCL_PRIORITY); 
	if (hr != DS_OK) 
	{
        close();
		return FMOD_ERR_OUTPUT_ALLOCATED;
	}

    /*
	    FIND DIRECTSOUND SPEAKERMODE
	*/
    mSpeakerMode = mSystem->mSpeakerMode;

    /*
	    PROCESS DIRECTSOUND CAPS FOR THIS CARD
	*/
	FMOD_memset(&dscaps, 0, sizeof(DSCAPS));
	dscaps.dwSize = sizeof(DSCAPS);

	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::init", "GetCaps\n"));

	hr = mDirectSound->GetCaps(&dscaps);

    mMinFrequency = (float)dscaps.dwMinSecondarySampleRate;
    mMaxFrequency = (float)dscaps.dwMaxSecondarySampleRate;

    #ifdef USE_SOFTWAREBUFFERS
    num2dchannels = 0; 
    #else
    num2dchannels = dscaps.dwFreeHwMixingStreamingBuffers;    
    #endif

	if (!num2dchannels)
    {
#ifdef DISABLE_SOFTWAREBUFFERS
        mUseSoftware2DBuffers = false;
	    num2dchannels = 0;
#else
        mUseSoftware2DBuffers = true;
	    num2dchannels = 32;
#endif
    }
    else
    {
        mUseSoftware3DBuffers = false;
    }

	if (dscaps.dwMaxHw3DAllBuffers == 0 || dscaps.dwMaxHw3DAllBuffers > 200 || (dscaps.dwFlags & DSCAPS_EMULDRIVER))	/* > 200 has gotta be a lie (some drivers report 256 hw channels then crash on the dx voice manager code.. reject them) Also reject cards that have emulated drivers. */
	{
		FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::init", "Bad Caps or emulated driver.  Reverting back to software\n"));
    	num3dchannels = 0;
	}
    else
    {
        #ifdef USE_SOFTWAREBUFFERS
        num3dchannels = 0;
        #else
    	num3dchannels = dscaps.dwFreeHw3DStreamingBuffers;
        #endif
    }

	if (!num3dchannels)
    {
#ifdef DISABLE_SOFTWAREBUFFERS
        mUseSoftware3DBuffers = false;
	    num3dchannels = 0;
#else
        mUseSoftware3DBuffers = true;
        mMinFrequency = DSBFREQUENCY_MIN;
        mMaxFrequency = DSBFREQUENCY_MAX;
	    num3dchannels = 32;
#endif
    }
    else
    {
        mUseSoftware3DBuffers = false;
    }

    /*
        Handle min/max channel allocation
    */
    if (num2dchannels < mSystem->mMinHardwareChannels2D)
    {
        num2dchannels = 0;
    }
    if (num3dchannels < mSystem->mMinHardwareChannels3D)
    {
        num3dchannels = 0;
    }
    if (num2dchannels > mSystem->mMaxHardwareChannels2D)
    {
        num2dchannels = mSystem->mMaxHardwareChannels2D;
    }
    if (num3dchannels > mSystem->mMaxHardwareChannels3D)
    {
        num3dchannels = mSystem->mMaxHardwareChannels3D;
    }

    /*
        Remember buffersize
    */
    mBufferLength = dspbufferlength;
    mNumBuffers   = dspnumbuffers;

    if (1)
    {
        int buffersizems;

        do
        {
            buffersizems = (mBufferLength * mNumBuffers) * 1000 / *outputrate;

            if (buffersizems < 500)
            {
                mNumBuffers++;
            }

        } while (buffersizems < 500);
    }
    mMixAheadBlocks = mNumBuffers - dspnumbuffers;  /* Remember what the user specified. */

	/*
        SET UP PRIMARY BUFFER
	*/
    FMOD_memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); 	    /* Zero it out. */
	dsbdesc.dwSize		  = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags		  = DSBCAPS_PRIMARYBUFFER;
    dsbdesc.dwBufferBytes = 0; 					    /* Buffer size is determined by sound hardware.  */
    dsbdesc.lpwfxFormat	  = 0; 				    /* Must be 0 for primary buffers. */

	if (mDirectXVersion < 8)
    {
		dsbdesc.dwSize -= sizeof(GUID);
    }

    dsbdesc.dwFlags |= DSBCAPS_CTRL3D;

	/*
	    CREATE ONE PRIMARY BUFFER
	*/
	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::init", "Create Primary Buffer\n"));

    hr = mDirectSound->CreateSoundBuffer(&dsbdesc, &primary, 0); 
	if (FAILED(hr)) 
	{
        close();
		return FMOD_ERR_OUTPUT_CREATEBUFFER;
	}

	/*
	    SET PRIMARY BUFFER'S FORMAT
	*/
    FMOD_memset(&pcmwf, 0, sizeof(WAVE_FORMATEX)); 
    pcmwf.wFormatTag        = WAVE_FORMAT_PCM; 
	pcmwf.nChannels         = 2;
	pcmwf.wBitsPerSample    = 16; 
	pcmwf.nBlockAlign       = pcmwf.nChannels * pcmwf.wBitsPerSample / 8;
    pcmwf.nSamplesPerSec    = 44100;
    pcmwf.nAvgBytesPerSec   = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign; 
	pcmwf.cbSize            = 0;

	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::init", "Set Primary Buffer Format\n"));

    hr = primary->SetFormat(&pcmwf); 
	if (hr != DS_OK) 
	{
        close();
		return FMOD_ERR_OUTPUT_FORMAT;
	}

	/*
	    GET DS3D LISTENER INTERFACE
	*/
	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::init", "Getting Listener Interface\n"));

    FMOD_memcpy(&guid, &IID_IDirectSound3DListener, sizeof(GUID));

	hr = primary->QueryInterface(guid, (void **)&mDirectSoundListener); 
	if (hr != DS_OK) 
    {
		num3dchannels = 0;
    }

	primary->Release();

    /*
        Initialise hardware reverb
    */
    if (num3dchannels && !mUseSoftware3DBuffers)
    {
        #if defined(FMOD_SUPPORT_EAX)
        
        if (initEAX4())
        {
            mReverbVersion = REVERB_VERSION_EAX4;
        }
        else 
        {
            closeEAX4();

            if (initEAX3())
            {
                mReverbVersion = REVERB_VERSION_EAX3;
            }
            else
            {
                closeEAX3();

                if (initEAX2())
                {
                    mReverbVersion = REVERB_VERSION_EAX2;
                }
                else
                {
                    closeEAX2();
                }
            }
        }
        #endif
        
        #if defined(FMOD_SUPPORT_I3DL2)
        if (mReverbVersion != REVERB_VERSION_EAX4 && mReverbVersion != REVERB_VERSION_EAX3 && mReverbVersion != REVERB_VERSION_EAX2)
        {
            if (initI3DL2())
            {
                mReverbVersion = REVERB_VERSION_I3DL2;
            }
        }
        #endif
    }

    /*
        SET UP 2D CHANNEL POOL
    */
    if (num2dchannels)
    {
        int count;

        mChannelPool = FMOD_Object_Alloc(ChannelPool);
        if (!mChannelPool)
        {
            close();
            return FMOD_ERR_MEMORY;
        }

        result = mChannelPool->init(mSystem, this, num2dchannels);
        if (result != FMOD_OK)
        {
            close();
            return result;
        }

        mChannel2D = (ChannelDSound *)FMOD_Memory_Calloc(sizeof(ChannelDSound) * num2dchannels);
        if (!mChannel2D)
        {
            close();
            return FMOD_ERR_MEMORY;
        }

        for (count = 0; count < num2dchannels; count++)
        {
            new (&mChannel2D[count]) ChannelDSound;
            mChannelPool->setChannel(count, &mChannel2D[count], 0);
        }
    }

    /*
        SET UP 3D CHANNEL POOL
    */
    if (num3dchannels)
    {
        int count;

        mChannelPool3D = FMOD_Object_Alloc(ChannelPool);
        if (!mChannelPool3D)
        {
            close();
            return FMOD_ERR_MEMORY;
        }

        result = mChannelPool3D->init(mSystem, this, num3dchannels);
        if (result != FMOD_OK)
        {
            close();
            return result;
        }

        switch(mReverbVersion)
        {
#if defined(FMOD_SUPPORT_EAX)
            case REVERB_VERSION_EAX4:
            {
                mChannel3D = (ChannelDSoundEAX4 *)FMOD_Memory_Calloc(sizeof(ChannelDSoundEAX4) * num3dchannels);               
                break;
            }
            case REVERB_VERSION_EAX3:
            {
                mChannel3D = (ChannelDSoundEAX3 *)FMOD_Memory_Calloc(sizeof(ChannelDSoundEAX3) * num3dchannels);
                break;
            }
            case REVERB_VERSION_EAX2:
            {
                mChannel3D = (ChannelDSoundEAX2 *)FMOD_Memory_Calloc(sizeof(ChannelDSoundEAX2) * num3dchannels);
                break;
            }
#endif
#if defined(FMOD_SUPPORT_I3DL2)
            case REVERB_VERSION_I3DL2:
            {
                mChannel3D = (ChannelDSoundI3DL2 *)FMOD_Memory_Calloc(sizeof(ChannelDSoundI3DL2) * num3dchannels);
                break;
            }
#endif
            default:
            {
                mChannel3D = (ChannelDSound *)FMOD_Memory_Calloc(sizeof(ChannelDSound) * num3dchannels);
                break;
            }
        }

        if (!mChannel3D)
        {
            close();
            return FMOD_ERR_MEMORY;
        }


        for (count = 0; count < num3dchannels; count++)
        {
            
            switch(mReverbVersion)
            {
#if defined(FMOD_SUPPORT_EAX)
                case REVERB_VERSION_EAX4:
                {
                    new (&mChannel3D[count]) ChannelDSoundEAX4;
                    break;
                }
                case REVERB_VERSION_EAX3:
                {
                    new (&mChannel3D[count]) ChannelDSoundEAX3;
                    break;
                }
                case REVERB_VERSION_EAX2:
                {
                    new (&mChannel3D[count]) ChannelDSoundEAX2;
                    break;
                }
#endif
#if defined(FMOD_SUPPORT_I3DL2)
                case REVERB_VERSION_I3DL2:
                {                    
                    new (&mChannel3D[count]) ChannelDSoundI3DL2;
                    break;
                }
#endif
                default:
                {
                    new (&mChannel3D[count]) ChannelDSound;
                    break;
                }
            }

            mChannelPool3D->setChannel(count, &mChannel3D[count], 0);
        }
    }

    mLastDopplerScale   = -1.0f;
    mLastDistancerScale = -1.0f;
    mLastRolloffrScale  = -1.0f;

    mBufferMemoryCurrent = mBufferMemoryMax = 0;

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::init", "Done\n"));

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: aee2841b auto
// OFFSET: 0x0093AE60
FMOD_RESULT OutputDSound::close()
{
    int count;


    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::close", "\n"));

    switch (mReverbVersion)
    {
#if defined(FMOD_SUPPORT_EAX)
        case REVERB_VERSION_EAX4:
        {
            closeEAX4();
            break;
        }
        case REVERB_VERSION_EAX3:
        {
            closeEAX3();
            break;
        }
        case REVERB_VERSION_EAX2:
        {
            closeEAX2();
            break;
        }
#endif
#if defined (FMOD_SUPPORT_I3DL2)
        case REVERB_VERSION_I3DL2:
        {
            closeI3DL2();
            break;
        }
#endif
    }
   
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::close", "Free channel pool 2d\n"));

    if (mChannelPool)
    {    
        mChannelPool->release();
        mChannelPool = 0;
    }

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::close", "Free channel pool 3d\n"));

    if (mChannelPool3D)
    {    
        mChannelPool3D->release();
        mChannelPool3D = 0;
    }
    if (mChannel2D)
    {    
        FMOD_Memory_Free(mChannel2D);
        mChannel2D = 0;
    }
    if (mChannel3D)
    {    
        FMOD_Memory_Free(mChannel3D);
        mChannel3D = 0;
    }

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::close", "Release directsound listener\n"));

    if (mDirectSoundListener)
    {
        mDirectSoundListener->Release();
        mDirectSoundListener = 0;
    }

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::close", "Release directsound object\n"));

    if (mDirectSound)
    {
        mDirectSound->Release();
        mDirectSound = 0;
    }

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::close", "FreeLibrary on dsound3d.dll\n"));

    if (mDSound3DModule)
    {
		FreeLibrary(mDSound3DModule); 
		mDSound3DModule = 0; 
    }

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::close", "FreeLibrary on dsound.dll\n"));

    if (mDSoundModule)
    {
        FreeLibrary(mDSoundModule);
        mDSoundModule = 0;
        mDLLInitialized = false;
    }

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::close", "Free driver list\n"));

    for (count=0; count < mNumDrivers; count++)
    {
         if (mDriverName[count])
         {
             FMOD_Memory_Free(mDriverName[count]);
             mDriverName[count] = 0;
         }
    }
    
    mNumDrivers = 0;

#ifdef FMOD_SUPPORT_RECORDING
    for (count=0; count < mRecordNumDrivers; count++)
    {
         if (mRecordDriverName[count])
         {
             FMOD_Memory_Free(mRecordDriverName[count]);
             mRecordDriverName[count] = 0;
         }
    }
    
    mRecordNumDrivers = 0;
#endif


    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputDSound::close", "done\n"));

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputDSound::getHandle(void **handle)
{
    if (!handle)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *handle = mDirectSound;

    return FMOD_OK;
}

	  
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: c3c90c25 manual
// OFFSET: 0x0093B020
FMOD_RESULT OutputDSound::update()
{
    HRESULT        hr;
    int            numlisteners;
    FMOD_RESULT    result;
    float          dopplerscale, distancescale, rolloffscale;
    FMOD_VECTOR    pos   = { 0.0f, 0.0f, 0.0f }, vel = { 0.0f, 0.0f, 0.0f };
    FMOD_VECTOR    front = { 0.0f, 0.0f, 1.0f }, top = { 0.0f, 1.0f, 0.0f };

    if (!mDirectSoundListener || !mChannelPool3D) 
    {
        return FMOD_OK;
    }

    result = mSystem->get3DNumListeners(&numlisteners);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (numlisteners == 1)
    {
        result = mSystem->get3DListenerAttributes(0, &pos, &vel, &front, &top);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    result = mSystem->get3DSettings(&dopplerscale, &distancescale, &rolloffscale);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (dopplerscale  != mLastDopplerScale   || 
        distancescale != mLastDistancerScale ||
        rolloffscale  != mLastRolloffrScale  ||
        mSystem->mListener[0].mMoved ||
        mSystem->mListener[0].mRotated)
    {
        DS3DLISTENER   dsListenerParams;

        FMOD_memset(&dsListenerParams, 0, sizeof(dsListenerParams));
        dsListenerParams.dwSize           = sizeof(DS3DLISTENER);
        dsListenerParams.flDistanceFactor = 1.0f / distancescale;
        dsListenerParams.flDopplerFactor  = dopplerscale;
        dsListenerParams.flRolloffFactor  = rolloffscale;
        dsListenerParams.vOrientFront.x   = front.x;
        dsListenerParams.vOrientFront.y   = front.y;
        dsListenerParams.vOrientFront.z   = front.z;
        dsListenerParams.vOrientTop.x     = top.x;
        dsListenerParams.vOrientTop.y     = top.y;
        dsListenerParams.vOrientTop.z     = top.z;
        dsListenerParams.vPosition.x      = pos.x;
        dsListenerParams.vPosition.y      = pos.y;
        dsListenerParams.vPosition.z      = pos.z;
        dsListenerParams.vVelocity.x      = vel.x;
        dsListenerParams.vVelocity.y      = vel.y;
        dsListenerParams.vVelocity.z      = vel.z;

        if (mSystem->mFlags & FMOD_INIT_3D_RIGHTHANDED)
        {
            dsListenerParams.vOrientFront.z   = -dsListenerParams.vOrientFront.z;
            dsListenerParams.vOrientTop.z     = -dsListenerParams.vOrientTop.z;
            dsListenerParams.vPosition.z      = -dsListenerParams.vPosition.z;
            dsListenerParams.vVelocity.z      = -dsListenerParams.vVelocity.z;
        }

        hr = mDirectSoundListener->SetAllParameters(&dsListenerParams, DS3D_DEFERRED); //mInitFlags & FMOD_INIT_DSOUND_DEFERRED ? DS3D_DEFERRED : DS3D_IMMEDIATE);
        if (hr != DS_OK)
        {
            FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputDSound::update", "IDirectSoundListener::SetAllParameters returned errcode %08X\n", hr));
            return FMOD_ERR_OUTPUT_DRIVERCALL;
        }
        mNeedToCommit = true;

        mLastDopplerScale   = dopplerscale; 
        mLastDistancerScale = distancescale;
        mLastRolloffrScale  = rolloffscale;        

        /*
            Update channels using tweaked distance models or fudge positioning for multilistener stuff.
        */
        {
            int numchannels, count;
            ChannelReal   *channelreal;
            ChannelDSound *channeldsound;

            if (mChannelPool3D)
            {
                result = mChannelPool3D->getNumChannels(&numchannels);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }
            else
            {
                numchannels = 0;
            }

            for (count = 0; count < numchannels; count++)
            {
                result = mChannelPool3D->getChannel(count, &channelreal);
                if (result != FMOD_OK)
                {
                    return result;
                }

                channeldsound = SAFE_CAST(ChannelDSound, channelreal);
                if (!channeldsound)
                {
                    return FMOD_ERR_INTERNAL;
                }

                if (numlisteners > 1 || channeldsound->mMode & (FMOD_3D_LINEARROLLOFF | FMOD_3D_CUSTOMROLLOFF) || mSystem->mRolloffCallback || (channeldsound->mParent && channeldsound->mParent->m3DDopplerLevel != 1.0f))
                {
                    result = channeldsound->set3DAttributes();
                    if (result != FMOD_OK)
                    {
                        return result;
                    }
                }
            }
        }
    }

    if (mNeedToCommit)
    {
        if (mDirectSoundListener->CommitDeferredSettings() != DS_OK)
        {
            FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputDSound::update", "IDirectSoundListener::CommitDeferredSettings returned errcode %08X\n", hr));
            return FMOD_ERR_OUTPUT_DRIVERCALL;
        }

        mNeedToCommit = false;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 95eb02dc manual
// OFFSET: 0x0093B2C0
FMOD_RESULT OutputDSound::createSample(FMOD_MODE mode, FMOD_CODEC_WAVEFORMAT *waveformat, Sample **sample)
{
	WAVE_FORMATEXTENSIBLE pcmwf;
	DSBUFFERDESC          dsbdesc; 
	int                   bits;
	HRESULT               hr;
    FMOD_RESULT           result;
    SampleDSound         *newsample = 0;
    GUID                  nullguid = { 0x00000000L, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::createSample", "length %d, channels %d, format %d, mode %08x\n", waveformat ? waveformat->lengthpcm : 0, waveformat ? waveformat->channels : 0, waveformat ? waveformat->format : FMOD_SOUND_FORMAT_NONE, mode));

    if (!sample)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (waveformat)
    {
        result = SoundI::getBitsFromFormat(waveformat->format, &bits);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (!bits && waveformat->format != FMOD_SOUND_FORMAT_NONE)
        {
            if (waveformat->format == FMOD_SOUND_FORMAT_MPEG || waveformat->format == FMOD_SOUND_FORMAT_IMAADPCM)
            {
                return FMOD_ERR_NEEDSSOFTWARE;
            }
            else
            {
                return FMOD_ERR_FORMAT;
            }
        }

        if (mDirectXVersion < 9 && (bits > 16 || waveformat->channels > 2))
        {
            return FMOD_ERR_FORMAT;
        }
    }

    if (*sample == 0)
    {
        newsample = FMOD_Object_Calloc(SampleDSound);
        if (!newsample)
        {
            return FMOD_ERR_INVALID_PARAM;
        }
    }
    else
    {
        newsample = SAFE_CAST(SampleDSound, *sample);
    }

    if (!waveformat)
    {
        *sample = newsample;
        return FMOD_OK;
    }

    newsample->mFormat = waveformat->format;

    /*
        ALLOCATE NEW SECONDARY BUFFER FOR THIS SAMPLE DEFINITION
    */
    FMOD_memset(&dsbdesc,0,sizeof(DSBUFFERDESC));
    dsbdesc.dwSize			= sizeof(dsbdesc);
    dsbdesc.dwBufferBytes	= waveformat->lengthpcm * bits / 8 * waveformat->channels;
    dsbdesc.lpwfxFormat		= (WAVEFORMATEX *)&pcmwf;
    dsbdesc.guid3DAlgorithm = nullguid;

    newsample->mLengthBytes = dsbdesc.dwBufferBytes;

    if (mUseSoftware3DBuffers && mode & FMOD_3D)
    {
        if (mInitFlags & FMOD_INIT_DSOUND_HRTFNONE)
        {
            FMOD_memcpy(&dsbdesc.guid3DAlgorithm, &DS3DALG_NO_VIRTUALIZATION, sizeof(GUID));
        }
        else if (mInitFlags & FMOD_INIT_DSOUND_HRTFLIGHT)
        {
            FMOD_memcpy(&dsbdesc.guid3DAlgorithm, &DS3DALG_HRTF_LIGHT, sizeof(GUID));
        }
        else if (mInitFlags & FMOD_INIT_DSOUND_HRTFFULL)
        {
            FMOD_memcpy(&dsbdesc.guid3DAlgorithm, &DS3DALG_HRTF_FULL, sizeof(GUID));
        }
    }

    dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME;
    if (mGlobalFocus)
    {
    	dsbdesc.dwFlags |= DSBCAPS_GLOBALFOCUS;
    }
    if (!(mode & FMOD_UNIQUE))
    {
        dsbdesc.dwFlags |= DSBCAPS_LOCDEFER;
    }

    newsample->mLOCSoftware = true;
	if (mode & FMOD_3D)
	{
        if (!mUseSoftware3DBuffers)
        {
            newsample->mLOCSoftware = false;
        }

		dsbdesc.dwFlags |= DSBCAPS_CTRL3D; 
		if (mode & FMOD_UNIQUE)
		{
            if (mUseSoftware3DBuffers)
            {
                dsbdesc.dwFlags |= DSBCAPS_LOCSOFTWARE;
            }
            else
            {
			    dsbdesc.dwFlags |= DSBCAPS_LOCHARDWARE;
            }
		}
	}
    else
    {
        /*
            Use software buffer if channelcount > 2 or it is a nonstandard bitdepth, hardware is bound to die if this is attempted.
        */
        if (!mUseSoftware2DBuffers && waveformat->channels <= 2 && (waveformat->format == FMOD_SOUND_FORMAT_PCM8 || waveformat->format == FMOD_SOUND_FORMAT_PCM16))
        {
            newsample->mLOCSoftware = false;
        }

        dsbdesc.dwFlags |= DSBCAPS_CTRLPAN;           /* 2d sounds require pan */
        if (mode & FMOD_UNIQUE)
        {
            dsbdesc.dwFlags &= ~DSBCAPS_LOCDEFER;

            if (mUseSoftware2DBuffers)
            {
                dsbdesc.dwFlags |= DSBCAPS_LOCSOFTWARE;
            }
            else
            {
			    dsbdesc.dwFlags |= DSBCAPS_LOCHARDWARE;
            }
        }
    }

    FMOD_memset(&pcmwf,0,sizeof(WAVE_FORMATEXTENSIBLE));

    if (mode & FMOD_3D && (waveformat->channels > 2 || (waveformat->format != FMOD_SOUND_FORMAT_PCM8 && waveformat->format != FMOD_SOUND_FORMAT_PCM16)))
    {
        return FMOD_ERR_FORMAT;
    }

    /* 
        We could use WAVEFORMATEXTENSIBLE for everything, but virtually every time we try to use it a creative card falls over on it.
        1.  Audigy NX bluescreens if you use WAVEFORMATEXTENSIBLE even for plain PCM, mono, 16bit voices.
        2.  SBLive value reported to go silent if channelmask is 0, even though the microsoft docs say 0 = direct out!
        3.  Even if you specify SPEAKER_STEREO as is recommended by Microsoft, the panning biases towards the left on some cards.
    */
    if (mDirectXVersion >= 9 && !(mode & FMOD_3D) && (waveformat->channels > 2 || (waveformat->format != FMOD_SOUND_FORMAT_PCM8 && waveformat->format != FMOD_SOUND_FORMAT_PCM16)))
    {
        pcmwf.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    }
    else
    {
        pcmwf.Format.wFormatTag = WAVE_FORMAT_PCM;
    }

    pcmwf.Format.nChannels            = waveformat->channels;
    pcmwf.Format.wBitsPerSample       = bits;
    pcmwf.Format.nBlockAlign          = pcmwf.Format.nChannels * pcmwf.Format.wBitsPerSample / 8;
    pcmwf.Format.nSamplesPerSec       = DEFAULT_FREQUENCY;
    pcmwf.Format.nAvgBytesPerSec	  = pcmwf.Format.nSamplesPerSec * pcmwf.Format.nBlockAlign;

    if (pcmwf.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        pcmwf.Format.cbSize               = 22;
        pcmwf.Samples.wValidBitsPerSample = bits;

        if (waveformat->channelmask & SPEAKER_WAVEFORMAT_MASK)
        {
            pcmwf.dwChannelMask = waveformat->channelmask & SPEAKER_WAVEFORMAT_MASK;
        }
        else
        {
            switch (mSpeakerMode)
            {
                case FMOD_SPEAKERMODE_MONO:          /* 1 speaker setup.  This includes 1 speaker. */
                {
                    pcmwf.dwChannelMask = SPEAKER_MONO;
                    break;
                }
                case FMOD_SPEAKERMODE_STEREO:          /* 2 speaker setup.  This includes front left, front right. */
                {
                    pcmwf.dwChannelMask = SPEAKER_STEREO;
                    break;
                }
                case FMOD_SPEAKERMODE_QUAD:          /* 4 speaker setup.  This includes front, center, left, rear and a subwoofer. Also known as a "quad" speaker configuration. */
                {
                    pcmwf.dwChannelMask = SPEAKER_QUAD;
                    break;
                }
                case FMOD_SPEAKERMODE_SURROUND:      /* 5 speaker setup.  This includes front, center, left, rear left, right right. */
                {
                    pcmwf.dwChannelMask = SPEAKER_SURROUND;
                    break;
                }
                case FMOD_SPEAKERMODE_5POINT1:       /* 5.1 speaker setup.  This includes front, center, left, rear left, rear right and a subwoofer. */
                {
                    pcmwf.dwChannelMask = SPEAKER_5POINT1; 
                    break;
                }
                case FMOD_SPEAKERMODE_7POINT1:       /* 7.1 speaker setup.  This includes front, center, left, rear left, rear right, side left, side right and a subwoofer. */
                {
                    pcmwf.dwChannelMask = SPEAKER_7POINT1; 
                    break;
                }
                default:              /* There is no specific speakermode.  Sound channels are mapped in order of input to output.  See remarks for more information. */
                {
                    pcmwf.dwChannelMask = 0;
                    break;
                }
            };
        }

        if (waveformat->format == FMOD_SOUND_FORMAT_PCMFLOAT)
        {
            FMOD_memcpy(&pcmwf.SubFormat, &_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(GUID));
        }
        else
        {
            FMOD_memcpy(&pcmwf.SubFormat, &_KSDATAFORMAT_SUBTYPE_PCM, sizeof(GUID));
        }        
    }

    /*
        CREATE THE BUFFER
    */
    hr = mDirectSound->CreateSoundBuffer(&dsbdesc, (LPDIRECTSOUNDBUFFER *)&newsample->mBuffer, NULL);
    if (FAILED(hr))
    {
        FMOD_Memory_Free(newsample);
        return FMOD_ERR_OUTPUT_CREATEBUFFER;
    }

    mBufferMemoryCurrent += dsbdesc.dwBufferBytes;
    if (mBufferMemoryCurrent > mBufferMemoryMax)
    {
        mBufferMemoryMax = mBufferMemoryCurrent;
    }

    /*
        GET 3D INTERFACE IF IT IS UNIQUE
    */
    if (mode & FMOD_3D && mode & FMOD_UNIQUE)
    {
        GUID guid;

        FMOD_memcpy(&guid, &IID_IDirectSound3DBuffer, sizeof(GUID));

	    if (newsample->mBuffer->QueryInterface(guid, (LPVOID *)&newsample->mBuffer3D) != DS_OK)
	    {
            FMOD_Memory_Free(newsample);

		    return FMOD_ERR_OUTPUT_CREATEBUFFER;
	    }
    }

    /*
        CLEAR OUT BUFFER
    */
    {
        void *ptr1, *ptr2;
        unsigned int len1, len2;

        hr = newsample->lock(0, waveformat->lengthpcm * bits / 8 * waveformat->channels, &ptr1, &ptr2, &len1, &len2);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (ptr1) 
        {
            FMOD_memset(ptr1, 0, len1);
        }
        if (ptr2) 
        {
            FMOD_memset(ptr2, 0, len2);
        }

        hr = newsample->unlock(ptr1, ptr2, len1, len2);
        if (hr != FMOD_OK)
        {
            return result;
        }
    }

    /*
        Make DirectSound do pre-processing on sound effects.
    */
    newsample->mBuffer->SetCurrentPosition(0);
    newsample->mLength = waveformat->lengthpcm;
    newsample->mOutput = this;

    *sample = newsample;

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::createSample", "done\n"));

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputDSound::getSoundRAM(int *currentalloced, int *maxalloced, int *total)
{
    if (currentalloced)
    {
        *currentalloced = mBufferMemoryCurrent;
    }
    if (maxalloced)
    {
        *maxalloced = mBufferMemoryMax;
    }
    if (total)
    {
        *total = 0;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputDSound::setReverbProperties(const FMOD_REVERB_PROPERTIES *prop)
{
    FMOD_RESULT result = FMOD_OK;

     switch (mReverbVersion)
    {
#if defined(FMOD_SUPPORT_EAX)
        case REVERB_VERSION_EAX4:
        {
            result = setPropertiesEAX4(prop);
            break;
        }
        case REVERB_VERSION_EAX3:
        {
            result = setPropertiesEAX3(prop);
            break;
        }
        case REVERB_VERSION_EAX2:
        {
            result = setPropertiesEAX2(prop);
            break;
        }
#endif
#if defined (FMOD_SUPPORT_I3DL2)
        case REVERB_VERSION_I3DL2:
        {
            result = setPropertiesI3DL2(prop);
            break;
        }
#endif
    }

	return result;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputDSound::getReverbProperties(FMOD_REVERB_PROPERTIES *prop)
{
    FMOD_RESULT result = FMOD_ERR_UNSUPPORTED;

     switch (mReverbVersion)
    {
#if defined(FMOD_SUPPORT_EAX)
        case REVERB_VERSION_EAX4:
        {
            result = getPropertiesEAX4(prop);
            break;
        }
        case REVERB_VERSION_EAX3:
        {
            result = getPropertiesEAX3(prop);
            break;
        }
        case REVERB_VERSION_EAX2:
        {
            result = getPropertiesEAX2(prop);
            break;
        }
#endif
#if defined (FMOD_SUPPORT_I3DL2)
        case REVERB_VERSION_I3DL2:
        {
            result = getPropertiesI3DL2(prop);
            break;
        }
#endif
    }

    return result;
}
            

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: dcc50e2e manual
// OFFSET: 0x0093B8D0
FMOD_RESULT OutputDSound::start()
{
    FMOD_RESULT             result;
    SampleDSound           *sample;
    HRESULT                 hr;
    FMOD_CODEC_WAVEFORMAT   waveformat;
    bool                    oldusesoftware;

    FMOD_memset(&waveformat, 0, sizeof(FMOD_CODEC_WAVEFORMAT));
    
    result = mSystem->getSoftwareFormat(&waveformat.frequency, &waveformat.format, &waveformat.channels, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    waveformat.lengthpcm = mBufferLength * mNumBuffers;
   
    oldusesoftware = mUseSoftware2DBuffers;

    #if 0
    mUseSoftware2DBuffers = true;   /* Force it to use software buffers?  On creative cards this screws 5.1/7.1 mixing - it only works if you use hardware buffers. */
    #endif

    result = createSample(FMOD_2D | FMOD_HARDWARE | FMOD_LOOP_NORMAL | FMOD_UNIQUE, &waveformat, &mSample);
    if (result != FMOD_OK)
    {
        mUseSoftware2DBuffers = oldusesoftware;
        return result;
    }
    
    mUseSoftware2DBuffers = oldusesoftware;

    sample = SAFE_CAST(SampleDSound, mSample);

    sample->mDefaultFrequency = (float)waveformat.frequency;
    sample->mFormat           = waveformat.format;
    sample->mChannels         = waveformat.channels;
    sample->mSystem           = mSystem;

#ifdef FMOD_OUTPUT_DSOUND_USETIMER
    result = OutputTimer::start();
#else
    result = OutputPolled::start();
#endif
    if (result != FMOD_OK)
    {
        return result;
    }

    hr = sample->mBuffer->SetVolume(0);
	if (hr != DS_OK)
	{
		FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputDSound::start", "IDirectSoundBuffer::SetVolume(0) returned errcode %08X\n", hr));
        return FMOD_ERR_OUTPUT_DRIVERCALL;
	}   

    hr = sample->mBuffer->SetFrequency(waveformat.frequency);
	if (hr != DS_OK)
	{
		FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputDSound::start", "IDirectSoundBuffer::SetFrequency(%d) returned errcode %08X\n", waveformat.frequency, hr));
        return FMOD_ERR_OUTPUT_DRIVERCALL;
	}   

    hr = sample->mBuffer->SetPan(DSBPAN_CENTER);
	if (hr != DS_OK)
	{
		FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputDSound::start", "IDirectSoundBuffer::SetPan(DSBPAN_CENTER) returned errcode %08X\n", hr));
        return FMOD_ERR_OUTPUT_DRIVERCALL;
	}   

    hr = sample->mBuffer->Play(0,0,DSBPLAY_LOOPING);
	if (hr != DS_OK)
	{
		FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputDSound::start", "IDirectSoundBuffer::Play returned errcode %08X\n", hr));
        return FMOD_ERR_OUTPUT_DRIVERCALL;
	}   

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputDSound::stop()
{
    FMOD_RESULT     result;
    SampleDSound   *sample;

    sample = SAFE_CAST(SampleDSound, mSample);
    if (!sample)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

    sample->mBuffer->Stop();    /* Dont return an error, the sound device may have died, just free stuff as normal. */

#ifdef FMOD_OUTPUT_DSOUND_USETIMER
    result = OutputTimer::stop();
#else
    result = OutputPolled::stop();
#endif
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mSample->release();
    if (result != FMOD_OK)
    {
        return result;
    }

    mSample = 0;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: a5f44c17 manual
// OFFSET: 0x0093C7F0
FMOD_RESULT OutputDSound::getPosition(unsigned int *pcm)
{
    SampleDSound   *sample;
    HRESULT         hr;
    DWORD           dwPlay, dwWrite;
    
#if 0
    unsigned int curr = timeGetTime();
    static unsigned int last = 0;
    char s[256];
    sprintf(s, "delta = %d ms\n", curr - last);
    OutputDebugString(s);
    last = curr;
#endif

    if (!mSample)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!pcm)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    sample = SAFE_CAST(SampleDSound, mSample);
    hr = sample->mBuffer->GetCurrentPosition(&dwPlay, &dwWrite);
	if (hr != DS_OK)
	{
		FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputDSound::start", "IDirectSoundBuffer::Play returned errcode %08X\n", hr));

        return FMOD_ERR_OUTPUT_DRIVERCALL;
	}

    if (mMixAheadBlocks)
    {
        return SoundI::getSamplesFromBytes(dwWrite, pcm, sample->mChannels, sample->mFormat);
    }
    else
    {
        return SoundI::getSamplesFromBytes(dwPlay, pcm, sample->mChannels, sample->mFormat);
    }
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputDSound::lock(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
    if (!mSample)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return mSample->lock(offset, length, ptr1, ptr2, len1, len2);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputDSound::unlock(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
    if (!mSample)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return mSample->unlock(ptr1, ptr2, len1, len2);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
int OutputDSound::getSampleMaxChannels(FMOD_MODE mode, FMOD_SOUND_FORMAT format)
{
    if (mode & FMOD_3D)
    {
        return 1;
    }
    else
    {
        return 8;       /* 2D voices can go up to 8 channels wide. */
    }
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
 
	[REMARKS]

    [PLATFORMS]
 
	[SEE_ALSO]
]
*/


/*
    ==============================================================================================================

    CALLBACK INTERFACE

    ==============================================================================================================
*/


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 263d4c2b auto
// OFFSET: 0x0093B9F0
FMOD_RESULT F_CALLBACK OutputDSound::getNumDriversCallback(FMOD_OUTPUT_STATE *output, int *numdrivers)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->getNumDrivers(numdrivers);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: cf6cdc75 auto
// OFFSET: 0x0093BA30
FMOD_RESULT F_CALLBACK OutputDSound::getDriverNameCallback(FMOD_OUTPUT_STATE *output, int id, char *name, int namelen)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->getDriverName(id, name, namelen);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: ddeace31 auto
// OFFSET: 0x0093CB10
FMOD_RESULT OutputDSound::getDriverCapsExCallback(FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode, int *num2dchannels, int *num3dchannels, int *totalchannels)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->getDriverCapsEx(id, caps, minfrequency, maxfrequency, controlpanelspeakermode, num2dchannels, num3dchannels, totalchannels);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 341dc789 auto
// OFFSET: 0x0093CB50
FMOD_RESULT F_CALLBACK OutputDSound::initExCallback(FMOD_OUTPUT_STATE *output, int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, FMOD_SPEAKERMODE *speakermode, int dspbufferlength, int dspnumbuffers, int max2dchannels, int max3dchannels, void *extradriverdata)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->init(selecteddriver, flags, outputrate, outputchannels, outputformat, speakermode, dspbufferlength, dspnumbuffers, max2dchannels, max3dchannels, extradriverdata);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 86831f60 manual
// OFFSET: 0x0093BA60
FMOD_RESULT F_CALLBACK OutputDSound::closeCallback(FMOD_OUTPUT_STATE *output)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->close();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 5fb0e8e0 manual
// OFFSET: 0x0093BA90
FMOD_RESULT F_CALLBACK OutputDSound::startCallback(FMOD_OUTPUT_STATE *output)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->start();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 3e8a6c96 auto
// OFFSET: 0x0093BAC0
FMOD_RESULT F_CALLBACK OutputDSound::stopCallback(FMOD_OUTPUT_STATE *output)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->stop();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 13ce4c10 manual
// OFFSET: 0x0093BB20
FMOD_RESULT F_CALLBACK OutputDSound::updateCallback(FMOD_OUTPUT_STATE *output)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->update();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 4598d291 manual
// OFFSET: 0x0093BB50
FMOD_RESULT F_CALLBACK OutputDSound::getHandleCallback(FMOD_OUTPUT_STATE *output, void **handle)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->getHandle(handle);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 3cc77ab4 manual
// OFFSET: 0x0093CBA0
FMOD_RESULT F_CALLBACK OutputDSound::getPositionCallback(FMOD_OUTPUT_STATE *output, unsigned int *pcm)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->getPosition(pcm);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 6367824e manual
// OFFSET: 0x0093BB80
FMOD_RESULT F_CALLBACK OutputDSound::lockCallback(FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->lock(offset, length, ptr1, ptr2, len1, len2);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 5c5d629b manual
// OFFSET: 0x0093BBD0
FMOD_RESULT F_CALLBACK OutputDSound::unlockCallback(FMOD_OUTPUT_STATE *output, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->unlock(ptr1, ptr2, len1, len2);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: e07ebdb2 auto
// OFFSET: 0x0093BC20
FMOD_RESULT F_CALLBACK OutputDSound::createSampleCallback(FMOD_OUTPUT_STATE *output, FMOD_MODE mode, FMOD_CODEC_WAVEFORMAT *waveformat, Sample **sample)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->createSample(mode, waveformat, sample);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: ba73cfe4 auto
// OFFSET: 0x0093BC50
FMOD_RESULT F_CALLBACK OutputDSound::getSoundRAMCallback(FMOD_OUTPUT_STATE *output, int *currentalloced, int *maxalloced, int *total)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->getSoundRAM(currentalloced, maxalloced, total);
}


#ifdef FMOD_SUPPORT_RECORDING
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: d915b0b5 manual
// OFFSET: 0x0093BCA0
FMOD_RESULT F_CALLBACK OutputDSound::recordGetNumDriversCallback(FMOD_OUTPUT_STATE *output, int *numdrivers)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->recordGetNumDrivers(numdrivers);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 2a5aba39 auto
// OFFSET: 0x0093BCD0
FMOD_RESULT F_CALLBACK OutputDSound::recordGetDriverInfoCallback(FMOD_OUTPUT_STATE *output, int id, char *name, int namelen)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->recordGetDriverInfo(id, name, namelen);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 8741d5ed auto
// OFFSET: 0x0093BD00
FMOD_RESULT F_CALLBACK OutputDSound::recordStartCallback(FMOD_OUTPUT_STATE *output, int id, FMOD_SOUND *sound, int loop)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->recordStart(id, (Sound *)sound, loop ? true : false);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 772a4da9 manual
// OFFSET: 0x0093BD30
FMOD_RESULT F_CALLBACK OutputDSound::recordStopCallback(FMOD_OUTPUT_STATE *output)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->recordStop();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: f0cf9059 manual
// OFFSET: 0x0093BD60
FMOD_RESULT F_CALLBACK OutputDSound::recordGetPositionCallback(FMOD_OUTPUT_STATE *output, unsigned int *pcm)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->recordGetPosition(pcm);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 1ea5dfad auto
// OFFSET: 0x0093BD90
FMOD_RESULT F_CALLBACK OutputDSound::recordLockCallback(FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->recordLock(offset, length, ptr1, ptr2, len1, len2);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 1363c973 auto
// OFFSET: 0x0093BDD0
FMOD_RESULT F_CALLBACK OutputDSound::recordUnlockCallback(FMOD_OUTPUT_STATE *output, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->recordUnlock(ptr1, ptr2, len1, len2);
}
#endif


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 43c647df auto
// OFFSET: 0x0093BE00
FMOD_RESULT F_CALLBACK OutputDSound::setReverbPropertiesCallback(FMOD_OUTPUT_STATE *output, const FMOD_REVERB_PROPERTIES *prop)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->setReverbProperties(prop);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: de6d987a auto
// OFFSET: 0x0093BE50
int F_CALLBACK OutputDSound::getSampleMaxChannelsCallback(FMOD_OUTPUT_STATE *output, FMOD_MODE mode, FMOD_SOUND_FORMAT format)
{
    OutputDSound *dsound = (OutputDSound *)output;

    return dsound->getSampleMaxChannels(mode, format);
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/

}

#endif /* FMOD_SUPPORT_DSOUND */
