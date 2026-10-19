#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL

#include "fmod_output_openal.h"
#include "fmod_sample_openal.h"
#include "fmod_channel_openal.h"
#include "fmod_channel_openal_eax2.h"
#include "fmod_channel_openal_eax3.h"
#include "fmod_channel_openal_eax4.h"
#include "fmod_channel_openal_eax5.h"

namespace FMOD
{

#define SPEED_OF_SOUND 340.0f   // Same as A3D

#define EXIT_ON_CONDITION(condition, error) \
    if (condition) { close(); return error; }
#define SAFE_RELEASE(pPointer) \
    if ((pPointer) != NULL) { (pPointer)->release(); (pPointer) = NULL; }
#define SAFE_FREE(pPointer) \
    if ((pPointer) != NULL) { FMOD_Memory_Free((pPointer)); (pPointer) = NULL; }

FMOD_OUTPUT_DESCRIPTION_EX openaloutput;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetOutputDescription is mandantory for every fmod plugin.
        This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_OUTPUT_DESCRIPTION_EX * F_API FMODGetOutputDescriptionEx()
    {
        return OutputOpenAL::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


/*
[
	[DESCRIPTION]
    Assign methods for various FMOD output hooks required by output type

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OUTPUT_DESCRIPTION_EX

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 42fe6e4d auto
// OFFSET: 0x0092C160
FMOD_OUTPUT_DESCRIPTION_EX *OutputOpenAL::getDescriptionEx()
{
    openaloutput.name                   = "FMOD OpenAL Output";
    openaloutput.version                = 0x00010100;
    openaloutput.mType                  = FMOD_OUTPUTTYPE_OPENAL;
    openaloutput.mSize                  = sizeof(OutputOpenAL);    
    openaloutput.polling                = true;

    openaloutput.getnumdrivers          = &OutputOpenAL::getNumDriversCallback;
    openaloutput.getdrivername          = &OutputOpenAL::getDriverNameCallback;
    openaloutput.getdrivercapsex2       = &OutputOpenAL::getDriverCapsExCallback;
    openaloutput.init                   = &OutputOpenAL::initCallback;
    openaloutput.close                  = &OutputOpenAL::closeCallback;
    openaloutput.start                  = &OutputOpenAL::startCallback;
    openaloutput.stop                   = &OutputOpenAL::stopCallback;
    openaloutput.gethandle              = &OutputOpenAL::getHandleCallback;
    openaloutput.update                 = &OutputOpenAL::updateCallback;
    openaloutput.postmixcallback        = &OutputOpenAL::postMixCallback;
    openaloutput.getposition            = &OutputOpenAL::getPositionCallback;
    openaloutput.lock                   = &OutputOpenAL::lockCallback;
    openaloutput.unlock                 = &OutputOpenAL::unlockCallback;
    openaloutput.createsample           = &OutputOpenAL::createSampleCallback;
    openaloutput.reverb_setproperties   = &OutputOpenAL::setReverbPropertiesCallback;
    openaloutput.getsamplemaxchannels   = &OutputOpenAL::getSampleMaxChannelsCallback;

    return &openaloutput;
}


/*
[
	[DESCRIPTION]
    OpenAL Constructor

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 49e35d77 manual
// OFFSET: 0x0092B6D0
OutputOpenAL::OutputOpenAL()
{
    mDLLInitialised  = false;
    mInitialised     = false;
    mNumDrivers      = 0;
}


/*
[
	[DESCRIPTION]
    Load the OpenAL DLL

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputOpenAL::registerDLL()
{
    if (mDLLInitialised)
    {
        return FMOD_OK;
    }

    if (!LoadOALLibrary(NULL, &mOALFnTable))
    {
        return FMOD_ERR_OUTPUT_INIT;
    }
    
    mDLLInitialised = true;
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Enumerate a list of OpenAL devices

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]
    OpenAL devices list extracts device names for native devices i.e. Creative, the
    generic devices use whatever is currently the default

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 9011d4bd manual
// OFFSET: 0x0092A340
FMOD_RESULT OutputOpenAL::enumerate()
{
    FMOD_RESULT  result        = FMOD_OK;
    char        *deviceList    = NULL;
    ALCdevice   *device        = NULL;
    ALCcontext  *context       = NULL;

	if (!mSetupOnce)
	{
        Plugin::init();

        result = registerDLL();
        if (result != FMOD_OK)
        {
            return result;
        }

        mSetupOnce = true;
	}

    for (int i = 0; i < mNumDrivers; i++)
    {
        SAFE_FREE(mDriverNames[i]);
    }
    mNumDrivers = 0;

    deviceList = (char *)mOALFnTable.alcGetString(NULL, ALC_DEVICE_SPECIFIER);

	while (*deviceList != NULL && mNumDrivers < FMOD_OUTPUT_MAXDRIVERS)
    {
        device = mOALFnTable.alcOpenDevice(deviceList);
        if (device == NULL)
        {
            continue;
        }

        context = mOALFnTable.alcCreateContext(device, NULL);
        if (context == NULL)
        {
            mOALFnTable.alcCloseDevice(device);
            continue;
        }

	    mOALFnTable.alcGetError(device);
	    mOALFnTable.alcMakeContextCurrent(context);
	    if (mOALFnTable.alcGetError(device) != ALC_NO_ERROR)
	    {
            mOALFnTable.alcDestroyContext(context);
            mOALFnTable.alcCloseDevice(device);
            continue;
        }

        int majorVersion = 0;
        int minorVersion = 0;

        mOALFnTable.alcGetIntegerv(device, ALC_MAJOR_VERSION, sizeof(int), &majorVersion);
        mOALFnTable.alcGetIntegerv(device, ALC_MINOR_VERSION, sizeof(int), &minorVersion);
        mOALFnTable.alcGetError(device);

        if (majorVersion >= 1 && minorVersion >= 1)
        {
            const char  *deviceName = mOALFnTable.alcGetString(device, ALC_DEVICE_SPECIFIER);
            bool         dupeFound  = false;

            for (int i = 0; i < mNumDrivers; i++)
            {
                if (FMOD_strcmp(deviceName, mDriverNames[i]) == 0)
                {
                    dupeFound = true;
                    break;
                }
            }

            if (!dupeFound)
            {
                mDriverNames[mNumDrivers] = (char *)FMOD_Memory_Calloc(FMOD_strlen(deviceName) + 1);
                if (mDriverNames[mNumDrivers] == NULL)
                {
                    mOALFnTable.alcMakeContextCurrent(NULL);
                    mOALFnTable.alcDestroyContext(context);
                    mOALFnTable.alcCloseDevice(device);
                    return FMOD_ERR_MEMORY;
                }

                FMOD_strcpy(mDriverNames[mNumDrivers], deviceName);
                mNumDrivers++;
            }
        }

        mOALFnTable.alcMakeContextCurrent(NULL);
        mOALFnTable.alcDestroyContext(context);
		mOALFnTable.alcCloseDevice(device);

        deviceList += FMOD_strlen(deviceList) + 1;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Return the number of OpenAL devices

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputOpenAL::getNumDrivers(int *numdrivers)
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
    Return the name of an OpenAL device given its index

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 4b8376ae manual
// OFFSET: 0x0092A610
FMOD_RESULT OutputOpenAL::getDriverName(int driver, char *name, int namelen)
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

    FMOD_strncpy(name, mDriverNames[driver], namelen - 1);

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Return the capabilities of the OpenAL device at the given index

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 7f67faa5 manual
// OFFSET: 0x0092A670
FMOD_RESULT OutputOpenAL::getDriverCapsEx(int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode, int *num2dchannels, int *num3dchannels, int *totalchannels)
{
    ALCdevice  *device       = NULL;
    ALCcontext *context      = NULL;
    FMOD_RESULT result       = FMOD_OK;

    result = enumerate();
    if (result != FMOD_OK)
    {
        return result;
    }

    // Check if using the software mode (fallback from hardware is already considered in the enumeration)
    if (FMOD_strcmp(mDriverNames[id], "Generic Software") == 0 ||
        FMOD_strcmp(mDriverNames[id], "DirectSound") == 0      ||
        FMOD_strcmp(mDriverNames[id], "MMSYSTEM") == 0)
    {
        *caps |= FMOD_CAPS_HARDWARE_EMULATED;
    }
    // Must be using hardware through DSound or a native driver
    else
    {
        *caps |= FMOD_CAPS_HARDWARE;
    }

    // OpenAL only supports PCM8 and PCM16, but we force PCM16
    *caps |= FMOD_CAPS_OUTPUT_FORMAT_PCM16;

    // OpenAL frequency is forced to 44100
    *minfrequency = *maxfrequency = 44100;

    /*
        Check for EAX support
    */
    #ifdef FMOD_SUPPORT_EAX
    // Open the selected device
    device = mOALFnTable.alcOpenDevice(mDriverNames[id]);
    if (!device)
    {
        return FMOD_ERR_OUTPUT_INIT;
    }

    // Create a context for the device
    context = mOALFnTable.alcCreateContext(device, NULL);
    if (!context)
    {
        mOALFnTable.alcCloseDevice(device);
        return FMOD_ERR_OUTPUT_INIT;
    }

    // Make context active
	mOALFnTable.alcGetError(device);
	mOALFnTable.alcMakeContextCurrent(context);
	if (mOALFnTable.alcGetError(device) != ALC_NO_ERROR)
	{
        mOALFnTable.alcDestroyContext(context);
        mOALFnTable.alcCloseDevice(device);
        return FMOD_ERR_OUTPUT_INIT;
	}

    // Determine highest EAX version supported
    if (mOALFnTable.alIsExtensionPresent("EAX5.0"))
    {
        *caps |= FMOD_CAPS_REVERB_EAX5;
        
        result = getSpeakerModeEAX5(controlpanelspeakermode);
        if (result != FMOD_OK)
        {
            FLOG((FMOD_DEBUG_LEVEL_WARNING, __FILE__, __LINE__, "OutputOpenAL::getDriverCapsEx", "Could not get speaker mode, defaulting to Stereo\n"));
            *controlpanelspeakermode = FMOD_SPEAKERMODE_STEREO;
        }
        mNumHwChannels = 128;
    }
    else if (mOALFnTable.alIsExtensionPresent("EAX4.0"))
    {
        *caps |= FMOD_CAPS_REVERB_EAX4;
        mNumHwChannels = 64;
    }
    else if (mOALFnTable.alIsExtensionPresent("EAX3.0"))
    {
        *caps |= FMOD_CAPS_REVERB_EAX3;
        mNumHwChannels = 64;
    }
    else if (mOALFnTable.alIsExtensionPresent("EAX2.0"))
    {
        *caps |= FMOD_CAPS_REVERB_EAX2;
        mNumHwChannels = 32;
    }
    else
    {
        mNumHwChannels = 8;
    }

    // Cleanup OpenAL device
    mOALFnTable.alcMakeContextCurrent(NULL);
    mOALFnTable.alcDestroyContext(context);
    mOALFnTable.alcCloseDevice(device);
    #endif

    *num2dchannels = mNumHwChannels;
    *num3dchannels = mNumHwChannels;
    *totalchannels = mNumHwChannels;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Initialise EAX for use by OpenAL

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 5a8858f9 auto
// OFFSET: 0x0092A890
FMOD_RESULT OutputOpenAL::initEAX()
{
    #ifdef FMOD_SUPPORT_EAX
    // Determine supported version of EAX
    if (mOALFnTable.alIsExtensionPresent("EAX5.0"))
    {
        mReverbVersion = REVERB_VERSION_EAX5;
        mNumHwChannels = 128;
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::initEAX", "EAX version 5.0 detected\n"));
    }
    else if (mOALFnTable.alIsExtensionPresent("EAX4.0"))
    {
        mReverbVersion = REVERB_VERSION_EAX4;
        mNumHwChannels = 60;
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::initEAX", "EAX version 4.0 detected\n"));
    }
    else if (mOALFnTable.alIsExtensionPresent("EAX3.0"))
    {
        mReverbVersion = REVERB_VERSION_EAX3;
        mNumHwChannels = 60;
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::initEAX", "EAX version 3.0 detected\n"));
    }
    else if (mOALFnTable.alIsExtensionPresent("EAX2.0"))
    {
        mReverbVersion = REVERB_VERSION_EAX2;
        mNumHwChannels = 30;
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::initEAX", "EAX version 2.0 detected\n"));
    }
    else
    {
        // Should not get this far, software mode OpenAL emulates EAX2
        mReverbVersion = REVERB_VERSION_NONE;
        mNumHwChannels = 8;
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::initEAX", "No EAX version detected"));
    }

    // Load EAX Get and Set functions
    mEAXGet = (EAXGet)mOALFnTable.alGetProcAddress("EAXGet");
    mEAXSet = (EAXSet)mOALFnTable.alGetProcAddress("EAXSet");
    if (!mEAXGet || !mEAXSet)
    {
        mReverbVersion = REVERB_VERSION_NONE;
    }

    // EAX5.0 Requires you to setup a session before making any EAX5.0 calls
	if (mReverbVersion == REVERB_VERSION_EAX5)
	{
		setupSessionEAX5();
	}
    #else
    mReverbVersion = REVERB_VERSION_NONE;
    mNumHwChannels = 8;
    #endif

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Initialise OpenAL output

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 66c50cf5 manual
// OFFSET: 0x0092B700
FMOD_RESULT OutputOpenAL::init(int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata)
{
    FMOD_RESULT result    = FMOD_OK;
    int formatBits        = 0;
    mDevice               = NULL;
    mContext              = NULL;
    mBufferData           = NULL;
    mNumChannels          = 0;
    mChannels             = NULL;
    mNumSources           = 0;
    mSources              = NULL;
    mMixerReverbDisabled  = false;
    mEAXSet               = NULL;
    mEAXGet               = NULL;
	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::init", "Initialising...\n"));
    
    result = enumerate();
    EXIT_ON_CONDITION(result != FMOD_OK, result);

    if (mNumDrivers <= 0)
	{
		FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputOpenAL::init", "No sound devices with support for OpenAL v1.1\n"));
        close();
		return FMOD_ERR_OUTPUT_INIT;
	}

    // Use provided number of channels
    mOutputChannels = outputchannels;
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::init", "Using provided number of channels: %d\n", mOutputChannels));
    // Override specified output rate
    mRate = *outputrate = 44100;
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::init", "Forcing sample rate: 44100\n"));
    // Override specified output format
    mFormat = *outputformat = FMOD_SOUND_FORMAT_PCM16;
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::init", "Forcing output format: PCM16\n"));
    
    // Get the number of bits per sample
    result = SoundI::getBitsFromFormat(mFormat, &formatBits);
    EXIT_ON_CONDITION(result != FMOD_OK, result);
    // Set buffer size
    mNumBuffers = dspnumbuffers;
    mBufferLength = dspbufferlength;
    mBufferLengthBytes = mBufferLength * mOutputChannels * (formatBits / 8);

    // Open the selected device
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::init", "Opening device\n"));
    mDevice = mOALFnTable.alcOpenDevice(mDriverNames[selecteddriver]);
    if (!mDevice)
    {
        FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputOpenAL::init", "Couldn't open device: %s", mDriverNames[selecteddriver]));
        close();
        return FMOD_ERR_OUTPUT_INIT;
    }

    // Create a context for the device
    mContext = mOALFnTable.alcCreateContext(mDevice, NULL);
    if (!mContext)
    {
        FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputOpenAL::init", "Couldn't create context for device: %s", mDriverNames[selecteddriver]));
        close();
        return FMOD_ERR_OUTPUT_INIT;
    }

    // Make context active
	mOALFnTable.alcGetError(mDevice);
	mOALFnTable.alcMakeContextCurrent(mContext);
	if (mOALFnTable.alcGetError(mDevice) != ALC_NO_ERROR)
	{
        FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputOpenAL::init", "Couldn't make context current for device: %s", mDriverNames[selecteddriver]));
        close();
        return FMOD_ERR_OUTPUT_INIT;
	}

    // Set speed of sound
    mOALFnTable.alSpeedOfSound(SPEED_OF_SOUND);
    if (mOALFnTable.alcGetError(mDevice) != ALC_NO_ERROR)
    {
        FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputOpenAL::init", "Couldn't set speed of sound to %.02f", SPEED_OF_SOUND));
        close();
        return FMOD_ERR_OUTPUT_INIT;
    }

    // Initialised EAX
    result = initEAX();
    EXIT_ON_CONDITION(result != FMOD_OK, result);

    // Clear Error Codes
	mOALFnTable.alGetError();
	mOALFnTable.alcGetError(mDevice);

    // Limit hardware channels by the user set max
    if (mNumHwChannels > mSystem->mMaxHardwareChannels3D)
    {
        mNumHwChannels = mSystem->mMaxHardwareChannels3D;
    }    
    // Reserve hardware channels for the FMOD software mixer
    mNumChannels = mNumHwChannels - mOutputChannels;
    // Shouldn't go negative, mNumHwChannels min is set at 8, but just in case
    if (mNumChannels < 0)
    {
        close();
        return FMOD_ERR_OUTPUT_INIT;        
    }
    
	// Allocate memory for mix buffer
    mBufferData = (char *)FMOD_Memory_Calloc(mNumBuffers * mBufferLengthBytes);
    EXIT_ON_CONDITION(mBufferData == NULL, FMOD_ERR_MEMORY);

    // Allocate memory for OpenAL sources
    mSources = (SourceOpenAL *)FMOD_Memory_Calloc(mNumHwChannels * sizeof(SourceOpenAL));
    EXIT_ON_CONDITION(mSources == NULL, FMOD_ERR_MEMORY);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::init", "Initialising device with %d channels\n", mNumHwChannels));
    
    /*
        Create OpenAL sources for each hardware channel
    */
    // Clear errors
    mOALFnTable.alGetError();
    do
	{
		mOALFnTable.alGenSources(1, &mSources[mNumSources].sid);
        if (mOALFnTable.alGetError() != AL_NO_ERROR)
		{
            break;  // Could not create OpenAL source
        }

        // Assign this source as currently unused
        mSources[mNumSources].used = false;
        
        // Allocate space for the buffer IDs
        mSources[mNumSources].bid = (ALuint *)FMOD_Memory_Calloc(mNumBuffers * sizeof(ALuint));
        EXIT_ON_CONDITION(mSources[mNumSources].bid == NULL, FMOD_ERR_MEMORY);
		
		// Generate buffers for each source
        for (int i = 0; i < mNumBuffers; i++)
		{
			mOALFnTable.alGenBuffers(1, &mSources[mNumSources].bid[i]);
            if (mOALFnTable.alGetError() != AL_NO_ERROR)
		    {
                break;  // Could not create OpenAL buffer
            }

			mOALFnTable.alBufferData(mSources[mNumSources].bid[i], AL_FORMAT_MONO16, mBufferData, mBufferLength * (formatBits / 8), mRate);
            if (mOALFnTable.alGetError() != AL_NO_ERROR)
		    {
                break;  // Could not fill OpenAL buffer
            }
		}
		
        mNumSources++;

	} while (mNumSources < mNumHwChannels);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::init", "Initialised device with %d channels\n", mNumSources));

    // Update mNumChannels incase enough sources couldn't be created
    mNumChannels = mNumSources - mOutputChannels;
    // Ensure enough sources were created for the FMOD software mixer
    if (mNumSources < mOutputChannels)
    {
        return FMOD_ERR_OUTPUT_INIT;
    }

    // FMOD software mixer uses the top OpenAL sources for output
    mMixerSourceOffset = mNumSources - mOutputChannels;

    // Set up the hardware channel pool
    mChannelPool3D = FMOD_Object_Alloc(ChannelPool);
    EXIT_ON_CONDITION(mChannelPool3D == NULL, FMOD_ERR_MEMORY);
    result = mChannelPool3D->init(mSystem, this, mNumChannels);
    EXIT_ON_CONDITION(result != FMOD_OK, result);

    // Allocate channel memory
    switch (mReverbVersion)
    {
        #ifdef FMOD_SUPPORT_EAX
        case REVERB_VERSION_EAX5:
        {
            mChannels = (ChannelOpenALEAX5 *)FMOD_Memory_Calloc(sizeof(ChannelOpenALEAX5) * mNumChannels);
            break;
        }
        case REVERB_VERSION_EAX4:
        {
            mChannels = (ChannelOpenALEAX4 *)FMOD_Memory_Calloc(sizeof(ChannelOpenALEAX4) * mNumChannels);
            break;
        }
        case REVERB_VERSION_EAX3:
        {
            mChannels = (ChannelOpenALEAX3 *)FMOD_Memory_Calloc(sizeof(ChannelOpenALEAX3) * mNumChannels);
            break;
        }
        case REVERB_VERSION_EAX2:
        {
            mChannels = (ChannelOpenALEAX2 *)FMOD_Memory_Calloc(sizeof(ChannelOpenALEAX2) * mNumChannels);
            break;
        }
        #endif
        default:
        {
            mChannels = (ChannelOpenAL *)FMOD_Memory_Calloc(sizeof(ChannelOpenAL) * mNumChannels);
            break;
        }
    }
    EXIT_ON_CONDITION(mChannels == NULL, FMOD_ERR_MEMORY);

    // Create the channels
    for (int i = 0; i < mNumChannels; i++)
    {
        switch (mReverbVersion)
        {
            #ifdef FMOD_SUPPORT_EAX
            case REVERB_VERSION_EAX5:
            {
				new (&mChannels[i]) ChannelOpenALEAX5;
                break;
            }
            case REVERB_VERSION_EAX4:
            {
                new (&mChannels[i]) ChannelOpenALEAX4;
                break;
            }
            case REVERB_VERSION_EAX3:
            {
                new (&mChannels[i]) ChannelOpenALEAX3;
                break;
            }
            case REVERB_VERSION_EAX2:
            {
                new (&mChannels[i]) ChannelOpenALEAX2;
                break;
            }
            #endif
            default:
            {
                new (&mChannels[i]) ChannelOpenAL;
                break;
            }
        }

        // Apply the channel to the channel pool
        mChannelPool3D->setChannel(i, &mChannels[i], 0);
    }

    // 2D channel pool is the same as the 3D pool
    mChannelPool = mChannelPool3D;

	// Get format enum for mix buffer
	switch (mOutputChannels)
	{
		case 1:
			mFormatOAL = mOALFnTable.alGetEnumValue("AL_FORMAT_MONO16");
			break;
		case 2:
			mFormatOAL = mOALFnTable.alGetEnumValue("AL_FORMAT_STEREO16");
			break;
		case 4:
			mFormatOAL = mOALFnTable.alGetEnumValue("AL_FORMAT_QUAD16");
			break;
		case 6:
			mFormatOAL = mOALFnTable.alGetEnumValue("AL_FORMAT_51CHN16");
			break;
		case 7:
			mFormatOAL = mOALFnTable.alGetEnumValue("AL_FORMAT_61CHN16");
			break;
		case 8:
			mFormatOAL = mOALFnTable.alGetEnumValue("AL_FORMAT_71CHN16");
			break;
		default:
			mFormatOAL = 0;
            FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputOpenAL::init", "alGetEnumValue failed\n"));
            return FMOD_ERR_OUTPUT_DRIVERCALL;
	}

    mInitialised = true;
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::init", "Done.\n"));
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Shutdown and cleanup resources created

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: dd0d6fe9 auto
// OFFSET: 0x0092A9B0
FMOD_RESULT OutputOpenAL::close()
{ 
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::close", "Closing...\n"));

    // Release the channel pool, (both pools are the same)
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::close", "Free channel pool\n"));
    SAFE_RELEASE(mChannelPool);
    mChannelPool = mChannelPool3D = NULL;
    // Free the channel data
    SAFE_FREE(mChannels);

    // Cleanup OpenAL sources
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::close", "Free OpenAL sources and buffers\n"));
    if (mSources)
	{
		for (int i = 0; i < mNumSources; i++)
		{
			mOALFnTable.alDeleteSources(1, &mSources[i].sid);
			mOALFnTable.alDeleteBuffers(mNumBuffers, mSources[i].bid);
			SAFE_FREE(mSources[i].bid);
		}
        SAFE_FREE(mSources);
    }

    // Clean up the mix buffer
    SAFE_FREE(mBufferData)

    // Cleanup OpenAL context and device
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::close", "Closing OpenAL device\n"));
    if (mContext)
    {
		mOALFnTable.alcMakeContextCurrent(NULL);
        mOALFnTable.alcDestroyContext(mContext);
        mContext = NULL;
    }
    if (mDevice)
    {
        mOALFnTable.alcCloseDevice(mDevice);
        mDevice = NULL;
    }

    // Cleanup driver list
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::close", "Free OpenAL driver list\n"));
    for (int i = 0; i < mNumDrivers; i++)
    {
        SAFE_FREE(mDriverNames[i]);
    }

    // Cleanup OpenAL library
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::close", "FreeLibrary on OpenAL32.dll\n"));
    UnloadOALLibrary();

    mDLLInitialised       = false;
    mSetupOnce            = false;
    mNumDrivers           = 0;
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::close", "Done.\n"));
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Start playing the software mixer buffer

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 29ad656d manual
// OFFSET: 0x0092ABA0
FMOD_RESULT OutputOpenAL::start()
{
    FMOD_RESULT	result = FMOD_OK;
    
    // Reset PCM base position
    mPcmBase = 0;

	// Mark the mixer sources as in use by software mixer
	for (int i = 0; i < mOutputChannels; i++)
	{
		mSources[mMixerSourceOffset + i].used = true;
	}

    // Fill OpenAL buffers
	for (int j = 0; j < mNumBuffers; j++)
	{
		int dataOffset = j * mBufferLengthBytes;

        // Note: If using OpenAL "Generic Software" device, OpenAL will downmix to stereo
        mOALFnTable.alBufferData(mSources[mMixerSourceOffset].bid[j], mFormatOAL, mBufferData + dataOffset, mBufferLengthBytes, mRate);
		mOALFnTable.alSourceQueueBuffers(mSources[mMixerSourceOffset].sid, 1, &mSources[mMixerSourceOffset].bid[j]);
	}
	
	// Start the mixer thread
    result = OutputTimer::start();
    if (result != FMOD_OK)
    {
        return result;
    }

	// Set the properties of the mixer source, then start it
    mOALFnTable.alSourcef(mSources[mMixerSourceOffset].sid, AL_GAIN, 1.0f);
	mOALFnTable.alSourcef(mSources[mMixerSourceOffset].sid, AL_PITCH, 1.0f);
	mOALFnTable.alSourcei(mSources[mMixerSourceOffset].sid, AL_LOOPING, AL_FALSE);
	mOALFnTable.alSourcePlay(mSources[mMixerSourceOffset].sid);

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Stop playing the shared buffer

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: dbd4b728 auto
// OFFSET: 0x0092AD20
FMOD_RESULT OutputOpenAL::stop()
{
    FMOD_RESULT	result = FMOD_OK;

    if (mInitialised)
    {
        // Stop the mixer thread
        result = OutputTimer::stop();
        if (result != FMOD_OK)
        {
            return result;
        }

        // Stop the mixer source
	    mOALFnTable.alSourceStop(mSources[mMixerSourceOffset].sid);
	    mOALFnTable.alSourcei(mSources[mMixerSourceOffset].sid, AL_BUFFER, NULL);
     
        // Mark the mixer sources as not in use by software mixer
	    for (int i = 0; i < mOutputChannels; i++)
	    {
		    mSources[mMixerSourceOffset + i].used = false;
	    }
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Return a handle to the OpenAL device

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputOpenAL::getHandle(void **handle)
{
    if (!handle)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *handle = mDevice;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Update OpenAL parameters

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 3d5f8812 auto
// OFFSET: 0x0092ADB0
FMOD_RESULT OutputOpenAL::update()
{
    FMOD_RESULT   result             = FMOD_OK;
    int           numListeners       = 0;
    float         dopplerScale       = 0.0f;
    float         distanceScale      = 0.0f;
    float         rolloffScale       = 0.0f;
    static float  lastDopplerScale   = 0.0f;
    static float  lastDistanceScale  = 0.0f;
    static float  lastRolloffScale   = 0.0f;
    
    // Note: FMOD default is LHS, OAL default is RHS
    FMOD_VECTOR fPosition        = { 0.0f, 0.0f, 0.0f };
    FMOD_VECTOR fVelocity        = { 0.0f, 0.0f, 0.0f };
    FMOD_VECTOR fForward         = { 0.0f, 0.0f, 1.0f };
    FMOD_VECTOR fUp              = { 0.0f, 1.0f, 0.0f };
    ALfloat     alPosition[]     = { 0.0f, 0.0f, 0.0f };
    ALfloat     alVelocity[]     = { 0.0f, 0.0f, 0.0f };
    ALfloat     alOrientation[]  = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };   // First 3 are Forward, second 3 are Up

    // Get the number of FMOD listeners
    result = mSystem->get3DNumListeners(&numListeners);
    if (result != FMOD_OK)
    {
        return result;
    }

    // If there is only one listener get its details
    if (numListeners == 1)
    {
        result = mSystem->get3DListenerAttributes(0, &fPosition, &fVelocity, &fForward, &fUp);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    // Get FMOD 3D sound settings
    result = mSystem->get3DSettings(&dopplerScale, &distanceScale, &rolloffScale);
    if (result != FMOD_OK)
    {
        return result;
    }

    // Only update settings if necessary
    if (!(dopplerScale  != lastDopplerScale  || 
          distanceScale != lastDistanceScale ||
          rolloffScale  != lastRolloffScale  ||
          mSystem->mListener[0].mMoved       ||
          mSystem->mListener[0].mRotated))
    {
        return FMOD_OK;
    }

    /* Update OpenAL listener details from FMOD data */
    alPosition[0] = fPosition.x;
    alPosition[1] = fPosition.y;
    alPosition[2] = fPosition.z;

    alVelocity[0] = fVelocity.x;
    alVelocity[1] = fVelocity.y;
    alVelocity[2] = fVelocity.z;

    alOrientation[0] = fForward.x;
    alOrientation[1] = fForward.y;
    alOrientation[2] = fForward.z;
    alOrientation[3] = fUp.x;
    alOrientation[4] = fUp.y;
    alOrientation[5] = fUp.z;

    // Flip the Z-axis if FMOD is not in RHS mode
    if (!(mSystem->mFlags & FMOD_INIT_3D_RIGHTHANDED))
    {
        alPosition[2]    *= -1;   // fPosition.z
        alVelocity[2]    *= -1;   // fVelocity.z
        alOrientation[2] *= -1;   // fForward.z
        alOrientation[5] *= -1;   // fUp.z
    }

    // Set the listener properties    
    mOALFnTable.alListenerfv(AL_POSITION, alPosition);
    mOALFnTable.alListenerfv(AL_VELOCITY, alVelocity);
    mOALFnTable.alListenerfv(AL_ORIENTATION, alOrientation);

    // Update the rolloff scale of all the sources
    for (int i = 0; i < mNumSources; i++)
    {
        mOALFnTable.alSourcef(mSources[i].sid, AL_ROLLOFF_FACTOR, rolloffScale);
    }

    // Update the doppler factor
    mOALFnTable.alDopplerFactor(dopplerScale);
    // Update distance factor by setting the speed of sound
    mOALFnTable.alSpeedOfSound(SPEED_OF_SOUND * distanceScale);

    // Remember scale values for next update()
    lastDopplerScale  = dopplerScale;
    lastDistanceScale = distanceScale;
    lastRolloffScale  = rolloffScale;

    // Update channels using tweaked distance models or fudge positioning for multilistener stuff
    for (int j = 0; j < mNumChannels; j++)
    {
        ChannelReal   *channelReal    = NULL;
        ChannelOpenAL *channelOpenAL  = NULL;

        result = mChannelPool3D->getChannel(j, &channelReal);
        if (result != FMOD_OK)
        {
            return result;
        }

        channelOpenAL = SAFE_CAST(ChannelOpenAL, channelReal);
        if (channelOpenAL == NULL)
        {
            return FMOD_ERR_INTERNAL;
        }                
        
        // Process the channel for multilistener or tweaked distance models
        if (numListeners > 1 || channelOpenAL->mMode & (FMOD_3D_LINEARROLLOFF | FMOD_3D_CUSTOMROLLOFF))
        {
            result = channelOpenAL->set3DAttributes();
            if (result != FMOD_OK)
            {
                return result;
            }
        }
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Process the OpenAL sources

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 96f9d076 auto
// OFFSET: 0x0092B060
FMOD_RESULT OutputOpenAL::postMix()
{
    FMOD_RESULT     result             = FMOD_OK;
    ChannelReal    *channelReal        = NULL;
    ChannelOpenAL  *channelOpenAL      = NULL;
    static int	    lastUpdatedChannel = 0;

    // Only update half of the channels each mix *optimisation*
    for (int i = 0; i < mNumChannels; i += 2)
	{
        result = mChannelPool3D->getChannel(lastUpdatedChannel++, &channelReal);
        if (result != FMOD_OK)
		{
            return result;
        }

		channelOpenAL = SAFE_CAST(ChannelOpenAL, channelReal);
        if (channelOpenAL == NULL)
        {
            return FMOD_ERR_INTERNAL;
        }
        
        result = channelOpenAL->updateChannel();
        if (result != FMOD_OK)
        {
            return result;
        }
		
        // Wrap the current channel index
        if (lastUpdatedChannel >= mNumChannels)
		{
			lastUpdatedChannel = 0;
		}
	}
	
	return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Return the sample offset in the buffer of the block currently playing

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: f5a2a938 auto
// OFFSET: 0x0092B0E0
FMOD_RESULT OutputOpenAL::getPosition(unsigned int *pcm)
{
    if (pcm == NULL)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    // Get index of current buffer
	ALint processed = 0;
    mOALFnTable.alGetSourcei(mSources[mMixerSourceOffset].sid, AL_BUFFERS_PROCESSED, &processed);

    // Check for starvation
	if (processed == mNumBuffers)
	{
		mOALFnTable.alSourcePlay(mSources[mMixerSourceOffset].sid);
		processed = 0;
	}

    // Return the sample number of the current buffer being processed	
    *pcm = (mPcmBase + (processed * mBufferLength)) % (mNumBuffers * mBufferLength);

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Give the mixer thread access to the shared mixer buffer

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputOpenAL::lock(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
    if (!mBufferData)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

	*ptr1 = mBufferData + offset;
	*len1 = length;
	*ptr2 = NULL;
	*len2 = 0;

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
// VERIFY: 1cf4ceb2 manual
// OFFSET: 0x0092B180
FMOD_RESULT OutputOpenAL::unlock(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
    ALuint        bufferId      = 0;
    ALint         playing       = AL_STOPPED;
    char         *pNewDataPos   = (char *)ptr1;
    unsigned int  dataToBuffer  = len1;

    if (!mBufferData)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

	// Keep looping until all the data written to mBufferData by the mixer thread is in OpenAL buffers
    while (dataToBuffer >= mBufferLengthBytes)
	{
		// Remove a used buffer from the source
        mOALFnTable.alSourceUnqueueBuffers(mSources[mMixerSourceOffset].sid, 1, &bufferId);
		// Copy the next chunk of new data into the currently unqueued OpenAL buffer
        mOALFnTable.alBufferData(bufferId, mFormatOAL, pNewDataPos, mBufferLengthBytes, mRate);
		// Requeue the buffer that is now filled with new data
        mOALFnTable.alSourceQueueBuffers(mSources[mMixerSourceOffset].sid, 1, &bufferId);
        
        // Update positions for next chunk of data to buffer
        pNewDataPos += mBufferLengthBytes;
        dataToBuffer -= mBufferLengthBytes;

        // Move to the next sample write position in mBufferData
        mPcmBase = (mPcmBase + mBufferLength) % (mNumBuffers * mBufferLength);
	}

	// If the buffers have stopped, start them up again (this means all buffers finished before new data arrived)
    mOALFnTable.alGetSourcei(mSources[mMixerSourceOffset].sid, AL_SOURCE_STATE, &playing);
	if (playing != AL_PLAYING)
    {
		mOALFnTable.alSourcePlay(mSources[mMixerSourceOffset].sid);
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Create an OpenAL hardware sample

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 41a98657 manual
// OFFSET: 0x0092BD30
FMOD_RESULT OutputOpenAL::createSample(FMOD_MODE mode, FMOD_CODEC_WAVEFORMAT *waveformat, Sample **sample)
{
    FMOD_RESULT    result         = FMOD_OK;
    SampleOpenAL  *newsample      = NULL;
    unsigned int   overflowBytes  = 0;

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::createSample", "PCM Length: %d, Byte Length: %d, Num Channels: %d, Format: %d, Mode: %08x\n", waveformat ? waveformat->lengthpcm : 0, waveformat ? waveformat->lengthbytes : 0, waveformat ? waveformat->channels : 0, waveformat ? waveformat->format : FMOD_SOUND_FORMAT_NONE, mode));

    if (!sample)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    // If a waveformat was specified then ensure it is valid
    if (waveformat)
    {
        int formatBits = 0;
        result = SoundI::getBitsFromFormat(waveformat->format, &formatBits);
        if (result != FMOD_OK)
        {
            return result;
        }

        // If no bits are returned and the format isn't NONE, MPEG or IMMADPCM then the format is invalid
        if (formatBits == 0 && waveformat->format != FMOD_SOUND_FORMAT_NONE
                        #ifdef FMOD_SUPPORT_DSPCODEC              
                            #ifdef FMOD_SUPPORT_IMAADPCM
                            && waveformat->format != FMOD_SOUND_FORMAT_IMAADPCM
                            #endif
                            #ifdef FMOD_SUPPORT_MPEG
                            && waveformat->format != FMOD_SOUND_FORMAT_MPEG
                            #endif
                        #endif
           )
        {
            return FMOD_ERR_FORMAT;
        }
    }
    
    // Check if a sample was provided
    if (*sample == NULL)
    {
        // If not, allocate one
        newsample = FMOD_Object_Calloc(SampleOpenAL);
        if (newsample == NULL)
        {
            return FMOD_ERR_MEMORY;
        }
    }
    else
    {
        // If so, use it
        newsample = SAFE_CAST(SampleOpenAL, *sample);
    }

    // If no waveformat is provided, nothing more to do
    if (waveformat == NULL)
    {
        *sample = newsample;
        return FMOD_OK;
    }

    // Check for compressed formats
    if (waveformat->format == FMOD_SOUND_FORMAT_IMAADPCM || waveformat->format == FMOD_SOUND_FORMAT_MPEG)
    {
        newsample->mLengthBytes       = waveformat->lengthbytes;
        newsample->mLoopPointDataEnd  = NULL;
        overflowBytes                 = 0;
    }
    else
    {
        // Get the length of the sample in bytes
        result = SoundI::getBytesFromSamples(waveformat->lengthpcm, &newsample->mLengthBytes, waveformat->channels, waveformat->format);
        if (result != FMOD_OK)
        {
            return result;
        }

        // Get the length of the overflow bytes for resampling and loop points 
        result = SoundI::getBytesFromSamples(FMOD_DSP_RESAMPLER_OVERFLOWLENGTH, &overflowBytes, waveformat->channels, waveformat->format);
        if (result != FMOD_OK)
        {
            return result;
        }
        
        if (overflowBytes <= 8)
        {
            // Loop points can use already allocated data 
            newsample->mLoopPointDataEnd = newsample->mLoopPointDataEndMemory;
        }
        else
        {
            // Loop point data is large so allocate now
            newsample->mLoopPointDataEnd = (char *)FMOD_Memory_Calloc(overflowBytes);
            if (newsample->mLoopPointDataEnd = NULL)
            {
                return FMOD_ERR_MEMORY;
            }
        }
    }

    // Only allocate sample memory if needed
    if (mode & FMOD_OPENMEMORY_POINT)
    {
        newsample->mBufferMemory = NULL;
        newsample->mBuffer = NULL;
    }
    else
    {
        // Allocate enough memory for the sample, overflow at either end and 16 byte alignment
        newsample->mBufferMemory = FMOD_Memory_Calloc(newsample->mLengthBytes + (overflowBytes * 2) + 16);
        if (!newsample->mBufferMemory)
        {
            return FMOD_ERR_MEMORY;
        }

        // Move the memory pointer past the the front overflow and align to 16 byte boundry
        newsample->mBuffer = (char *)(((size_t)newsample->mBufferMemory + overflowBytes + 15) & ~15);
    }

    newsample->mFormat = waveformat->format;
    newsample->mLength = waveformat->lengthpcm;
    *sample = newsample;

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::createSample", "done\n"));
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Pass the reverb properties through to the current reverb version

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputOpenAL::setReverbProperties(const FMOD_REVERB_PROPERTIES *prop)
{
    FMOD_RESULT result = FMOD_OK;

    #if defined(FMOD_SUPPORT_EAX)
    switch (mReverbVersion)
    {
        case REVERB_VERSION_EAX5:
        {
            result = setPropertiesEAX5(prop);
            break;
        }
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
    }
    #endif

	return result;
}


/*
[
	[DESCRIPTION]
    Get the maximum number of channels a sample can have

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
int OutputOpenAL::getSampleMaxChannels(FMOD_MODE mode, FMOD_SOUND_FORMAT format)
{
    return 16;       /* 2D and 3D voices can be panned without needing to split them up.  Allow up to 16 channel wide. */
}


/*
[
	[DESCRIPTION]
    Get a free OpenALChannel from the channel pool

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]
    Each channel is associated with OpenAL sources, if not enough are available then no channel can be returned

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: c7375c31 manual
// OFFSET: 0x0092B2C0
FMOD_RESULT OutputOpenAL::getFreeChannel(FMOD_MODE mode, ChannelReal **realchannel, int numchannels, int numsoundchannels, int *found)
{
    FMOD_RESULT        result          = FMOD_OK;
    ChannelOpenAL     *channel         = NULL;
    FMOD_SPEAKERMODE   speakerMode     = FMOD_SPEAKERMODE_STEREO;
    int                requiredSources = 2;
    int                sourcesFound    = 0;
    
    // Try to get a free channel from the pool
    result = Output::getFreeChannel(mode, realchannel, numchannels, numsoundchannels, found);
    if (result != FMOD_OK)
    {
        return result;
    }

    channel = SAFE_CAST(ChannelOpenAL, *realchannel);

    /* Maximum number of sources needed is equal to the speaker mode since
       the DSP network will mix to that before it gets to the OpenALChannel */
    switch (mSystem->mSpeakerMode)
    {
        case FMOD_SPEAKERMODE_MONO    : requiredSources = 1;
                                        break;
        case FMOD_SPEAKERMODE_STEREO  : requiredSources = 2;
                                        break;
        case FMOD_SPEAKERMODE_QUAD    : requiredSources = 4;
                                        break;
        case FMOD_SPEAKERMODE_5POINT1 : requiredSources = 6;
                                        break;
        case FMOD_SPEAKERMODE_7POINT1 : requiredSources = 8;
                                        break;
        default                       : requiredSources = 2;
                                        break;
    }

    /* If the number of channels in the source is less than the speaker mode
       then we only need enough sources for the sound source */
    if (numsoundchannels < requiredSources)
    {
        requiredSources = numsoundchannels;
    }

    // Find enough sources for the number of sound channels required
    for (int i = 0; i < mNumSources; i++)
	{
		if (!mSources[i].used)
		{		
            // Check if we have found enough sources
            if (++sourcesFound == requiredSources)
            {
				break;
            }
		}
	}

	// Check if there aren't enough source
    if (sourcesFound != requiredSources)
	{
        // Remove any real channels that were retrieved from "OutputPolled::getFreeChannel"
        while ((*found)--)
        {
            if (realchannel[*found])
            {
                // Remove the flags set by the channel real base code
                realchannel[*found]->mFlags &= ~CHANNELREAL_FLAG_ALLOCATED;
                realchannel[*found]->mFlags &= ~CHANNELREAL_FLAG_IN_USE;
                realchannel[*found]->mFlags |= CHANNELREAL_FLAG_STOPPED;                
                realchannel[*found] = NULL;
            }
        }

        return FMOD_ERR_CHANNEL_ALLOC;
	}

    return FMOD_OK;
}


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
// VERIFY: 9d0778a8 auto
// OFFSET: 0x0092B3F0
FMOD_RESULT F_CALLBACK OutputOpenAL::getNumDriversCallback(FMOD_OUTPUT_STATE *output, int *numdrivers)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->getNumDrivers(numdrivers);
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
// VERIFY: ef4ac265 auto
// OFFSET: 0x0092B430
FMOD_RESULT F_CALLBACK OutputOpenAL::getDriverNameCallback(FMOD_OUTPUT_STATE *output, int id, char *name, int namelen)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->getDriverName(id, name, namelen);
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
// VERIFY: 010c47cd auto
// OFFSET: 0x0092B460
FMOD_RESULT OutputOpenAL::getDriverCapsExCallback(FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode, int *num2dchannels, int *num3dchannels, int *totalchannels)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->getDriverCapsEx(id, caps, minfrequency, maxfrequency, controlpanelspeakermode, num2dchannels, num3dchannels, totalchannels);
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
// VERIFY: 4a60d5c9 auto
// OFFSET: 0x0092C0F0
FMOD_RESULT F_CALLBACK OutputOpenAL::initCallback(FMOD_OUTPUT_STATE *output, int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->init(selecteddriver, flags, outputrate, outputchannels, outputformat, dspbufferlength, dspnumbuffers, extradriverdata);
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
// VERIFY: e21110df manual
// OFFSET: 0x0092B4A0
FMOD_RESULT F_CALLBACK OutputOpenAL::closeCallback(FMOD_OUTPUT_STATE *output)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->close();
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
// VERIFY: 3ccb6c2b manual
// OFFSET: 0x0092B4D0
FMOD_RESULT F_CALLBACK OutputOpenAL::startCallback(FMOD_OUTPUT_STATE *output)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->start();
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
// VERIFY: 262ad209 manual
// OFFSET: 0x0092B500
FMOD_RESULT F_CALLBACK OutputOpenAL::stopCallback(FMOD_OUTPUT_STATE *output)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->stop();
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
// VERIFY: 99fcfcd1 manual
// OFFSET: 0x0092B530
FMOD_RESULT F_CALLBACK OutputOpenAL::updateCallback(FMOD_OUTPUT_STATE *output)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->update();
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
FMOD_RESULT F_CALLBACK OutputOpenAL::getHandleCallback(FMOD_OUTPUT_STATE *output, void **handle)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->getHandle(handle);
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
// VERIFY: c2dceeb0 manual
// OFFSET: 0x0092B560
FMOD_RESULT F_CALLBACK OutputOpenAL::getPositionCallback(FMOD_OUTPUT_STATE *output, unsigned int *pcm)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->getPosition(pcm);
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
// VERIFY: f370d09a auto
// OFFSET: 0x0092B590
FMOD_RESULT F_CALLBACK OutputOpenAL::lockCallback(FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->lock(offset, length, ptr1, ptr2, len1, len2);
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
// VERIFY: f260dfd0 auto
// OFFSET: 0x0092B5E0
FMOD_RESULT F_CALLBACK OutputOpenAL::unlockCallback(FMOD_OUTPUT_STATE *output, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->unlock(ptr1, ptr2, len1, len2);
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
// VERIFY: 574850e8 auto
// OFFSET: 0x0092C130
FMOD_RESULT F_CALLBACK OutputOpenAL::createSampleCallback(FMOD_OUTPUT_STATE *output, FMOD_MODE mode, FMOD_CODEC_WAVEFORMAT *waveformat, Sample **sample)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->createSample(mode, waveformat, sample);
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
// VERIFY: 57257375 auto
// OFFSET: 0x0092B610
FMOD_RESULT F_CALLBACK OutputOpenAL::setReverbPropertiesCallback(FMOD_OUTPUT_STATE *output, const FMOD_REVERB_PROPERTIES *prop)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->setReverbProperties(prop);
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
int F_CALLBACK OutputOpenAL::getSampleMaxChannelsCallback(FMOD_OUTPUT_STATE *output, FMOD_MODE mode, FMOD_SOUND_FORMAT format)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->getSampleMaxChannels(mode, format);
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
// VERIFY: 445be904 manual
// OFFSET: 0x0092B680
FMOD_RESULT F_CALLBACK OutputOpenAL::postMixCallback(FMOD_OUTPUT_STATE *output)
{
    OutputOpenAL *openal = (OutputOpenAL *)output;

    return openal->postMix();
}

}        /* namespace FMOD */

#endif   /* #ifdef FMOD_SUPPORT_OPENAL */
