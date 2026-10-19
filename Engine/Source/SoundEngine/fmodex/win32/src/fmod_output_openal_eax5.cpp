#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL
#ifdef FMOD_SUPPORT_EAX

#include "fmod_output_openal.h"
#include "fmod_eax5.h"

namespace FMOD
{

bool gEAX5LockedOALFxSlot0 = false;
bool gEAX5LockedOALFxSlot1 = false;
bool gEAX5LockedOALFxSlot2 = false;
bool gEAX5LockedOALFxSlot3 = false;

/*
[
	[DESCRIPTION]
    Create an EAX5 session

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]
    A session must be created before using EAX5

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 9c599770 auto
// OFFSET: 0x0094A100
FMOD_RESULT	OutputOpenAL::setupSessionEAX5()
{
    GUID                  guid;
    EAXSESSIONPROPERTIES  eaxSession;
    
    eaxSession.ulEAXVersion = EAX_50;
    eaxSession.ulMaxActiveSends = 2;
    FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX50_Context, sizeof(GUID));

    mEAXSet(&guid, EAXCONTEXT_EAXSESSION, 0, &eaxSession, sizeof(EAXSESSIONPROPERTIES));
    if (mOALFnTable.alGetError() != AL_NO_ERROR)
    {
		return FMOD_ERR_INTERNAL;
    }

	return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Get the current speaker mode

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]
    This is called from getCaps, before the main device has been initialised
    so we need to do EAX5 specific initialisation here for the temporary
    device

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 60f22fe3 auto
// OFFSET: 0x0094A170
FMOD_RESULT OutputOpenAL::getSpeakerModeEAX5(FMOD_SPEAKERMODE *speakerMode)
{
    EAXGet                eaxGet          = NULL;
    EAXSet                eaxSet          = NULL;
    long                  oalSpeakerMode  = 0;
    GUID                  guid;
    EAXSESSIONPROPERTIES  eaxSession;  

    /*
        Get the EAX function pointers
    */
    eaxGet = (EAXGet)mOALFnTable.alGetProcAddress("EAXGet");
    eaxSet = (EAXSet)mOALFnTable.alGetProcAddress("EAXSet");
    if (!eaxGet || !eaxSet)
    {
        return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

    /*
        Create an EAX5 session here for the getCaps temporary OAL context
    */
    eaxSession.ulEAXVersion = EAX_50;
    eaxSession.ulMaxActiveSends = 2;
    FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX50_Context, sizeof(GUID));
    
    eaxSet(&guid, EAXCONTEXT_EAXSESSION, 0, &eaxSession, sizeof(EAXSESSIONPROPERTIES));
    if (mOALFnTable.alGetError() != AL_NO_ERROR)
    {
		return FMOD_ERR_OUTPUT_DRIVERCALL;
    }
    
    /*
        Determine the speaker mode
    */
    eaxGet(&guid, EAXCONTEXT_SPEAKERCONFIG, 0, &oalSpeakerMode, sizeof(long));
    if (mOALFnTable.alGetError() != AL_NO_ERROR)
    {
		return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

    switch (oalSpeakerMode)
    {
        case HEADPHONES:
        case SPEAKERS_2:
        {
            *speakerMode = FMOD_SPEAKERMODE_STEREO;
            break;
        }
        case SPEAKERS_4:
        {
            *speakerMode = FMOD_SPEAKERMODE_QUAD;
            break;
        }
        case SPEAKERS_5:
        {
            *speakerMode = FMOD_SPEAKERMODE_5POINT1;
            break;
        }
        case SPEAKERS_6:
        {
            *speakerMode = FMOD_SPEAKERMODE_RAW;
            break;
        }
        case SPEAKERS_7:
        {
            *speakerMode = FMOD_SPEAKERMODE_7POINT1;
            break;
        }
    }                

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Attempt to lock an EAX slot

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 580e3057 manual
// OFFSET: 0x0094A2C0
FMOD_RESULT OutputOpenAL::lockSlotEAX5(GUID &slotGUID)
{
    int                  slotIsLocked;
    EAXFXSLOTPROPERTIES  fxSlot;

    FMOD_memcpy(&fxSlot.guidLoadEffect, &FMOD_EAX_REVERB_EFFECT, sizeof(GUID));

    /*
        Check if this slot is locked
    */
    mEAXGet(&slotGUID, EAXFXSLOT_LOCK, 0, &slotIsLocked, sizeof(int));
    if (mOALFnTable.alGetError() != AL_NO_ERROR)
    {
        return FMOD_ERR_REVERB_INSTANCE;
    }

    if (slotIsLocked)
    {
        GUID effect;

        // Find out what type of effect is in the locked slot
        mEAXGet(&slotGUID, EAXFXSLOT_LOADEFFECT, 0, &effect, sizeof(GUID));
        if (mOALFnTable.alGetError() != AL_NO_ERROR)
        {
            return FMOD_ERR_REVERB_INSTANCE;
        }
        
        // Check if the slot contains a reverb, if so we can still use that slot
        if (memcmp(&effect, &fxSlot.guidLoadEffect, sizeof(GUID)))
        {
            return FMOD_ERR_REVERB_INSTANCE;
        }
    }
    else    // Slot isn't locked
    {       
        fxSlot.lLock               = EAXFXSLOT_LOCKED;
        fxSlot.lVolume             = EAXFXSLOT_DEFAULTVOLUME;
        fxSlot.ulFlags             = EAXFXSLOT_DEFAULTFLAGS;
        fxSlot.lOcclusion          = EAXFXSLOT_DEFAULTOCCLUSION;
        fxSlot.flOcclusionLFRatio  = EAXFXSLOT_DEFAULTOCCLUSIONLFRATIO;

        // Lock the slot
        mEAXSet(&slotGUID, EAXFXSLOT_ALLPARAMETERS, 0, &fxSlot, sizeof(EAXFXSLOTPROPERTIES));
        if (mOALFnTable.alGetError() != AL_NO_ERROR)
        {
            return FMOD_ERR_REVERB_INSTANCE;
        }
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Set properties for EAX5

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]
    With EAX5 all four slots can be locked with reverb

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: d0a3b577 manual
// OFFSET: 0x0094A440
FMOD_RESULT OutputOpenAL::setPropertiesEAX5(const FMOD_REVERB_PROPERTIES *prop)
{
    GUID                 slotGUID;
    GUID                 sourceGUID;
    EAXREVERBPROPERTIES  eaxProperties;
    FMOD_RESULT          result;
    int                  room;

    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::setPropertiesEAX5", "Setting reverb properties for instance: %d\n", prop->Instance));

    /*
        Make sure all reverb effects are disabled on the mixer sources
    */
    if (!mMixerReverbDisabled)
    {
        room = -10000;
        FMOD_memcpy(&sourceGUID, &FMOD_EAXPROPERTYID_EAX50_Source, sizeof(GUID));

        // Only the first mixer source is technically used (multichannel source), but set all anyway
        for (int i = mMixerSourceOffset; i < mNumSources; i++)
        {
            mEAXSet(&sourceGUID, EAXSOURCE_ROOM, mSources[i].sid, &room, sizeof(int));
            if (mOALFnTable.alGetError() != AL_NO_ERROR)
            {
		        return FMOD_ERR_INTERNAL;
            }
        }
        
        mMixerReverbDisabled = true;
    }

    /*
        Each reverb instance is equivalent to a hardware effect slot,
        attempt to lock that slot for use by FMOD
    */
    switch (prop->Instance)
    {
        case 1:
        {
            FMOD_memcpy(&slotGUID, &FMOD_EAXPROPERTYID_EAX50_FXSlot1, sizeof(GUID));
            if (!gEAX5LockedOALFxSlot1)
            {
                result = lockSlotEAX5(slotGUID);
                if (result != FMOD_OK)
                {
                    return result;
                }

                gEAX5LockedOALFxSlot1 = true;
            }
            break;
        }
        case 2:
        {
            FMOD_memcpy(&slotGUID, &FMOD_EAXPROPERTYID_EAX50_FXSlot2, sizeof(GUID));
            if (!gEAX5LockedOALFxSlot2)
            {
                result = lockSlotEAX5(slotGUID);
                if (result != FMOD_OK)
                {
                    return result;
                }

                gEAX5LockedOALFxSlot2 = true;
            }
            break;
        }
        case 3:
        {
            FMOD_memcpy(&slotGUID, &FMOD_EAXPROPERTYID_EAX50_FXSlot3, sizeof(GUID));
            if (!gEAX5LockedOALFxSlot3)
            {
                result = lockSlotEAX5(slotGUID);
                if (result != FMOD_OK)
                {
                    return result;
                }

                gEAX5LockedOALFxSlot3 = true;
            }
            break;
        }
        default:
        {
            FMOD_memcpy(&slotGUID, &FMOD_EAXPROPERTYID_EAX50_FXSlot0, sizeof(GUID));
            if (!gEAX5LockedOALFxSlot0)
            {
                result = lockSlotEAX5(slotGUID);
                if (result != FMOD_OK)
                {
                    return result;
                }

                gEAX5LockedOALFxSlot0 = true;
            }
        }
    }

    FMOD_memset(&eaxProperties, 0, sizeof(EAXREVERBPROPERTIES));
    eaxProperties.ulEnvironment           = prop->Environment < 0 ? 0 : prop->Environment;
    eaxProperties.flEnvironmentSize       = prop->EnvSize;           
    eaxProperties.flEnvironmentDiffusion  = prop->EnvDiffusion;      
    eaxProperties.lRoom                   = prop->Room;              
    eaxProperties.lRoomHF                 = prop->RoomHF;            
    eaxProperties.lRoomLF                 = prop->RoomLF;            
    eaxProperties.flDecayTime             = prop->DecayTime;         
    eaxProperties.flDecayHFRatio          = prop->DecayHFRatio;      
    eaxProperties.flDecayLFRatio          = prop->DecayLFRatio;      
    eaxProperties.lReflections            = prop->Reflections;       
    eaxProperties.flReflectionsDelay      = prop->ReflectionsDelay;  
    eaxProperties.vReflectionsPan.x       = prop->ReflectionsPan[0]; 
    eaxProperties.vReflectionsPan.y       = prop->ReflectionsPan[1]; 
    eaxProperties.vReflectionsPan.z       = prop->ReflectionsPan[2]; 
    eaxProperties.lReverb                 = prop->Reverb;            
    eaxProperties.flReverbDelay           = prop->ReverbDelay;       
    eaxProperties.vReverbPan.x            = prop->ReverbPan[0];
    eaxProperties.vReverbPan.y            = prop->ReverbPan[1];
    eaxProperties.vReverbPan.z            = prop->ReverbPan[2];
    eaxProperties.flEchoTime              = prop->EchoTime;          
    eaxProperties.flEchoDepth             = prop->EchoDepth;         
    eaxProperties.flModulationTime        = prop->ModulationTime;    
    eaxProperties.flModulationDepth       = prop->ModulationDepth;   
    eaxProperties.flAirAbsorptionHF       = prop->AirAbsorptionHF;   
    eaxProperties.flHFReference           = prop->HFReference;       
    eaxProperties.flLFReference           = prop->LFReference;       
    eaxProperties.flRoomRolloffFactor     = prop->RoomRolloffFactor; 
    eaxProperties.ulFlags                 = prop->Flags & 0xFF;     // Mask out FMOD specific flags, leaving only EAX flags

    mEAXSet(&slotGUID, EAXREVERB_ALLPARAMETERS, 0, &eaxProperties, sizeof(EAXREVERBPROPERTIES));
    if (mOALFnTable.alGetError() != AL_NO_ERROR)
    {
        FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputOpenAL::setPropertiesEAX5", "Error setting reverb properties\n"));
        return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

	return FMOD_OK;
}

}       /* namespace FMOD */

#endif  /* FMOD_SUPPORT_EAX */
#endif  /* FMOD_SUPPORT_OPENAL */
