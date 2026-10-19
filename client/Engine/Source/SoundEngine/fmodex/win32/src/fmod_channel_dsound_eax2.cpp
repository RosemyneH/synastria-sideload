#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_EAX

#include "fmod_output_dsound.h"
#include "fmod_channel_dsound_eax2.h"
#include "fmod_eax2.h"


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
// VERIFY: be964640 manual
// OFFSET: 0x0094EFE0
FMOD_RESULT ChannelDSoundEAX2::setReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES *prop)
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
    dsprops.lOutsideVolumeHF       = prop->OutsideVolumeHF;    
    dsprops.flRoomRolloffFactor    = prop->RoomRolloffFactor;  
    dsprops.flAirAbsorptionFactor  = prop->AirAbsorptionFactor;
    dsprops.dwFlags                = prop->Flags;              

    FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX20_BufferProperties, sizeof(GUID));

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
// VERIFY: 8f2ac928 auto
// OFFSET: 0x0094F140
FMOD_RESULT ChannelDSoundEAX2::getReverbProperties(FMOD_REVERB_CHANNELPROPERTIES *prop)
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
    FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX20_BufferProperties, sizeof(GUID));

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
    prop->OcclusionDirectRatio  = 0;
    prop->Exclusion             = 0; 
    prop->ExclusionLFRatio      = 0;
    prop->OutsideVolumeHF       = dsprops.lOutsideVolumeHF;
    prop->DopplerFactor         = 0;
    prop->RolloffFactor         = 0;
    prop->RoomRolloffFactor     = dsprops.flRoomRolloffFactor;
    prop->AirAbsorptionFactor   = dsprops.flAirAbsorptionFactor;
    prop->Flags                 = dsprops.dwFlags;

    return FMOD_OK;
}

}

#endif
