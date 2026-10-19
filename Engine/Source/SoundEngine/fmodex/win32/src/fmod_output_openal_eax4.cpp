#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL
#ifdef FMOD_SUPPORT_EAX

#include "fmod_output_openal.h"
#include "fmod_eax4.h"

namespace FMOD
{

bool gEAX4LockedOALFxSlot2 = false;
bool gEAX4LockedOALFxSlot3 = false;

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
// VERIFY: 99a83cc9 manual
// OFFSET: 0x0094B0E0
FMOD_RESULT OutputOpenAL::lockSlotEAX4(GUID &slotGUID)
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
        fxSlot.lLock    = EAXFXSLOT_LOCKED;
        fxSlot.lVolume  = EAXFXSLOT_DEFAULTVOLUME;
        fxSlot.ulFlags  = EAXFXSLOT_DEFAULTFLAGS;

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
    Set properties for EAX4

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]
    With EAX4 the first slot is locked as reverb so we can still use it,
    the second slot is locked as chorus, so it is no use to us, slots
    3 and 4 are free to be tasked as reverb

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 8dcd1b50 manual
// OFFSET: 0x0094B250
FMOD_RESULT OutputOpenAL::setPropertiesEAX4(const FMOD_REVERB_PROPERTIES *prop)
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

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::setPropertiesEAX4", "Setting reverb properties for instance: %d\n", prop->Instance));

    /*
        Make sure all reverb effects are disabled on the mixer sources
    */
    if (!mMixerReverbDisabled)
    {
        room = -10000;
        FMOD_memcpy(&sourceGUID, &FMOD_EAXPROPERTYID_EAX40_Source, sizeof(GUID));

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
            FMOD_memcpy(&slotGUID, &FMOD_EAXPROPERTYID_EAX40_FXSlot2, sizeof(GUID));
            if (!gEAX4LockedOALFxSlot2)
            {
                result = lockSlotEAX4(slotGUID);
                if (result != FMOD_OK)
                {
                    return result;
                }

                gEAX4LockedOALFxSlot2 = true;
            }
            break;
        }
        case 2:
        {
            FMOD_memcpy(&slotGUID, &FMOD_EAXPROPERTYID_EAX40_FXSlot3, sizeof(GUID));
            if (!gEAX4LockedOALFxSlot3)
            {
                result = lockSlotEAX4(slotGUID);
                if (result != FMOD_OK)
                {
                    return result;
                }

                gEAX4LockedOALFxSlot3 = true;
            }
            break;
        }
        default:
        {
            FMOD_memcpy(&slotGUID, &FMOD_EAXPROPERTYID_EAX40_FXSlot0, sizeof(GUID));
        }
    }

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
        FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputOpenAL::setPropertiesEAX4", "Error setting reverb properties\n"));
        return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

	return FMOD_OK;
}

}       /* namespace FMOD */

#endif  /* FMOD_SUPPORT_EAX */
#endif  /* FMOD_SUPPORT_OPENAL */
