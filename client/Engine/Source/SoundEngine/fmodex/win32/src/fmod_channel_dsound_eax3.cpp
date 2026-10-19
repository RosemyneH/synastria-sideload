#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_EAX

#include "fmod_channel_dsound_eax3.h"
#include "fmod_eax3.h"
#include "fmod_output_dsound.h"


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
// VERIFY: 3cb91b5b manual
// OFFSET: 0x0094F2B0
FMOD_RESULT ChannelDSoundEAX3::setReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    HRESULT             hr;
    EAXBUFFERPROPERTIES dsprops;
    GUID                guid;

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
	    NOW SET CHANNEL PROPERTIES
	*/
    dsprops.lDirect                = prop->Direct;             
    dsprops.lDirectHF              = prop->DirectHF;           
    dsprops.lRoom                  = prop->Room;               
    dsprops.lRoomHF                = prop->RoomHF;             
    dsprops.lObstruction           = prop->Obstruction;        
    dsprops.flObstructionLFRatio   = prop->ObstructionLFRatio; 
    dsprops.lOcclusion             = prop->Occlusion;          
    dsprops.flOcclusionLFRatio     = prop->OcclusionLFRatio;   
    dsprops.flOcclusionRoomRatio   = prop->OcclusionRoomRatio; 
    dsprops.flOcclusionDirectRatio = prop->OcclusionDirectRatio;
    dsprops.lExclusion             = prop->Exclusion;          
    dsprops.flExclusionLFRatio     = prop->ExclusionLFRatio;   
    dsprops.lOutsideVolumeHF       = prop->OutsideVolumeHF;    
    dsprops.flDopplerFactor        = prop->DopplerFactor;      
    dsprops.flRolloffFactor        = prop->RolloffFactor;      
    dsprops.flRoomRolloffFactor    = prop->RoomRolloffFactor;  
    dsprops.flAirAbsorptionFactor  = prop->AirAbsorptionFactor;
    dsprops.ulFlags                = prop->Flags;              

    FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX30_BufferProperties, sizeof(GUID));

    hr = mBufferReverb->Set(guid, DSPROPERTY_EAXBUFFER_ALLPARAMETERS, NULL, 0, &dsprops, sizeof(EAXBUFFERPROPERTIES));
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
// VERIFY: 47b851b9 auto
// OFFSET: 0x0094F430
FMOD_RESULT ChannelDSoundEAX3::getReverbProperties(FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    HRESULT             hr;
    EAXBUFFERPROPERTIES dsprops;
    GUID                guid;
    ULONG               ulReceived;

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
    FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX30_BufferProperties, sizeof(GUID));

    hr = mBufferReverb->Get(guid, DSPROPERTY_EAXBUFFER_ALLPARAMETERS, NULL, 0, &dsprops, sizeof(EAXBUFFERPROPERTIES), &ulReceived);
    if (FAILED(hr))
    {
        return FMOD_ERR_UNSUPPORTED;
    }

    prop->Direct                = dsprops.lDirect;         
    prop->DirectHF              = dsprops.lDirectHF;
    prop->Room                  = dsprops.lRoom;
    prop->RoomHF                = dsprops.lRoomHF;
    prop->Obstruction           = dsprops.lObstruction;
    prop->ObstructionLFRatio    = dsprops.flObstructionLFRatio;
    prop->Occlusion             = dsprops.lOcclusion;
    prop->OcclusionLFRatio      = dsprops.flOcclusionLFRatio;
    prop->OcclusionRoomRatio    = dsprops.flOcclusionRoomRatio;
    prop->OcclusionDirectRatio  = dsprops.flOcclusionDirectRatio;
    prop->Exclusion             = dsprops.lExclusion; 
    prop->ExclusionLFRatio      = dsprops.flExclusionLFRatio;
    prop->OutsideVolumeHF       = dsprops.lOutsideVolumeHF;
    prop->DopplerFactor         = dsprops.flDopplerFactor;
    prop->RolloffFactor         = dsprops.flRolloffFactor;
    prop->RoomRolloffFactor     = dsprops.flRoomRolloffFactor;
    prop->AirAbsorptionFactor   = dsprops.flAirAbsorptionFactor;
    prop->Flags                 = dsprops.ulFlags;

    return FMOD_OK;
}

}

#endif
