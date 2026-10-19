/****************************************************************************

*****************************************************************************/ 

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SFXREVERB

#include "fmod.h"
#include "fmod_3d.h"
#include "fmod_dspi.h"
#include "fmod_dsp_sfxreverb.h"
#include "aSfxDsp.hpp"

#ifdef PLATFORM_PS3_SPU
    #include "fmod_systemi_spu.h"
    #include "fmod_spu_printf.h"
#else
    #include "fmod_systemi.h"
#endif

#include <stdio.h>
#include <assert.h>

#ifdef PLATFORM_PS3
extern unsigned int _binary_spu_fmod_dsp_sfxreverb_pic_start[];
#endif

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dspsfxreverb;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetDSPDescription is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_DSP_DESCRIPTION_EX * F_API FMODGetDSPDescriptionEx()
    {
        return DSPSfxReverb::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */

#define FMODLISTENER_MINLFREFERENCE                20.0f
#define FMODLISTENER_MAXLFREFERENCE                1000.0f
#define FMODLISTENER_DEFAULTLFREFERENCE            250.0f

#define FMODLISTENER_MINROOMLF                     -10000
#define FMODLISTENER_MAXROOMLF                     0
#define FMODLISTENER_DEFAULTROOMLF                 0

#ifndef PLATFORM_PS3_SPU
//
// All parameter ranges and defaults are defined in '3dl2.h'
//
FMOD_DSP_PARAMETERDESC dspsfxreverb_param[] =
{
    {-10000.0f, 0.0f, 0.0f, 
        "Dry Level",                 "mB",     "Dry Level" },

    {I3DL2LISTENER_MINROOM, I3DL2LISTENER_MAXROOM, I3DL2LISTENER_DEFAULTROOM, 
        "Room",                 "mB",     "Room" },
        
    {I3DL2LISTENER_MINROOMHF, I3DL2LISTENER_MAXROOMHF, I3DL2LISTENER_DEFAULTROOMHF, 
        "Room HF",              "mB",     "Room HF" },
        
    {I3DL2LISTENER_MINROOMROLLOFFFACTOR, I3DL2LISTENER_MAXROOMROLLOFFFACTOR, I3DL2LISTENER_DEFAULTROOMROLLOFFFACTOR,
        "Room Rolloff",  "",     "Room Rolloff Factor" },
        
    {I3DL2LISTENER_MINDECAYTIME, I3DL2LISTENER_MAXDECAYTIME, I3DL2LISTENER_DEFAULTDECAYTIME,
        "Decay Time",           "s",     "Decay Time" },
        
    {I3DL2LISTENER_MINDECAYHFRATIO,I3DL2LISTENER_MAXDECAYHFRATIO,I3DL2LISTENER_DEFAULTDECAYHFRATIO,
        "Decay HF Ratio",       "",     "Decay HF Ratio" },
        
    { I3DL2LISTENER_MINREFLECTIONS,I3DL2LISTENER_MAXREFLECTIONS ,I3DL2LISTENER_DEFAULTREFLECTIONS,
        "Reflections",          "mB",     "Reflections" },
        
    { I3DL2LISTENER_MINREFLECTIONSDELAY, I3DL2LISTENER_MAXREFLECTIONSDELAY, I3DL2LISTENER_DEFAULTREFLECTIONSDELAY,
        "Reflect Delay",    "",     "Reflections Delay" },
        
    { I3DL2LISTENER_MINREVERB, I3DL2LISTENER_MAXREVERB, I3DL2LISTENER_DEFAULTREVERB,
        "Reverb",               "mB",     "Reverb" },
        
    { I3DL2LISTENER_MINREVERBDELAY, I3DL2LISTENER_MAXREVERBDELAY, I3DL2LISTENER_DEFAULTREVERBDELAY,
        "Reverb Delay",         "s",     "Reverb Delay" },
        
    { I3DL2LISTENER_MINDIFFUSION, I3DL2LISTENER_MAXDIFFUSION, I3DL2LISTENER_DEFAULTDIFFUSION,
        "Diffusion",            "%",     "Diffusion" },
        
    { I3DL2LISTENER_MINDENSITY, I3DL2LISTENER_MAXDENSITY, I3DL2LISTENER_DEFAULTDENSITY,
        "Density",              "%",     "Density" },
        
    { I3DL2LISTENER_MINHFREFERENCE, I3DL2LISTENER_MAXHFREFERENCE , I3DL2LISTENER_DEFAULTHFREFERENCE,
        "HF Reference",         "Hz",     "HF Reference" },

    { FMODLISTENER_MINROOMLF, FMODLISTENER_MAXROOMLF , FMODLISTENER_DEFAULTROOMLF,
        "Room LF",              "mB",     "Room LF" },

    { FMODLISTENER_MINLFREFERENCE, FMODLISTENER_MAXLFREFERENCE , FMODLISTENER_DEFAULTLFREFERENCE,
        "LF Reference",         "Hz",     "LF Reference" }

};

#endif // PLATFORM_PS3_SPU


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: c9b74d0b auto
// OFFSET: 0x008EA410
FMOD_DSP_DESCRIPTION_EX *DSPSfxReverb::getDescriptionEx()
{
#ifndef PLATFORM_PS3_SPU
    FMOD_memset(&dspsfxreverb, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    FMOD_strcpy(dspsfxreverb.name, "SFX Reverb");
    dspsfxreverb.version       = 0x00010100;
    dspsfxreverb.create        = DSPSfxReverb::createCallback;
    dspsfxreverb.release       = DSPSfxReverb::releaseCallback;
    dspsfxreverb.reset         = DSPSfxReverb::resetCallback;

    #ifdef PLATFORM_PS3
    dspsfxreverb.read          = (FMOD_DSP_READCALLBACK)_binary_spu_fmod_dsp_sfxreverb_pic_start;    /* SPU PIC entry address */
    #else
    dspsfxreverb.read          = DSPSfxReverb::readCallback;
    #endif

    dspsfxreverb.numparameters = sizeof(dspsfxreverb_param) / sizeof(dspsfxreverb_param[0]);
    dspsfxreverb.paramdesc     = dspsfxreverb_param;
    dspsfxreverb.setparameter  = DSPSfxReverb::setParameterCallback;
    dspsfxreverb.getparameter  = DSPSfxReverb::getParameterCallback;

    dspsfxreverb.mType         = FMOD_DSP_TYPE_SFXREVERB;
    dspsfxreverb.mCategory     = FMOD_DSP_CATEGORY_FILTER;
    dspsfxreverb.mSize         = sizeof(DSPSfxReverb);
#else
    dspsfxreverb.read          = DSPSfxReverb::readCallback;                  /* We only care about read function on SPU */
#endif

    return &dspsfxreverb;
}

#ifndef PLATFORM_PS3_SPU
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// Room, roomHF, flRoomRolloffFactor, flDecayTime, flDecayHFRatio, lReflections, refDelay, rev, revDelay, diff, dens, flHFReference.


#define FIRELIGHT_OFF \
 -600, 0, 0.0f, 20.0f, 0.1f, 1000, 0.3f, 0, 0.1f, 100.0f, 100.0f, 5000.0f

#define FIRELIGHT_SATURATION \
 -600, 0, 0.0f, 20.0f, 0.1f, 1000, 0.3f, 0000, 0.1f, 100.0f, 100.0f, 5000.0f

#define FIRELIGHT_SATURATION2 \
 -600, 0, 0.0f, 0.1f, 0.1f, 1000, 0.3f, 0000, 0.1f, 100.0f, 100.0f, 5000.0f
 
#define I3DL2_ENVIRONMENT_PRESET_10SEC \
 -600, 0, 0.0f, 10.0f, 1.0f, 0, 0.007f, 0000, 0.011f, 100.0f, 100.0f, 5000.0f

#define I3DL2_ENVIRONMENT_PRESET_20SEC \
 0, 0, 0.0f, 20.0f, 1.0f, 0, 0.007f, 0000, 0.011f, 100.0f, 100.0f, 5000.0f

#define I3DL2_ENVIRONMENT_PRESET_20SEC_FILTERED \
 -600, 0, 0.0f, 20.0f, 0.85f, 0, 0.007f, 0000, 0.011f, 100.0f, 100.0f, 5000.0f

#define I3DL2_ENVIRONMENT_PRESET_60SEC \
 -600, 0, 0.0f, 60.0f, 1.0f, 0, 0.007f, 0000, 0.011f, 100.0f, 100.0f, 5000.0f

#define I3DL2_ENVIRONMENT_PRESET_1000SEC \
 0, 0, 0.0f, 1000.0f, 1.0f, 0, 0.007f, 2000, 0.011f, 100.0f, 100.0f, 5000.0f

#define I3DL2_ENVIRONMENT_PRESET_1000SEC_FILTERED \
 0, 0, 0.0f, 1000.0f, 0.5f, 0, 0.007f, 1000, 0.011f, 100.0f, 100.0f, 1000.0f



/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 9181e009 manual
// OFFSET: 0x008E9EC0
FMOD_RESULT DSPSfxReverb::createInternal()
{
    FMOD_RESULT result;
	I3DL2_LISTENERPROPERTIES props =
    {
        FIRELIGHT_OFF
    };
    int count;

    init();

    mSystem->getSoftwareFormat(&mOutputRate, 0, 0, 0, 0, 0);

    /*
        Initial properties (will get overridden)
    */
    mDryLevelLin    = 0.0f;
    mDryLevelmB     = -100000.0f;
    mRoomLF         = FMODLISTENER_DEFAULTROOMLF;
    mLFReference    = FMODLISTENER_DEFAULTLFREFERENCE;
    mUpdate         = false;

    FMOD_memcpy(&mProps, &props, sizeof(I3DL2_LISTENERPROPERTIES));

    /*
        Create DSP processing object and initialise
    */
    mpSfxDsp = (ASfxDsp *)FMOD_Memory_Alloc(sizeof(ASfxDsp));
    if (!mpSfxDsp)
    {
        return FMOD_ERR_MEMORY;
    }

    if (mpSfxDsp->init((float)mOutputRate))
    {
        return FMOD_ERR_MEMORY;
    }

    mpSfxDsp->UpdateBufferSize(mSystem->mDSPBlockSize);

	mpSfxDsp->mNumLateReverbDelays = kNumLateReverbDelays;
	mpSfxDsp->mNumMatrixStages = int(FMOD_LOG((float)mpSfxDsp->mNumLateReverbDelays) / FMOD_LOG(2.0f) + 0.5f);
    mpSfxDsp->ClearBuffers();

    /*
        Initialise reverb parameters
    */
    for (count = 0; count < mDescription.numparameters; count++)
    {
        result = setParameter(count, mDescription.paramdesc[count].defaultval);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    /*
        Do a forced update here to make sure everything is calculated correctly first.
    */
    mLFReference = mLFReferenceUpdate;
    FMOD_memcpy(&mProps, &mUpdateProps, sizeof(I3DL2_LISTENERPROPERTIES));
    mRoomLF      = mRoomLFUpdate;
    mUpdate      = false;

    SetRoom(&mProps);
    SetRoomHF(&mProps);
    SetDecayTime(&mProps);
    SetDecayHFRatio(&mProps);
    SetReflectionsLevel(&mProps);
    SetReflectionsDelay(&mProps);
    SetReverbLevel(&mProps);
    SetReverbDelay(&mProps);
    SetDiffusion(&mProps);
    SetDensity(&mProps);
    SetHFReference(&mProps);
    SetRoomLF(mRoomLF);
    SetLFReference(mLFReference);

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPSfxReverb::releaseInternal()
{
    mpSfxDsp->close();

    FMOD_Memory_Free(mpSfxDsp);
    mpSfxDsp = 0;

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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPSfxReverb::resetInternal()
{
    mpSfxDsp->UpdateBufferSize(1024);

    mpSfxDsp->ClearBuffers();

    return FMOD_OK;
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 88abcc14 auto
// OFFSET: 0x008EA170
FMOD_RESULT DSPSfxReverb::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    if (mUpdate)
    {
        if (mProps.lRoom != mUpdateProps.lRoom)
        {
            mProps.lRoom = mUpdateProps.lRoom;
            SetRoom(&mProps);
        }

        if (mProps.lRoomHF != mUpdateProps.lRoomHF)
        {
            mProps.lRoomHF = mUpdateProps.lRoomHF;
            SetRoomHF(&mProps);
        }

        if (mProps.flRoomRolloffFactor != mUpdateProps.flRoomRolloffFactor)
        {
            mProps.flRoomRolloffFactor = mUpdateProps.flRoomRolloffFactor;
        }

        if (mProps.flDecayTime != mUpdateProps.flDecayTime)
        {
            mProps.flDecayTime = mUpdateProps.flDecayTime;
            SetDecayTime(&mProps);
        }

        if (mProps.flDecayHFRatio != mUpdateProps.flDecayHFRatio)
        {
            mProps.flDecayHFRatio = mUpdateProps.flDecayHFRatio;
            SetDecayHFRatio(&mProps);
        }

        if (mProps.lReflections != mUpdateProps.lReflections)
        {
            mProps.lReflections = mUpdateProps.lReflections;
            SetReflectionsLevel(&mProps);
        }

        if (mProps.flReflectionsDelay != mUpdateProps.flReflectionsDelay)
        {
            mProps.flReflectionsDelay = mUpdateProps.flReflectionsDelay;
            SetReflectionsDelay(&mProps);
        }

        if (mProps.lReverb != mUpdateProps.lReverb)
        {
            mProps.lReverb = mUpdateProps.lReverb;
            SetReverbLevel(&mProps);
        }

        if (mProps.flReverbDelay != mUpdateProps.flReverbDelay)
        {
            mProps.flReverbDelay = mUpdateProps.flReverbDelay;
            SetReverbDelay(&mProps);
        }

        if (mProps.flDiffusion != mUpdateProps.flDiffusion)
        {
            mProps.flDiffusion = mUpdateProps.flDiffusion;
            SetDiffusion(&mProps);
        }

        if (mProps.flDensity != mUpdateProps.flDensity)
        {
            mProps.flDensity = mUpdateProps.flDensity;
            SetDensity(&mProps);
        }

        if (mProps.flHFReference != mUpdateProps.flHFReference)
        {
            mProps.flHFReference = mUpdateProps.flHFReference;
            SetHFReference(&mProps);
        }

        if (mRoomLF != mRoomLFUpdate)
        {
            mRoomLF = mRoomLFUpdate;
            SetRoomLF(mRoomLF);
        }

        if (mLFReference != mLFReferenceUpdate)
        {
            mLFReference = mLFReferenceUpdate;
            SetLFReference(mLFReference);
        }

        mUpdate = false;
    }

	mpSfxDsp->DoDSPProcessing(inbuffer, outbuffer, (int)inchannels, (unsigned int)length, (float)mOutputRate, mDryLevelLin);

    return FMOD_OK;
}


#ifndef PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: e791a70f auto
// OFFSET: 0x008E8FB0
FMOD_RESULT DSPSfxReverb::setParameterInternal(int index, float value)
{
    FMOD_RESULT result = FMOD_OK;

    switch (index)
    {
        case FMOD_DSP_SFXREVERB_DRYLEVEL:
        {
            mDryLevelmB = value;
            mDryLevelLin = (float)FMOD_POW(10.0f, mDryLevelmB / 2000.0f);
            break;
        }
        case FMOD_DSP_SFXREVERB_ROOM:
        {
            mUpdateProps.lRoom = (int)(value + 0.5f);
            mUpdate = true;
            break;
        }
        case FMOD_DSP_SFXREVERB_ROOMHF:
        {
            mUpdateProps.lRoomHF = (int)(value + 0.5f);
            mUpdate = true;
            break;
        }
        case FMOD_DSP_SFXREVERB_ROOMROLLOFFFACTOR:
        {
            mUpdateProps.flRoomRolloffFactor = value;
            mUpdate = true;
            break;
        }
        case FMOD_DSP_SFXREVERB_DECAYTIME:
        {
            mUpdateProps.flDecayTime = value;
            mUpdate = true;
            break;
        }
        case FMOD_DSP_SFXREVERB_DECAYHFRATIO:
        {
            mUpdateProps.flDecayHFRatio = value;
            mUpdate = true;
            break;
        }        
        case FMOD_DSP_SFXREVERB_REFLECTIONSLEVEL:
        {
            mUpdateProps.lReflections = (int)(value + 0.5f); 
            mUpdate = true;
            break;
        }        
        case FMOD_DSP_SFXREVERB_REFLECTIONSDELAY:
        {
            mUpdateProps.flReflectionsDelay = value;
            mUpdate = true;
            break;
        }        
        case FMOD_DSP_SFXREVERB_REVERBLEVEL:
        {
            mUpdateProps.lReverb = (int)(value + 0.5f); 
            mUpdate = true;
            break;
        }        
        case FMOD_DSP_SFXREVERB_REVERBDELAY:
        {
            mUpdateProps.flReverbDelay = value;
            mUpdate = true;
            break;
        }        
        case FMOD_DSP_SFXREVERB_DIFFUSION:
        {
            mUpdateProps.flDiffusion = value;
            mUpdate = true;
            break;
        }        
        case FMOD_DSP_SFXREVERB_DENSITY:
        {
            mUpdateProps.flDensity = value;
            mUpdate = true;
            break;
        }        
        case FMOD_DSP_SFXREVERB_HFREFERENCE:
        {
            mUpdateProps.flHFReference = value;
            mUpdate = true;
            break;
        }
        case FMOD_DSP_SFXREVERB_ROOMLF:
        {            
            mRoomLFUpdate = (int)(value + 0.5f);
            mUpdate = true;
            break;
        }
        case FMOD_DSP_SFXREVERB_LFREFERENCE:
        {
            mLFReferenceUpdate = value;
            mUpdate = true;
            break;
        }
        default:
        {
           result = FMOD_ERR_INVALID_PARAM;
           break;
        } 
    }

    return result;
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 3b5dbf48 auto
// OFFSET: 0x008E91D0
FMOD_RESULT DSPSfxReverb::getParameterInternal(int index, float *value, char *valuestr)
{

    switch (index)
    {
        case FMOD_DSP_SFXREVERB_DRYLEVEL:
        {
            if (value)
                *value = mDryLevelmB;
            if (valuestr)
                sprintf(valuestr, "%f", mDryLevelmB);
            break;
        }
        case FMOD_DSP_SFXREVERB_ROOM:
        {
            if (value)
                *value = (float)mProps.lRoom;
            if (valuestr)
                sprintf(valuestr, "%d", (int)mProps.lRoom);
            break;
        }
        case FMOD_DSP_SFXREVERB_ROOMHF:
        {            
            if (value)
                *value = (float)mProps.lRoomHF;
            if (valuestr)
                sprintf(valuestr, "%d", (int)mProps.lRoomHF);
            break;
        }
        case FMOD_DSP_SFXREVERB_ROOMROLLOFFFACTOR:
        {
            if (value)
                *value = (float)mProps.flRoomRolloffFactor;
            if (valuestr)
                sprintf(valuestr, "%f", mProps.flRoomRolloffFactor);
            break;
        }
        case FMOD_DSP_SFXREVERB_DECAYTIME:
        {
            if (value)
                *value = (float)mProps.flDecayTime;
            if (valuestr)
                sprintf(valuestr, "%f", mProps.flDecayTime);
            break;
        }
        case FMOD_DSP_SFXREVERB_DECAYHFRATIO:
        {
            if (value)
                *value = (float)mProps.flDecayHFRatio;
            if (valuestr)
                sprintf(valuestr, "%f", mProps.flDecayHFRatio);
            break;
        }        
        case FMOD_DSP_SFXREVERB_REFLECTIONSLEVEL:
        {
            if (value)
                *value = (float)mProps.lReflections;
            if (valuestr)
                sprintf(valuestr, "%d", (int)mProps.lReflections);
            break;
        }        
        case FMOD_DSP_SFXREVERB_REFLECTIONSDELAY:
        {
            if (value)
                *value = (float)mProps.flReflectionsDelay;
            if (valuestr)
                sprintf(valuestr, "%f", mProps.flReflectionsDelay);
            break;
        }        
        case FMOD_DSP_SFXREVERB_REVERBLEVEL:
        {
            if (value)
                *value = (float)mProps.lReverb;
            if (valuestr)
                sprintf(valuestr, "%d", (int)mProps.lReverb);
            break;
        }        
        case FMOD_DSP_SFXREVERB_REVERBDELAY:
        {
            if (value)
                *value = (float)mProps.flReverbDelay;
            if (valuestr)
                sprintf(valuestr, "%f", mProps.flReverbDelay);
            break;
        }        
        case FMOD_DSP_SFXREVERB_DIFFUSION:
        {
            if (value)
                *value = (float)mProps.flDiffusion;
            if (valuestr)
                sprintf(valuestr, "%f", mProps.flDiffusion);
            break;
        }        
        case FMOD_DSP_SFXREVERB_DENSITY:
        {
            if (value)
                *value = (float)mProps.flDensity;
            if (valuestr)
                sprintf(valuestr, "%f", mProps.flDensity);
            break;
        }        
        case FMOD_DSP_SFXREVERB_HFREFERENCE:
        {
            if (value)
                *value = (float)mProps.flHFReference;
            if (valuestr)
                sprintf(valuestr, "%f", mProps.flHFReference);
            break;
        }
        case FMOD_DSP_SFXREVERB_ROOMLF:
        {            
            if (value)
                *value = (float)mRoomLF;
            if (valuestr)
                sprintf(valuestr, "%d", (int)mRoomLF);
            break;
        }
        case FMOD_DSP_SFXREVERB_LFREFERENCE:
        {
            if (value)
                *value = mLFReference;
            if (valuestr)
                sprintf(valuestr, "%f", mLFReference);
            break;
        }
        default:
        {
           return FMOD_ERR_INVALID_PARAM;
           break;
        }
    }
    return FMOD_OK;
}

// Adjust intensity level for the room effect (i.e. both the early reflections and the late reverberation).
// VERIFY: a77396c4 auto
// OFFSET: 0x008E98E0
bool DSPSfxReverb::SetRoom(I3DL2_LISTENERPROPERTIES *pProps)					    // [-10000, 0]      default: -10000 mB
{
	if (pProps->lRoom < I3DL2LISTENER_MINROOM)
		pProps->lRoom = I3DL2LISTENER_MINROOM;
	else if (pProps->lRoom > I3DL2LISTENER_MAXROOM)
		pProps->lRoom = I3DL2LISTENER_MAXROOM;
		
	mProps.lRoom = pProps->lRoom;
    bool err = SetReflectionsLevel(pProps);
	err |= SetReverbLevel(pProps);
	
	return err;
}

// Adjust low-pass filter for the room effect (i.e. both the early reflections and the late reverberation).
// Room_HF is defined as the attenuation at high frequencies relative to the intensity at low frequencies.
// VERIFY: 09415751 auto
// OFFSET: 0x008E9930
bool DSPSfxReverb::SetRoomHF(I3DL2_LISTENERPROPERTIES *pProps)						// [-10000, 0]      default: 0 mB
{	
	if (pProps->lRoomHF  < I3DL2LISTENER_MINROOMHF)
		pProps->lRoomHF = I3DL2LISTENER_MINROOMHF;
	else if (pProps->lRoomHF > I3DL2LISTENER_MAXROOMHF)
		pProps->lRoomHF = I3DL2LISTENER_MAXROOMHF;
		
	mProps.lRoomHF = pProps->lRoomHF;
    
    float gainDB = (float)pProps->lRoomHF * 0.01f / 2.0f;	// Convert to dB. 
											// 2 filters in series, so each has half the attenuation
	float gain = (float)FMOD_POW(10.0f, gainDB / 20.0f);				// Convert to linear
	
	float K;
	
	bool err = Calculate1stOrderLowpassCoeff(gain, pProps->flHFReference, (float)mOutputRate, &K);
	mpSfxDsp->mRoomHF = 1.0f - K;
	
	return err;
}

// VERIFY: 623b095a manual
// OFFSET: 0x008E94B0
bool DSPSfxReverb::Calculate1stOrderLowpassCoeff(float gain, float cutoff, float sampleRate, float *pK)
{

    // *** For now, limit the gain to 1 ***
	if (gain > 1.0f)
	{
		gain = 1.0f;
	}
	
	// But b24ac becomes NAN...
	if (gain == 1.0f)
	{
		*pK = 0.0f;
	}
	else
	{
		assert(sampleRate);
		float Gain = gain * gain;							// gain squared
		
		float a = Gain - 1.0f;
		float b = 2.0f * (1.0f - Gain * (float)FMOD_COS(2.0f * FMOD_PI * cutoff / sampleRate));
		float c = a;
		
		float b24ac = b*b - 4.0f*a*c;
		
		// assert(b24ac >= 0);
        if (b24ac < 0)
            b24ac = 0; 
		
		*pK = (-b + (float)FMOD_SQRT(b24ac)) / (2.0f * a);
		if (*pK < 0.0f)
		{
			*pK = 0.0f;
		}
		else if (*pK > 0.99f)
		{
			*pK = 0.99f;
		}
	}
	
	return 0;
}

// Adjust low-pass filter for the room effect (i.e. both the early reflections and the late reverberation).
// Room_HF is defined as the attenuation at high frequencies relative to the intensity at low frequencies.
// VERIFY: e6d2e231 auto
// OFFSET: 0x008E99C0
bool DSPSfxReverb::SetRoomLF(int roomLF)						// [-10000, 0]      default: 0 mB
{
	if (roomLF < FMODLISTENER_MINROOMLF)
    {
		roomLF = FMODLISTENER_MINROOMLF;
    }
	else if (roomLF > FMODLISTENER_MAXROOMLF)
    {
        roomLF = FMODLISTENER_MAXROOMLF;
    }

    mRoomLF = roomLF;
    mpSfxDsp->mRoomLF = roomLF / 100.0f;

	bool err = CalculateShelfCoeffs(mpSfxDsp->mRoomLF , mLFReference, (float)mOutputRate,
                                    &mpSfxDsp->mRoomLFcoeffs.a0, &mpSfxDsp->mRoomLFcoeffs.a1, &mpSfxDsp->mRoomLFcoeffs.a2,
                                    &mpSfxDsp->mRoomLFcoeffs.b1, &mpSfxDsp->mRoomLFcoeffs.b2);
	return err;
}


// VERIFY: cf4ae94a auto
// OFFSET: 0x008E9560
bool DSPSfxReverb::CalculateShelfCoeffs(float gain, float cutoff, float sampleRate, float *a0, float *a1, float *a2, float *b1, float *b2)
{
    float t1, t2, g1, g2;

    float sqrtgainlin = FMOD_EXP(gain * 0.057564627f);
    t1 = t2 = FMOD_TAN(3.14159265f * cutoff / sampleRate);
    g1 = g2 = 1.414213562f; 
    t2 /= sqrtgainlin;

    float denom = 1.0f / (1.0f + t2 * (g2 + t2));
    *a0 = (1.0f + t1 * (t1 + g1)) * denom;
    *a1 = (t1 * t1 - 1.0f) * denom * 2.0f;
    *a2 = (1.0f + t1 * (t1 - g1)) * denom;
    *b1 = -2.0f * (t2 * t2 - 1.0f) * denom;
    *b2 = -(1.0f + t2 * (t2 - g2)) * denom;

    return 0;
}

// Controls the rolloff of room effect intensity vs. distance, in the same way that DS3D's Rolloff Factor
// operates on the direct path intensity. Default is 0.0, implying that the reverberation intensity does not
// depend on source-listener distance. A value of 1.0 means that the reverberation decays by 6 dB per doubling
// of distance.
bool DSPSfxReverb::SetRoomRolloffFactor(I3DL2_LISTENERPROPERTIES *pProps)     // [0.0, 10.0]      default: 0.0
{
	mProps.flRoomRolloffFactor = pProps->flRoomRolloffFactor;
    return 0;
}

#define	NEW_MAX_DECAY_TIME	10000	// Change this to match the I3DL2 spec.
	
// Reverberation decay time
// VERIFY: bd981581 auto
// OFFSET: 0x008E9A40
bool DSPSfxReverb::SetDecayTime(I3DL2_LISTENERPROPERTIES *pProps)				// [0.1, 20.0]      default: 1.0 s
{
	float K;
	bool err = false;
    // err = SetDelayLineLengths(pProps);		// *** Put this somewhere else, unless it will change for different decay times.
	int delayLine;
	
	if (pProps->flDecayTime< I3DL2LISTENER_MINDECAYTIME)
	{
		pProps->flDecayTime = I3DL2LISTENER_MINDECAYTIME;
	}
	else if (pProps->flDecayTime > NEW_MAX_DECAY_TIME)	
	{
		pProps->flDecayTime = NEW_MAX_DECAY_TIME;
	}

	mProps.flDecayTime = pProps->flDecayTime;	
    
	for (delayLine=0; delayLine<mpSfxDsp->mNumLateReverbDelays; delayLine++)
	{
		if (pProps->flDecayTime)
		{
			// At low frequencies, each delay line should decay by 60 dB in flDecayTime seconds.
			// Attenuation each time through the delay line
			float attenuationDB = (-60.f / pProps->flDecayTime) * mpSfxDsp->mLateDelayLenSec[delayLine];

			// Convert to linear
			float feedback = (float)FMOD_POW(10.0f, attenuationDB / 20.0f);
			mpSfxDsp->mFeedback[delayLine] = feedback;

			// Also handle HF decay
			// At the HF reference frequency, each delay line should decay by 60 dB in flDecayHFRatio * flDecayTime seconds.
			float attenuationDB_HF = (-60.0f / (pProps->flDecayHFRatio * pProps->flDecayTime)) * mpSfxDsp->mLateDelayLenSec[delayLine];

			// Subtract the DC attenuation to get the filter attenuation
			attenuationDB_HF -= attenuationDB;

			// Convert to linear
			float attenuation_HF = (float)FMOD_POW(10.0f, attenuationDB_HF / 20.0f);

			err |= Calculate1stOrderLowpassCoeff(attenuation_HF, pProps->flHFReference, (float)mOutputRate, &K);
			mpSfxDsp->mDecayHF[delayLine] = 1.0f - K;
		}
	}
    // NEW
    SetReverbLevel(&mProps);	// Adjust the reverb gain to correspond to the new decay time, 
							    // to keep the energy constant.
    // \NEW
	/*

Calculate1stOrderLowpassCoeff: b=2.000000, b24ac=-0.000000 a=-1.000000
Calculate1stOrderLowpassCoeff(0.000037, 5000.000000, 48000.000000)
-> attenuationDB = -9.843841
-> feedback = 0.321965
-> attenuationDB_HF = -88.594559
-> K = -1.#IND00
*/
	return err;
}

/*
#define LOWEST_DELAY_LINE_LENGTH_SEC		(0.061f)
#define	DELAY_LINE_RATIO					(1.32f)		
#define LOWEST_SHORT_DELAY_LINE_LENGTH_SEC	(0.0017f)
#define	SHORT_DELAY_LINE_RATIO					(1.61f)
*/

// VERIFY: 34087620 auto
// OFFSET: 0x008E9B90
bool DSPSfxReverb::SetDelayLineLengths(I3DL2_LISTENERPROPERTIES *pProps)
{
	/*
	mpSfxDsp->SetLateDelays(LOWEST_DELAY_LINE_LENGTH_SEC, DELAY_LINE_RATIO, 
							LOWEST_DELAY_LINE_LENGTH_B_SEC, DELAY_LINE_RATIO_B, 
							(float)mOutputRate);
    */

	float density = 0.01f * pProps->flDensity;	// 0 ~ 1.
	
	density *= (density * density);
		
	// Modal density ~= 4pi*V*f^2 / c^3 (modes per Hz)
	// c = 343 m/sec
	// If f = 1000,
		// If V = 44m x 25m x 17m = 18,700 m^3, MD = 5823.
		// If V = 2m x 2m x 2m = 8 m^3, MD = 2.49
		// Ratio ~= 2338
	// If f = 100,
		// If V = 18,700 m^3, MD = 58.23
		// If V = 8 m^3, MD = 0.025
		// Ratio ~= 2337.
		
	// Map flDensity 100 to MD 1.0
	// Map flDensity 0 to MD 0.2
#define	MIN_DENS	(0.1f)
#define DENS_SCAL	(1.0f - MIN_DENS)
	density *= DENS_SCAL;
	density += MIN_DENS;
	
	if (density < MIN_DENS)
	{
		density = MIN_DENS;
	}
	else if (density > 1.0f)
	{
		density = 1.0f;
	}
	
	float	lowestDelayLenSec = LOWEST_DELAY_LINE_LENGTH_SEC * density;
	
	mpSfxDsp->SetLateDelays(lowestDelayLenSec, DELAY_LINE_RATIO, 
							LOWEST_DELAY_LINE_LENGTH_B_SEC, DELAY_LINE_RATIO_B, 
							(float)mOutputRate);
	
	bool err = SetDecayTime(pProps);	// Make sure it takes into account the new delay lengths
	
	return err;
}

// Ratio of high-frequency decay time relative to low-frequency decay time. Decay_HF_Ratio would typically
// control one or several filters located in the feedback loop of the reverberator, whereas Room_HF should
// control a filter located at the input or output of the reverberator (outside of the feedback loop).
// VERIFY: 7fc22fd0 manual
// OFFSET: 0x008E9C20
bool DSPSfxReverb::SetDecayHFRatio(I3DL2_LISTENERPROPERTIES *pProps)		// [0.1, 2.0]       default: 0.5
{
	if (pProps->flDecayHFRatio < I3DL2LISTENER_MINDECAYHFRATIO)
		pProps->flDecayHFRatio = I3DL2LISTENER_MINDECAYHFRATIO;
	else if (pProps->flDecayHFRatio > I3DL2LISTENER_MAXDECAYHFRATIO)	// Do we support >1? 
		pProps->flDecayHFRatio = I3DL2LISTENER_MAXDECAYHFRATIO;

	mProps.flDecayHFRatio = pProps->flDecayHFRatio;

	bool err = SetDecayTime(pProps);		// This also handles HF decay
	
	return err;
}

// Adjusts intensity level of early reflections (relative to Room value)
// VERIFY: 13a06b59 auto
// OFFSET: 0x008E9610
bool DSPSfxReverb::SetReflectionsLevel(I3DL2_LISTENERPROPERTIES *pProps)		// [-10000, 1000]   default: -10000 mB
{
	if (pProps->lReflections < I3DL2LISTENER_MINREFLECTIONS)
		pProps->lReflections = I3DL2LISTENER_MINREFLECTIONS;
	else if (pProps->lReflections > I3DL2LISTENER_MAXREFLECTIONS)	
		pProps->lReflections = I3DL2LISTENER_MAXREFLECTIONS;

	mProps.lReflections = pProps->lReflections;

	float reflectionsLevelDB = (float)(pProps->lReflections + pProps->lRoom) * 0.01f;	// Add Room level, convert to dB

	// Convert to linear
	float ampGain = FMOD_POW(10.0f, reflectionsLevelDB / 20.0f);		
	
	// If we have 8 outputs, scale the gain by sqrt(1/8).
	mpSfxDsp->mERgain = ampGain * FMOD_SQRT(1.0f/(float)(kNumEarlyDelayTaps + 1.0f));

	return 0;
}

// Delay time of the first reflection (relative to the direct path)
// VERIFY: f998db10 auto
// OFFSET: 0x008E9C80
bool DSPSfxReverb::SetReflectionsDelay(I3DL2_LISTENERPROPERTIES *pProps)	// [0.0, 0.3]       default: 0.02 s
{
	if (pProps->flReflectionsDelay < I3DL2LISTENER_MINREFLECTIONSDELAY)
		pProps->flReflectionsDelay = I3DL2LISTENER_MINREFLECTIONSDELAY;
	else if (pProps->flReflectionsDelay > I3DL2LISTENER_MAXREFLECTIONSDELAY)	
		pProps->flReflectionsDelay = I3DL2LISTENER_MAXREFLECTIONSDELAY;

    mProps.flReflectionsDelay = pProps->flReflectionsDelay;
	mpSfxDsp->mEarlyLateSec[0] = mProps.flReflectionsDelay;
	
	int refDelaySamples = (int)((float)mOutputRate * mProps.flReflectionsDelay);
	
	if (refDelaySamples == 0)
	{
		refDelaySamples++;		// Avoid long wrap-around delay
	}
	
	mpSfxDsp->mEarlyLateSamples[0] = refDelaySamples;
	
	SetReverbDelay(&mProps);		// This affects the absolute position of the reverb delay.
	
	return 0;
}

// Adjusts intensity of late reverberation (relative to Room value)
// Note: Reverb and Decay_time are independent. If the application adjusts Decay_time without changing
// Reverb, the driver must keep the intensity of the late reverberation constant.
// VERIFY: b1c81df7 auto
// OFFSET: 0x008E9690
bool DSPSfxReverb::SetReverbLevel(I3DL2_LISTENERPROPERTIES *pProps)					// [-10000, 2000]   default: -10000 mB
{
	if (pProps->lReverb < I3DL2LISTENER_MINREVERB)
		pProps->lReverb = I3DL2LISTENER_MINREVERB;
	else if (pProps->lReverb > I3DL2LISTENER_MAXREVERB)	
		pProps->lReverb = I3DL2LISTENER_MAXREVERB;

	mProps.lReverb = pProps->lReverb;
    
    float	reverbLevelDB = (float)(pProps->lReverb + pProps->lRoom) * 0.01f;	// Add room level, convert to dB
	float	reverbGain = (float)FMOD_POW(10.0f, reverbLevelDB / 20.0f);						// convert to linear
	int		delayLine;
	float	powerGain;
	
	// Adjust this to compensate for the gain of the late reverb loop.
	// First, calculate the average feedback power gain, and the average scatter power gain:
	float aveFeedbackPowerGain = 0;
	
	for (delayLine = 0; delayLine<mpSfxDsp->mNumLateReverbDelays; delayLine++)
	{
		float feedbackGain = mpSfxDsp->mFeedback[delayLine];
		aveFeedbackPowerGain += (feedbackGain * feedbackGain); 
	}
	
	aveFeedbackPowerGain /= mpSfxDsp->mNumLateReverbDelays;
	
	// Power gain = power gain of first pass + loop gain
	// = 1 + Feedback / (1-Feedback)
	if (aveFeedbackPowerGain != 1.0f)
	{
		powerGain = 1.0f + (aveFeedbackPowerGain / (1.0f - aveFeedbackPowerGain));
	}
	else
	{
		powerGain = 1.0f; 
	}
	
    float ampGain;

	// Adjust reverbGain by the amplitude gain of the late feedback loop
	if (powerGain > 0.0f)
	{
		ampGain = reverbGain / (float)FMOD_SQRT(powerGain);
	}
    else
    {
        ampGain = reverbGain;
    }
	
	// If we have 8 outs, scale gain by sqrt(1/8)
	mpSfxDsp->mLRgain = ampGain * FMOD_SQRT(1.0/(float)kNumLateReverbDelays);
	
	return 0;
}

// Defines the time limit between the early reflections and the late reverberation (relative to the time
// of the first reflection).
// VERIFY: d7a71fe5 auto
// OFFSET: 0x008E9770
bool DSPSfxReverb::SetReverbDelay(I3DL2_LISTENERPROPERTIES *pProps)				// [0.0, 0.1]       default: 0.04 s
{
	if (pProps->flReverbDelay < I3DL2LISTENER_MINREVERBDELAY)
		pProps->flReverbDelay = I3DL2LISTENER_MINREVERBDELAY;
	else if (pProps->flReverbDelay > I3DL2LISTENER_MAXREVERBDELAY)	
		pProps->flReverbDelay = I3DL2LISTENER_MAXREVERBDELAY;

	mProps.flReverbDelay = pProps->flReverbDelay;
	
	float	refPlusRevDelaySec = mProps.flReflectionsDelay + mProps.flReverbDelay;

	mpSfxDsp->SetLate_EarlyLateDelayTaps(refPlusRevDelaySec, kEarlyLateNextLengthSec, kEarlyLateDelayRatio, (float)mOutputRate);

	return 0;
}

// Controls the echo density in the late reverberation decay. 0% = minimum, 100% = maximum.
// VERIFY: 496634b0 manual
// OFFSET: 0x008E9D10
bool DSPSfxReverb::SetDiffusion(I3DL2_LISTENERPROPERTIES *pProps)					// [0.0, 100.0]     default: 100.0 %
{
	if (pProps->flDiffusion < I3DL2LISTENER_MINDIFFUSION)
		pProps->flDiffusion = I3DL2LISTENER_MINDIFFUSION;
	else if (pProps->flDiffusion > I3DL2LISTENER_MAXDIFFUSION)	
		pProps->flDiffusion = I3DL2LISTENER_MAXDIFFUSION;

	mProps.flDiffusion = pProps->flDiffusion;
    
    // Set the gains inside the matrix
	float diffusion = pProps->flDiffusion * 0.01f;
	diffusion = diffusion * FMOD_PI / 4.0f;
	mpSfxDsp->mHadamard = (float)FMOD_TAN(diffusion);
	
	// Compensate for the matrix gain
	// A ----------cos() ----------- C
	//			   \  / sin()
	//			    \/
	//			   /  \ -sin()
	// B --------- cos() ----------- D
	
	// ->
	// A --- cos() ----------- C
	//			   \  / tan
	//			    \/
	//			   /  \ -tan
	// B --- cos() ----------- D
	
	// C = A cos(th) + B sin(th) = A cos(th) + B cos(th)tan(th)
	// C = B cos(th) - A sin(th) = B cos(th) - A cos(th)tan(th)
	
	// If C = A * diffScale + (B * diffScale) * Hadamard, and
	//    D = B * diffScale - (A * diffScale) * Hadamard,
	// If A = B = 1, C^2 + D^2 = 2. Then
	// C^2 = diffScale^2 * (1 + 2*Hadamard + Hadamard^2), and
	// D^2 = diffScale^2 * (1 - 2*Hadamard + Hadamard^2). Then
	// 1 = diffScale^2 * (1 + Hadamard^2)
	// For full diffusion, Hadamard = 1; diff = 0.707 = cos(PI/4) (sinusoidal rotation matrix)
	// Apply this gain for each stage of the matrix.
	float diffusionScale = (float)FMOD_POW(FMOD_COS(diffusion), mpSfxDsp->mNumMatrixStages); 
	mpSfxDsp->mDiffusionScale = diffusionScale;		// * diffusionScale;	// 2 matrices

	return 0;
}

// Controls the modal density in the late reverberation decay. 0% = minimum, 100% = maximum.
// VERIFY: bd304034 manual
// OFFSET: 0x008E9DC0
bool DSPSfxReverb::SetDensity(I3DL2_LISTENERPROPERTIES *pProps)						// [0.0, 100.0]     default: 100.0 %
{
	if (pProps->flDensity < I3DL2LISTENER_MINDENSITY)
		pProps->flDensity = I3DL2LISTENER_MINDENSITY;
	else if (pProps->flDensity > I3DL2LISTENER_MAXDENSITY)	
		pProps->flDensity = I3DL2LISTENER_MAXDENSITY;

	mProps.flDensity = pProps->flDensity;

	bool err = SetDelayLineLengths(pProps);	

	return err;
}

// Reference high frequency.
// VERIFY: 5611bf94 auto
// OFFSET: 0x008E9E10
bool DSPSfxReverb::SetHFReference(I3DL2_LISTENERPROPERTIES *pProps)				// [20.0, 20000.0]  default: 5000.0 Hz
{
	if (pProps->flHFReference < I3DL2LISTENER_MINHFREFERENCE)
		pProps->flHFReference = I3DL2LISTENER_MINHFREFERENCE;
	else if (pProps->flHFReference > I3DL2LISTENER_MAXHFREFERENCE)	
		pProps->flHFReference = I3DL2LISTENER_MAXHFREFERENCE;

	mProps.flHFReference = pProps->flHFReference;
    SetRoomHF(&mProps);
    SetDecayTime(&mProps);	
    return 0;
}

// Reference low frequency.
// VERIFY: 6b529cdf auto
// OFFSET: 0x008E9E70
bool DSPSfxReverb::SetLFReference(float lfReference)				// [20.0, 20000.0]  default: 5000.0 Hz
{
	if (lfReference < FMODLISTENER_MINLFREFERENCE)
    {
		lfReference = FMODLISTENER_MINLFREFERENCE;
    }
	else if (lfReference > FMODLISTENER_MAXLFREFERENCE)
    {
		lfReference = FMODLISTENER_MAXLFREFERENCE;
    }

    mLFReference = lfReference;

    SetRoomLF(mRoomLF);
    return 0;
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


#endif

/*
    ==============================================================================================================

    CALLBACK INTERFACE

    ==============================================================================================================
*/


#ifndef PLATFORM_PS3_SPU
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: c58eba0d manual
// OFFSET: 0x008EA3B0
FMOD_RESULT F_CALLBACK DSPSfxReverb::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPSfxReverb *sfxreverb = (DSPSfxReverb *)dsp;

    return sfxreverb->createInternal();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 04480fa6 manual
// OFFSET: 0x008E97E0
FMOD_RESULT F_CALLBACK DSPSfxReverb::releaseCallback(FMOD_DSP_STATE *dsp)
{
    DSPSfxReverb *sfxreverb = (DSPSfxReverb *)dsp;

    return sfxreverb->releaseInternal();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 0f12bfba manual
// OFFSET: 0x008E9830
FMOD_RESULT F_CALLBACK DSPSfxReverb::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPSfxReverb *sfxreverb = (DSPSfxReverb *)dsp;

    return sfxreverb->resetInternal();
}
#endif


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 3f4270f0 auto
// OFFSET: 0x008EA3E0
FMOD_RESULT F_CALLBACK DSPSfxReverb::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPSfxReverb *sfxreverb = (DSPSfxReverb *)dsp;

    return sfxreverb->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
}


#ifndef PLATFORM_PS3_SPU
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: bb401184 manual
// OFFSET: 0x008E9870
FMOD_RESULT F_CALLBACK DSPSfxReverb::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPSfxReverb *sfxreverb = (DSPSfxReverb *)dsp;

    return sfxreverb->setParameterInternal(index, value);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 00c65d10 auto
// OFFSET: 0x008E98B0
FMOD_RESULT F_CALLBACK DSPSfxReverb::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPSfxReverb *sfxreverb = (DSPSfxReverb *)dsp;

    return sfxreverb->getParameterInternal(index, value, valuestr);
}


#endif

}

#endif
