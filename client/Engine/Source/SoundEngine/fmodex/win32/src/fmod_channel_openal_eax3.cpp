#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL
#ifdef FMOD_SUPPORT_EAX

#include "fmod_channel_openal_eax3.h"
#include "fmod_eax3.h"

namespace FMOD
{

/*
[
	[DESCRIPTION]
    Initialise this channel for EAX3 specific settings

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
// VERIFY: ac018d42 auto
// OFFSET: 0x00947AC0
FMOD_RESULT ChannelOpenALEAX3::setupChannel()
{
    EAXBUFFERPROPERTIES  eaxBufferProperties;
	GUID                 eaxBufferGuid;

    if (!mOutputOAL)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

    FMOD_memcpy(&eaxBufferGuid, &FMOD_DSPROPSETID_EAX30_BufferProperties, sizeof(GUID));

    /*
        Set the default channel reverb properties
    */
    eaxBufferProperties.lDirect                 = EAXBUFFER_DEFAULTDIRECT;
    eaxBufferProperties.lDirectHF               = EAXBUFFER_DEFAULTDIRECTHF;
    eaxBufferProperties.lRoom                   = EAXBUFFER_DEFAULTROOM;
    eaxBufferProperties.lRoomHF                 = EAXBUFFER_DEFAULTROOMHF;
    eaxBufferProperties.lObstruction            = EAXBUFFER_DEFAULTOBSTRUCTION;
    eaxBufferProperties.flObstructionLFRatio    = EAXBUFFER_DEFAULTOBSTRUCTIONLFRATIO;
    eaxBufferProperties.lOcclusion              = EAXBUFFER_DEFAULTOCCLUSION;
    eaxBufferProperties.flOcclusionLFRatio      = EAXBUFFER_DEFAULTOCCLUSIONLFRATIO;
    eaxBufferProperties.flOcclusionRoomRatio    = EAXBUFFER_DEFAULTOCCLUSIONROOMRATIO;
    eaxBufferProperties.flOcclusionDirectRatio  = EAXBUFFER_DEFAULTOCCLUSIONDIRECTRATIO;
    eaxBufferProperties.lExclusion              = EAXBUFFER_DEFAULTEXCLUSION;
    eaxBufferProperties.flExclusionLFRatio      = EAXBUFFER_DEFAULTEXCLUSIONLFRATIO;
    eaxBufferProperties.lOutsideVolumeHF        = EAXBUFFER_DEFAULTOUTSIDEVOLUMEHF;
    eaxBufferProperties.flDopplerFactor         = EAXBUFFER_DEFAULTDOPPLERFACTOR;
    eaxBufferProperties.flRolloffFactor         = EAXBUFFER_DEFAULTROLLOFFFACTOR;
    eaxBufferProperties.flRoomRolloffFactor     = EAXBUFFER_DEFAULTROOMROLLOFFFACTOR;
    eaxBufferProperties.flAirAbsorptionFactor   = EAXBUFFER_DEFAULTAIRABSORPTIONFACTOR;
    eaxBufferProperties.ulFlags                 = EAXBUFFER_DEFAULTFLAGS;

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
// VERIFY: 612962e4 auto
// OFFSET: 0x00947BC0
FMOD_RESULT ChannelOpenALEAX3::setReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES *prop)
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

    FMOD_memcpy(&eaxBufferGuid, &FMOD_DSPROPSETID_EAX30_BufferProperties, sizeof(GUID));

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
    eaxBufferProperties.flOcclusionDirectRatio  = prop->OcclusionDirectRatio;
    eaxBufferProperties.lExclusion              = prop->Exclusion;          
    eaxBufferProperties.flExclusionLFRatio      = prop->ExclusionLFRatio;   
    eaxBufferProperties.lOutsideVolumeHF        = prop->OutsideVolumeHF;    
    eaxBufferProperties.flDopplerFactor         = prop->DopplerFactor;      
    eaxBufferProperties.flRolloffFactor         = prop->RolloffFactor;      
    eaxBufferProperties.flRoomRolloffFactor     = prop->RoomRolloffFactor;  
    eaxBufferProperties.flAirAbsorptionFactor   = prop->AirAbsorptionFactor;
    eaxBufferProperties.ulFlags                 = prop->Flags;  

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
// VERIFY: ae0f0ee1 manual
// OFFSET: 0x00947CF0
FMOD_RESULT ChannelOpenALEAX3::getReverbProperties(FMOD_REVERB_CHANNELPROPERTIES *prop)
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

    FMOD_memcpy(&eaxBufferGuid, &FMOD_DSPROPSETID_EAX30_BufferProperties, sizeof(GUID));

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
    prop->OcclusionDirectRatio  = eaxBufferProperties.flOcclusionDirectRatio;
    prop->Exclusion             = eaxBufferProperties.lExclusion; 
    prop->ExclusionLFRatio      = eaxBufferProperties.flExclusionLFRatio;
    prop->OutsideVolumeHF       = eaxBufferProperties.lOutsideVolumeHF;
    prop->DopplerFactor         = eaxBufferProperties.flDopplerFactor;
    prop->RolloffFactor         = eaxBufferProperties.flRolloffFactor;
    prop->RoomRolloffFactor     = eaxBufferProperties.flRoomRolloffFactor;
    prop->AirAbsorptionFactor   = eaxBufferProperties.flAirAbsorptionFactor;
    prop->Flags                 = eaxBufferProperties.ulFlags;

    return FMOD_OK;
}

}

#endif /* FMOD_SUPPORT_EAX */
#endif /* FMOD_SUPPORT_OPENAL */
