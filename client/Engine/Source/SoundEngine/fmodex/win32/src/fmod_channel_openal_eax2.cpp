#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL
#ifdef FMOD_SUPPORT_EAX

#include "fmod_channel_openal_eax2.h"
#include "fmod_eax2.h"

namespace FMOD
{

/*
[
	[DESCRIPTION]
    Initialise this channel for EAX2 specific settings

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
// VERIFY: b0f83add auto
// OFFSET: 0x0094B4E0
FMOD_RESULT ChannelOpenALEAX2::setupChannel()
{
    EAXBUFFERPROPERTIES  eaxBufferProperties;
	GUID                 eaxBufferGuid;

    if (!mOutputOAL)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

    FMOD_memcpy(&eaxBufferGuid, &FMOD_DSPROPSETID_EAX20_BufferProperties, sizeof(GUID));

    /*
        Set the default channel reverb properties
    */
    eaxBufferProperties.lDirect                 = EAXBUFFER_DEFAULTDIRECT;
    eaxBufferProperties.lDirectHF               = EAXBUFFER_DEFAULTDIRECTHF;
    eaxBufferProperties.lRoom                   = EAXBUFFER_DEFAULTROOM;
    eaxBufferProperties.lRoomHF                 = EAXBUFFER_DEFAULTROOMHF;
    eaxBufferProperties.flRoomRolloffFactor     = EAXBUFFER_DEFAULTROOMROLLOFFFACTOR;
    eaxBufferProperties.lObstruction            = EAXBUFFER_DEFAULTOBSTRUCTION;
    eaxBufferProperties.flObstructionLFRatio    = EAXBUFFER_DEFAULTOBSTRUCTIONLFRATIO;
    eaxBufferProperties.lOcclusion              = EAXBUFFER_DEFAULTOCCLUSION;
    eaxBufferProperties.flOcclusionLFRatio      = EAXBUFFER_DEFAULTOCCLUSIONLFRATIO;
    eaxBufferProperties.flOcclusionRoomRatio    = EAXBUFFER_DEFAULTOCCLUSIONROOMRATIO;
    eaxBufferProperties.lOutsideVolumeHF        = EAXBUFFER_DEFAULTOUTSIDEVOLUMEHF;
    eaxBufferProperties.flAirAbsorptionFactor   = EAXBUFFER_DEFAULTAIRABSORPTIONFACTOR;
    eaxBufferProperties.dwFlags                 = EAXBUFFER_DEFAULTFLAGS;

    /*
        Apply the defaults to each source
    */
    for (int i = 0; i < mNumSources; i++)
	{
	    mOutputOAL->mEAXSet(&eaxBufferGuid, DSPROPERTY_EAXBUFFER_ALLPARAMETERS, mSources[i]->sid, &eaxBufferProperties, sizeof(EAXBUFFERPROPERTIES));
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

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 89c25c66 auto
// OFFSET: 0x0094B5D0
FMOD_RESULT ChannelOpenALEAX2::setReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    EAXBUFFERPROPERTIES  eaxBufferProperties;
	GUID                 eaxBufferGuid;

    if (!mOutputOAL)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    FMOD_memcpy(&eaxBufferGuid, &FMOD_DSPROPSETID_EAX20_BufferProperties, sizeof(GUID));

	/*
	    Set the channel reverb properties
	*/
    eaxBufferProperties.lDirect                 = prop->Direct;             
    eaxBufferProperties.lDirectHF               = prop->DirectHF;           
    eaxBufferProperties.lRoom                   = prop->Room;               
    eaxBufferProperties.lRoomHF                 = prop->RoomHF;             
    eaxBufferProperties.lObstruction            = prop->Obstruction;        
    eaxBufferProperties.flObstructionLFRatio    = prop->ObstructionLFRatio; 
    eaxBufferProperties.lOcclusion              = prop->Occlusion;          
    eaxBufferProperties.flOcclusionLFRatio      = prop->OcclusionLFRatio;   
    eaxBufferProperties.flOcclusionRoomRatio    = prop->OcclusionRoomRatio; 
    eaxBufferProperties.lOutsideVolumeHF        = prop->OutsideVolumeHF;    
    eaxBufferProperties.flRoomRolloffFactor     = prop->RoomRolloffFactor;  
    eaxBufferProperties.flAirAbsorptionFactor   = prop->AirAbsorptionFactor;
    eaxBufferProperties.dwFlags                 = prop->Flags;  

    /*
        Apply the values to each source
    */
    for (int i = 0; i < mNumSources; i++)
	{
	    mOutputOAL->mEAXSet(&eaxBufferGuid, DSPROPERTY_EAXBUFFER_ALLPARAMETERS, mSources[i]->sid, &eaxBufferProperties, sizeof(EAXBUFFERPROPERTIES));
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

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 508d103e manual
// OFFSET: 0x0094B6F0
FMOD_RESULT ChannelOpenALEAX2::getReverbProperties(FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    EAXBUFFERPROPERTIES  eaxBufferProperties;
	GUID                 eaxBufferGuid;

    if (!mOutputOAL)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    FMOD_memcpy(&eaxBufferGuid, &FMOD_DSPROPSETID_EAX20_BufferProperties, sizeof(GUID));

	/*
	    Get the channel reverb properties
	*/
    mOutputOAL->mEAXGet(&eaxBufferGuid, DSPROPERTY_EAXBUFFER_ALLPARAMETERS, mSources[0]->sid, &eaxBufferProperties, sizeof(EAXBUFFERPROPERTIES));
    if (mOutputOAL->mOALFnTable.alGetError() != AL_NO_ERROR)
    {
        return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

    FMOD_memset(prop, 0, sizeof(FMOD_REVERB_CHANNELPROPERTIES));
    prop->Direct                = eaxBufferProperties.lDirect;         
    prop->DirectHF              = eaxBufferProperties.lDirectHF;
    prop->Room                  = eaxBufferProperties.lRoom;
    prop->RoomHF                = eaxBufferProperties.lRoomHF;
    prop->Obstruction           = eaxBufferProperties.lObstruction;
    prop->ObstructionLFRatio    = eaxBufferProperties.flObstructionLFRatio;
    prop->Occlusion             = eaxBufferProperties.lOcclusion;
    prop->OcclusionLFRatio      = eaxBufferProperties.flOcclusionLFRatio;
    prop->OcclusionRoomRatio    = eaxBufferProperties.flOcclusionRoomRatio;
    prop->OutsideVolumeHF       = eaxBufferProperties.lOutsideVolumeHF;
    prop->RoomRolloffFactor     = eaxBufferProperties.flRoomRolloffFactor;
    prop->AirAbsorptionFactor   = eaxBufferProperties.flAirAbsorptionFactor;
    prop->Flags                 = eaxBufferProperties.dwFlags;

    return FMOD_OK;
}

}

#endif /* FMOD_SUPPORT_EAX */
#endif /* FMOD_SUPPORT_OPENAL */
