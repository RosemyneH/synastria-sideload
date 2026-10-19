#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL
#ifdef FMOD_SUPPORT_EAX

#include "fmod_channel_openal_eax4.h"
#include "fmod_eax4.h"

namespace FMOD
{

/*
[
	[DESCRIPTION]
    Initialise this channel for EAX4 specific settings

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]
    This will also reset the sources back to defaults when a new channel
    is reusing sources

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: ab3c8d32 auto
// OFFSET: 0x00947E10
FMOD_RESULT ChannelOpenALEAX4::setupChannel()
{
    EAXACTIVEFXSLOTS           eaxSlots;
    EAXSOURCEPROPERTIES        eaxSourceProperties;
	GUID                       eaxSourceGuid;
    GUID                       eaxFXSlot0, eaxFXSlotNull;

    if (!mOutputOAL)
    {
        return FMOD_ERR_UNINITIALIZED;
    }
    
    FMOD_memcpy(&eaxFXSlot0, &FMOD_EAXPROPERTYID_EAX40_FXSlot0, sizeof(GUID));
    FMOD_memcpy(&eaxFXSlotNull, &FMOD_EAX_NULL_GUID, sizeof(GUID));
    FMOD_memcpy(&eaxSourceGuid, &FMOD_EAXPROPERTYID_EAX40_Source, sizeof(GUID));
    
    /*
        Set the default slot 0 to on
    */
    FMOD_memcpy(&eaxSlots.guidActiveFXSlots[0], &eaxFXSlot0, sizeof(GUID));
    FMOD_memcpy(&eaxSlots.guidActiveFXSlots[1], &eaxFXSlotNull, sizeof(GUID));

    /*
        Set the default channel reverb properties
    */
    eaxSourceProperties.lDirect                 = EAXSOURCE_DEFAULTDIRECT;
    eaxSourceProperties.lDirectHF               = EAXSOURCE_DEFAULTDIRECTHF;
    eaxSourceProperties.lRoom                   = EAXSOURCE_DEFAULTROOM;
    eaxSourceProperties.lRoomHF                 = EAXSOURCE_DEFAULTROOMHF;
    eaxSourceProperties.lObstruction            = EAXSOURCE_DEFAULTOBSTRUCTION;
    eaxSourceProperties.flObstructionLFRatio    = EAXSOURCE_DEFAULTOBSTRUCTIONLFRATIO;
    eaxSourceProperties.lOcclusion              = EAXSOURCE_DEFAULTOCCLUSION;
    eaxSourceProperties.flOcclusionLFRatio      = EAXSOURCE_DEFAULTOCCLUSIONLFRATIO;
    eaxSourceProperties.flOcclusionRoomRatio    = EAXSOURCE_DEFAULTOCCLUSIONROOMRATIO;
    eaxSourceProperties.flOcclusionDirectRatio  = EAXSOURCE_DEFAULTOCCLUSIONDIRECTRATIO;
    eaxSourceProperties.lExclusion              = EAXSOURCE_DEFAULTEXCLUSION;
    eaxSourceProperties.flExclusionLFRatio      = EAXSOURCE_DEFAULTEXCLUSIONLFRATIO;
    eaxSourceProperties.lOutsideVolumeHF        = EAXSOURCE_DEFAULTOUTSIDEVOLUMEHF;
    eaxSourceProperties.flDopplerFactor         = EAXSOURCE_DEFAULTDOPPLERFACTOR;
    eaxSourceProperties.flRolloffFactor         = EAXSOURCE_DEFAULTROLLOFFFACTOR;
    eaxSourceProperties.flRoomRolloffFactor     = EAXSOURCE_DEFAULTROOMROLLOFFFACTOR;
    eaxSourceProperties.flAirAbsorptionFactor   = EAXSOURCE_DEFAULTAIRABSORPTIONFACTOR;
    eaxSourceProperties.ulFlags                 = EAXSOURCE_DEFAULTFLAGS;

    /*
        Apply the defaults to each source
    */
    for (int i = 0; i < mNumSources; i++)
	{
        mOutputOAL->mEAXSet(&eaxSourceGuid, EAXSOURCE_ACTIVEFXSLOTID, mSources[i]->sid, &eaxSlots, sizeof(EAXACTIVEFXSLOTS));
        if (mOutputOAL->mOALFnTable.alGetError() != AL_NO_ERROR)
        {
            return FMOD_ERR_OUTPUT_DRIVERCALL;
        }

	    mOutputOAL->mEAXSet(&eaxSourceGuid, EAXSOURCE_ALLPARAMETERS, mSources[i]->sid, &eaxSourceProperties, sizeof(EAXSOURCEPROPERTIES));
        if (mOutputOAL->mOALFnTable.alGetError() != AL_NO_ERROR)
        {
            return FMOD_ERR_OUTPUT_DRIVERCALL;
        }
    }

	return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Set the reverb properties on the hardware

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]
    EAX4 has 4 effects slots, but can only have 2 active on any one source.
    The first slot is dedicated to reverb, the second slot is dedicated chorus,
    any of the remaining two slots can be tasked for reverb

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 60683150 auto
// OFFSET: 0x00947F90
FMOD_RESULT ChannelOpenALEAX4::setReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    EAXSOURCEPROPERTIES eaxSourceProperties;
    EAXACTIVEFXSLOTS    eaxSlots;
    unsigned long       eaxSourceFlags;
    GUID                eaxSourceGuid;
    GUID                eaxFXSlot0, eaxFXSlot2, eaxFXSlot3, eaxFXSlotNull;
    int                 numActiveSlots;

    if (!mOutputOAL)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    FMOD_memcpy(&eaxFXSlot0, &FMOD_EAXPROPERTYID_EAX40_FXSlot0, sizeof(GUID));
    FMOD_memcpy(&eaxFXSlot2, &FMOD_EAXPROPERTYID_EAX40_FXSlot2, sizeof(GUID));
    FMOD_memcpy(&eaxFXSlot3, &FMOD_EAXPROPERTYID_EAX40_FXSlot3, sizeof(GUID));
    FMOD_memcpy(&eaxFXSlotNull, &FMOD_EAX_NULL_GUID, sizeof(GUID));    
    FMOD_memcpy(&eaxSourceGuid, &FMOD_EAXPROPERTYID_EAX40_Source, sizeof(GUID));

    /*
        Only support a maximum 2 active slots per channel
    */
    numActiveSlots = 0;
    if (prop->Flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT0)
    {
        numActiveSlots++; 
    }
    if (prop->Flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT1)
    {
        numActiveSlots++;
    }
    if (prop->Flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2)
    {
        numActiveSlots++;
    }
    if (numActiveSlots > 2)
    {
        return FMOD_ERR_UNSUPPORTED;   
    }

    /*
        Store the slot IDs for the chosen environment
    */
    FMOD_memcpy(&eaxSlots.guidActiveFXSlots[0], &eaxFXSlotNull, sizeof(GUID));
    FMOD_memcpy(&eaxSlots.guidActiveFXSlots[1], &eaxFXSlotNull, sizeof(GUID));
    if (prop->Flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT0)
    {
        FMOD_memcpy(&eaxSlots.guidActiveFXSlots[0], &eaxFXSlot0, sizeof(GUID));

        if (prop->Flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT1)
        {
            FMOD_memcpy(&eaxSlots.guidActiveFXSlots[1], &eaxFXSlot2, sizeof(GUID));
        }
        else if (prop->Flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2)
        {
            FMOD_memcpy(&eaxSlots.guidActiveFXSlots[1], &eaxFXSlot3, sizeof(GUID));
        }
    }
    else if (prop->Flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT1)
    {
        FMOD_memcpy(&eaxSlots.guidActiveFXSlots[0], &eaxFXSlot2, sizeof(GUID));

        if (prop->Flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2)
        {
            FMOD_memcpy(&eaxSlots.guidActiveFXSlots[1], &eaxFXSlot3, sizeof(GUID));
        }
    }
    else if (prop->Flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2)
    {
        FMOD_memcpy(&eaxSlots.guidActiveFXSlots[0], &eaxFXSlot2, sizeof(GUID));
    }
    else    // Set to Slot 0 by default
    {
        FMOD_memcpy(&eaxSlots.guidActiveFXSlots[0], &eaxFXSlot0, sizeof(GUID));
    }

	/*
	    Set the channel reverb properties
	*/
    mOutputOAL->mEAXGet(&eaxSourceGuid, EAXSOURCE_FLAGS, mSources[0]->sid, &eaxSourceFlags,  sizeof(unsigned long));
    if (mOutputOAL->mOALFnTable.alGetError() != AL_NO_ERROR)
    {
        return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

    eaxSourceProperties.lDirect                 = prop->Direct;             
    eaxSourceProperties.lDirectHF               = prop->DirectHF;           
    eaxSourceProperties.lRoom                   = prop->Room;               
    eaxSourceProperties.lRoomHF                 = prop->RoomHF;             
    eaxSourceProperties.lObstruction            = prop->Obstruction;        
    eaxSourceProperties.flObstructionLFRatio    = prop->ObstructionLFRatio; 
    eaxSourceProperties.lOcclusion              = prop->Occlusion;          
    eaxSourceProperties.flOcclusionLFRatio      = prop->OcclusionLFRatio;   
    eaxSourceProperties.flOcclusionRoomRatio    = prop->OcclusionRoomRatio; 
    eaxSourceProperties.flOcclusionDirectRatio  = prop->OcclusionDirectRatio;
    eaxSourceProperties.lExclusion              = prop->Exclusion;          
    eaxSourceProperties.flExclusionLFRatio      = prop->ExclusionLFRatio;   
    eaxSourceProperties.lOutsideVolumeHF        = prop->OutsideVolumeHF;    
    eaxSourceProperties.flDopplerFactor         = prop->DopplerFactor;      
    eaxSourceProperties.flRolloffFactor         = prop->RolloffFactor;      
    eaxSourceProperties.flRoomRolloffFactor     = prop->RoomRolloffFactor;  
    eaxSourceProperties.flAirAbsorptionFactor   = prop->AirAbsorptionFactor;

    eaxSourceFlags = (prop->Flags & FMOD_REVERB_CHANNELFLAGS_DIRECTHFAUTO) ? (eaxSourceFlags | EAXSOURCEFLAGS_DIRECTHFAUTO) : (eaxSourceFlags & ~EAXSOURCEFLAGS_DIRECTHFAUTO);
    eaxSourceFlags = (prop->Flags & FMOD_REVERB_CHANNELFLAGS_ROOMAUTO)     ? (eaxSourceFlags | EAXSOURCEFLAGS_ROOMAUTO)     : (eaxSourceFlags & ~EAXSOURCEFLAGS_ROOMAUTO);
    eaxSourceFlags = (prop->Flags & FMOD_REVERB_CHANNELFLAGS_ROOMHFAUTO)   ? (eaxSourceFlags | EAXSOURCEFLAGS_ROOMHFAUTO)   : (eaxSourceFlags & ~EAXSOURCEFLAGS_ROOMHFAUTO);
    eaxSourceProperties.ulFlags = eaxSourceFlags;

    /*
        Apply the active slots and properties to the sources
    */
    for (int i = 0; i < mNumSources; i++)
	{
        mOutputOAL->mEAXSet(&eaxSourceGuid, EAXSOURCE_ACTIVEFXSLOTID, mSources[i]->sid, &eaxSlots, sizeof(EAXACTIVEFXSLOTS));
        if (mOutputOAL->mOALFnTable.alGetError() != AL_NO_ERROR)
        {
            return FMOD_ERR_OUTPUT_DRIVERCALL;
        }

	    mOutputOAL->mEAXSet(&eaxSourceGuid, EAXSOURCE_ALLPARAMETERS, mSources[i]->sid, &eaxSourceProperties, sizeof(EAXSOURCEPROPERTIES));
        if (mOutputOAL->mOALFnTable.alGetError() != AL_NO_ERROR)
        {
            return FMOD_ERR_OUTPUT_DRIVERCALL;
        }
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Get the reverb properties from the hardware

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]
    EAX4 has 4 effects slots, but can only have 2 active on any one source.
    The first slot is dedicated to reverb, the second slot is dedicated chorus,
    any of the remaining two slots can be tasked for reverb

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: dc315a1e manual
// OFFSET: 0x009482C0
FMOD_RESULT ChannelOpenALEAX4::getReverbProperties(FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    EAXSOURCEPROPERTIES  eaxSourceProperties;
    EAXACTIVEFXSLOTS     eaxSlots;
    GUID                 eaxSourceGuid;
    GUID                 eaxFXSlot0, eaxFXSlot2, eaxFXSlot3;
    int                  numInactiveSlots;

    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!mOutputOAL)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

    FMOD_memcpy(&eaxFXSlot0, &FMOD_EAXPROPERTYID_EAX40_FXSlot0, sizeof(GUID));
    FMOD_memcpy(&eaxFXSlot2, &FMOD_EAXPROPERTYID_EAX40_FXSlot2, sizeof(GUID));
    FMOD_memcpy(&eaxFXSlot3, &FMOD_EAXPROPERTYID_EAX40_FXSlot3, sizeof(GUID));
    FMOD_memcpy(&eaxSourceGuid, &FMOD_EAXPROPERTYID_EAX40_Source, sizeof(GUID));
    
	/*
	    Get the channel reverb properties
	*/
    mOutputOAL->mEAXGet(&eaxSourceGuid, EAXSOURCE_ALLPARAMETERS, mSources[0]->sid, &eaxSourceProperties, sizeof(EAXSOURCEPROPERTIES));
    if (mOutputOAL->mOALFnTable.alGetError() != AL_NO_ERROR)
    {
        return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

    FMOD_memset(prop, 0, sizeof(FMOD_REVERB_CHANNELPROPERTIES));
    prop->Direct                = eaxSourceProperties.lDirect;         
    prop->DirectHF              = eaxSourceProperties.lDirectHF;
    prop->Room                  = eaxSourceProperties.lRoom;
    prop->RoomHF                = eaxSourceProperties.lRoomHF;
    prop->Obstruction           = eaxSourceProperties.lObstruction;
    prop->ObstructionLFRatio    = eaxSourceProperties.flObstructionLFRatio;
    prop->Occlusion             = eaxSourceProperties.lOcclusion;
    prop->OcclusionLFRatio      = eaxSourceProperties.flOcclusionLFRatio;
    prop->OcclusionRoomRatio    = eaxSourceProperties.flOcclusionRoomRatio;
    prop->OcclusionDirectRatio  = eaxSourceProperties.flOcclusionDirectRatio;
    prop->Exclusion             = eaxSourceProperties.lExclusion; 
    prop->ExclusionLFRatio      = eaxSourceProperties.flExclusionLFRatio;
    prop->OutsideVolumeHF       = eaxSourceProperties.lOutsideVolumeHF;
    prop->DopplerFactor         = eaxSourceProperties.flDopplerFactor;
    prop->RolloffFactor         = eaxSourceProperties.flRolloffFactor;
    prop->RoomRolloffFactor     = eaxSourceProperties.flRoomRolloffFactor;
    prop->AirAbsorptionFactor   = eaxSourceProperties.flAirAbsorptionFactor;
       
    prop->Flags = 0;
    prop->Flags |= (eaxSourceProperties.ulFlags & EAXSOURCEFLAGS_DIRECTHFAUTO) ? FMOD_REVERB_CHANNELFLAGS_DIRECTHFAUTO : 0;
    prop->Flags |= (eaxSourceProperties.ulFlags & EAXSOURCEFLAGS_ROOMAUTO)     ? FMOD_REVERB_CHANNELFLAGS_ROOMAUTO     : 0;
    prop->Flags |= (eaxSourceProperties.ulFlags & EAXSOURCEFLAGS_ROOMHFAUTO)   ? FMOD_REVERB_CHANNELFLAGS_ROOMHFAUTO   : 0;

    // Get all the effect slots associated with this source
    mOutputOAL->mEAXGet(&eaxSourceGuid, EAXSOURCE_ACTIVEFXSLOTID, mSources[0]->sid, &eaxSlots, sizeof(EAXACTIVEFXSLOTS));
    if (mOutputOAL->mOALFnTable.alGetError() != AL_NO_ERROR)
    {
        return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

    /*
        See if there are any active effect slots
    */
    numInactiveSlots = 0;

    // Check active slot 0 for any of the 3 possible effect IDs
    if (!memcmp(&eaxSlots.guidActiveFXSlots[0], &eaxFXSlot0, sizeof(GUID)))
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT0;
    }
    else if (!memcmp(&eaxSlots.guidActiveFXSlots[0], &eaxFXSlot2, sizeof(GUID)))
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2;
    }
    else if (!memcmp(&eaxSlots.guidActiveFXSlots[0], &eaxFXSlot3, sizeof(GUID)))
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT3;
    }
    else
    {
        numInactiveSlots++;
    }

    // Check active slot 1 for any of the 3 possible effect IDs
    if (!memcmp(&eaxSlots.guidActiveFXSlots[1], &eaxFXSlot0, sizeof(GUID)))
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT0;
    }
    else if (!memcmp(&eaxSlots.guidActiveFXSlots[1], &eaxFXSlot2, sizeof(GUID)))
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2;
    }
    else if (!memcmp(&eaxSlots.guidActiveFXSlots[1], &eaxFXSlot3, sizeof(GUID)))
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT3;
    }
    else
    {
        numInactiveSlots++;
    }

    // If no slots have been assigned (max slots = 2), assign to slot 0
    if (numInactiveSlots == 2)
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT0;
    }

    return FMOD_OK;
}

}

#endif /* FMOD_SUPPORT_EAX */
#endif /* FMOD_SUPPORT_OPENAL */
