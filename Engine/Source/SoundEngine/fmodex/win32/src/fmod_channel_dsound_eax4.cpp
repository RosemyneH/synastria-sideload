#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_EAX

#include "fmod_channel_dsound_eax4.h"
#include "fmod_eax4.h"
#include "fmod_output_dsound.h"
#include "fmod_soundi.h"


namespace FMOD
{

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
// VERIFY: 439c3100 manual
// OFFSET: 0x0094F5B0
FMOD_RESULT ChannelDSoundEAX4::setReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    HRESULT             hr;
    EAXSOURCEPROPERTIES eaxsrcprops;
    EAXACTIVEFXSLOTS    eaxslots;
    GUID                guid;
    unsigned int        flags = prop->Flags;

    if (!mOutputDSound)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

	/*
		OBTAIN REVERB INTERFACE
	*/
	if (mBuffer3D && !mBufferReverb)
	{
  		if (mFlags & CHANNELREAL_FLAG_PAUSED && mOutputDSound->mDirectXVersion >= 8)
  		{
            mBuffer->AcquireResources(DSBPLAY_LOCHARDWARE, 0, NULL);
        }

        FMOD_memcpy(&guid, &FMOD_IID_IKsPropertySet, sizeof(GUID));

        hr = mBuffer3D->QueryInterface(guid, (void **)&mBufferReverb);
	    if (hr != DS_OK)
	    {
		    return FMOD_ERR_UNSUPPORTED;
	    }
	}

    /*
        Set the target FX slot(s) for this source
    */
    if ((flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT0) &&
        (flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT1) &&
        (flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2))
    {
        return FMOD_ERR_UNSUPPORTED;
    }

    /*
        Set ACTIVE FX slot(s)
    */
    if (flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT0)
    {
        FMOD_memcpy(&eaxslots.guidActiveFXSlots[0], &FMOD_EAXPROPERTYID_EAX40_FXSlot0, sizeof(GUID));
        FMOD_memcpy(&eaxslots.guidActiveFXSlots[1], &FMOD_EAX_NULL_GUID, sizeof(GUID));
        flags = (flags & ~FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT0);

        if (flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT1)
        {
            FMOD_memcpy(&eaxslots.guidActiveFXSlots[1], &FMOD_EAXPROPERTYID_EAX40_FXSlot2, sizeof(GUID));
            flags = (flags & ~FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT1);
        }
        else if (flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2)
        {
            FMOD_memcpy(&eaxslots.guidActiveFXSlots[1], &FMOD_EAXPROPERTYID_EAX40_FXSlot3, sizeof(GUID));
            flags = (flags & ~FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2);
        }
    }
    else if (flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT1)
    {
        FMOD_memcpy(&eaxslots.guidActiveFXSlots[0], &FMOD_EAXPROPERTYID_EAX40_FXSlot2, sizeof(GUID));
        FMOD_memcpy(&eaxslots.guidActiveFXSlots[1], &FMOD_EAX_NULL_GUID, sizeof(GUID));
        flags = (flags & ~FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT1);

        if (flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2)
        {
            FMOD_memcpy(&eaxslots.guidActiveFXSlots[1], &FMOD_EAXPROPERTYID_EAX40_FXSlot3, sizeof(GUID));
            flags = (flags & ~FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2);
        }
    }
    else if (flags & FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2)
    {
        FMOD_memcpy(&eaxslots.guidActiveFXSlots[0], &FMOD_EAXPROPERTYID_EAX40_FXSlot3, sizeof(GUID));
        FMOD_memcpy(&eaxslots.guidActiveFXSlots[1], &FMOD_EAX_NULL_GUID, sizeof(GUID));
        flags = (flags & ~FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2);
    }
    else    // Set to Slot 0 by default
    {
        FMOD_memcpy(&eaxslots.guidActiveFXSlots[0], &FMOD_EAXPROPERTYID_EAX40_FXSlot0, sizeof(GUID));
        FMOD_memcpy(&eaxslots.guidActiveFXSlots[1], &FMOD_EAX_NULL_GUID, sizeof(GUID));
    }

    FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX40_Source, sizeof(GUID));

    hr = mBufferReverb->Set(guid, EAXSOURCE_ACTIVEFXSLOTID, NULL, 0, &eaxslots, sizeof(EAXACTIVEFXSLOTS));
    if (FAILED(hr))
    {
        return FMOD_ERR_REVERB_INSTANCE;
    }

	/*
	    NOW SET CHANNEL PROPERTIES
	*/
    eaxsrcprops.lDirect                = prop->Direct;             
    eaxsrcprops.lDirectHF              = prop->DirectHF;           
    eaxsrcprops.lRoom                  = prop->Room;               
    eaxsrcprops.lRoomHF                = prop->RoomHF;             
    eaxsrcprops.lObstruction           = prop->Obstruction;        
    eaxsrcprops.flObstructionLFRatio   = prop->ObstructionLFRatio; 
    eaxsrcprops.lOcclusion             = prop->Occlusion;          
    eaxsrcprops.flOcclusionLFRatio     = prop->OcclusionLFRatio;   
    eaxsrcprops.flOcclusionRoomRatio   = prop->OcclusionRoomRatio; 
    eaxsrcprops.flOcclusionDirectRatio = prop->OcclusionDirectRatio;
    eaxsrcprops.lExclusion             = prop->Exclusion;          
    eaxsrcprops.flExclusionLFRatio     = prop->ExclusionLFRatio;   
    eaxsrcprops.lOutsideVolumeHF       = prop->OutsideVolumeHF;    
    eaxsrcprops.flDopplerFactor        = prop->DopplerFactor;      
    eaxsrcprops.flRolloffFactor        = prop->RolloffFactor;      
    eaxsrcprops.flRoomRolloffFactor    = prop->RoomRolloffFactor;  
    eaxsrcprops.flAirAbsorptionFactor  = prop->AirAbsorptionFactor;
    eaxsrcprops.ulFlags                = flags;              

    FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX40_Source, sizeof(GUID));

    hr = mBufferReverb->Set(guid, EAXSOURCE_ALLPARAMETERS, NULL, 0, &eaxsrcprops, sizeof(EAXSOURCEPROPERTIES));
    if (FAILED(hr))
    {
        return FMOD_ERR_INVALID_PARAM;
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
// VERIFY: bf015724 manual
// OFFSET: 0x0094F910
FMOD_RESULT ChannelDSoundEAX4::getReverbProperties(FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    HRESULT             hr;
    EAXSOURCEPROPERTIES eaxsrcprops;
    EAXACTIVEFXSLOTS    eaxslots;
    GUID                guid;
    GUID                guidfx0, guidfx2, guidfx3;
    ULONG               ulReceived;
    int                 noActiveSlots = 0;

    if (!mOutputDSound)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

	/*
		OBTAIN REVERB INTERFACE
	*/
	if (mBuffer3D && !mBufferReverb)
	{
  		if (mFlags & CHANNELREAL_FLAG_PAUSED && mOutputDSound->mDirectXVersion >= 8)
  		{
            mBuffer->AcquireResources(DSBPLAY_LOCHARDWARE, 0, NULL);
        }

        FMOD_memcpy(&guid, &FMOD_IID_IKsPropertySet, sizeof(GUID));

        hr = mBuffer3D->QueryInterface(guid, (void **)&mBufferReverb);
	    if (hr != DS_OK)
	    {
		    return FMOD_ERR_UNSUPPORTED;
	    }
	}

    /*
        Get reverb properties
    */
    FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX40_Source, sizeof(GUID));

    hr = mBufferReverb->Get(guid, EAXSOURCE_ALLPARAMETERS, NULL, 0, &eaxsrcprops, sizeof(EAXSOURCEPROPERTIES), &ulReceived);
    if (FAILED(hr))
    {
        return FMOD_ERR_UNSUPPORTED;
    }

    prop->Direct                = eaxsrcprops.lDirect;         
    prop->DirectHF              = eaxsrcprops.lDirectHF;
    prop->Room                  = eaxsrcprops.lRoom;
    prop->RoomHF                = eaxsrcprops.lRoomHF;
    prop->Obstruction           = eaxsrcprops.lObstruction;
    prop->ObstructionLFRatio    = eaxsrcprops.flObstructionLFRatio;
    prop->Occlusion             = eaxsrcprops.lOcclusion;
    prop->OcclusionLFRatio      = eaxsrcprops.flOcclusionLFRatio;
    prop->OcclusionRoomRatio    = eaxsrcprops.flOcclusionRoomRatio;
    prop->OcclusionDirectRatio  = eaxsrcprops.flOcclusionDirectRatio;
    prop->Exclusion             = eaxsrcprops.lExclusion; 
    prop->ExclusionLFRatio      = eaxsrcprops.flExclusionLFRatio;
    prop->OutsideVolumeHF       = eaxsrcprops.lOutsideVolumeHF;
    prop->DopplerFactor         = eaxsrcprops.flDopplerFactor;
    prop->RolloffFactor         = eaxsrcprops.flRolloffFactor;
    prop->RoomRolloffFactor     = eaxsrcprops.flRoomRolloffFactor;
    prop->AirAbsorptionFactor   = eaxsrcprops.flAirAbsorptionFactor;
    prop->Flags                 = eaxsrcprops.ulFlags;

    hr = mBufferReverb->Get(guid, EAXSOURCE_ACTIVEFXSLOTID, NULL, 0, &eaxslots, sizeof(EAXACTIVEFXSLOTS), &ulReceived);
    if (FAILED(hr))
    {
        return FMOD_ERR_UNSUPPORTED;
    }

    FMOD_memcpy(&guidfx0, &FMOD_EAXPROPERTYID_EAX40_FXSlot0, sizeof(GUID));
    FMOD_memcpy(&guidfx2, &FMOD_EAXPROPERTYID_EAX40_FXSlot2, sizeof(GUID));
    FMOD_memcpy(&guidfx3, &FMOD_EAXPROPERTYID_EAX40_FXSlot3, sizeof(GUID));

    if (!memcmp(&eaxslots.guidActiveFXSlots[0], &guidfx0, sizeof(GUID)))
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT0;
    }
    else if (!memcmp(&eaxslots.guidActiveFXSlots[0], &guidfx2, sizeof(GUID)))
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT1;
    }
    else if (!memcmp(&eaxslots.guidActiveFXSlots[0], &guidfx3, sizeof(GUID)))
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2;
    }
    else
    {
        noActiveSlots++;
    }

    if (!memcmp(&eaxslots.guidActiveFXSlots[1], &guidfx0, sizeof(GUID)))
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT0;
    }
    else if (!memcmp(&eaxslots.guidActiveFXSlots[1], &guidfx2, sizeof(GUID)))
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT1;
    }
    else if (!memcmp(&eaxslots.guidActiveFXSlots[1], &guidfx3, sizeof(GUID)))
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT2;
    }
    else
    {
        noActiveSlots++;
    }

    if (noActiveSlots == 2)  // If no slots have been assigned, assign to slot 0
    {
        prop->Flags |= FMOD_REVERB_CHANNELFLAGS_ENVIRONMENT0;
    }

    return FMOD_OK;
}

}

#endif
