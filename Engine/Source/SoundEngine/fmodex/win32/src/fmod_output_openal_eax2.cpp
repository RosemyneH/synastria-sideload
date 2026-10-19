#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL
#ifdef FMOD_SUPPORT_EAX

#include "fmod_output_openal.h"
#include "fmod_eax2.h"

namespace FMOD
{

/*
[
	[DESCRIPTION]
    Set properties for EAX2

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: f5bc842a manual
// OFFSET: 0x0094AD70
FMOD_RESULT OutputOpenAL::setPropertiesEAX2(const FMOD_REVERB_PROPERTIES *prop)
{
    GUID                   bufferGUID;
    GUID                   listenerGUID;
    EAXLISTENERPROPERTIES  eaxProperties;
    int                    room;

    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::setPropertiesEAX2", "Setting reverb properties\n"));

    /*
        Make sure all reverb effects are disabled on the mixer sources
    */
    if (!mMixerReverbDisabled)
    {
        room = -10000;
        FMOD_memcpy(&bufferGUID, &FMOD_DSPROPSETID_EAX20_BufferProperties, sizeof(GUID));

        // Only the first mixer source is technically used (multichannel source), but set all anyway
        for (int i = mMixerSourceOffset; i < mNumSources; i++)
        {
            mEAXSet(&bufferGUID, DSPROPERTY_EAXBUFFER_ROOM, mSources[i].sid, &room, sizeof(int));
            if (mOALFnTable.alGetError() != AL_NO_ERROR)
            {
		        return FMOD_ERR_INTERNAL;
            }
        }
        
        mMixerReverbDisabled = true;
    }

    FMOD_memcpy(&listenerGUID, &FMOD_DSPROPSETID_EAX20_ListenerProperties, sizeof(GUID));

    eaxProperties.flEnvironmentSize       = prop->EnvSize;           
    eaxProperties.dwEnvironment           = prop->Environment < 0 ? 0 : prop->Environment;
    eaxProperties.flEnvironmentDiffusion  = prop->EnvDiffusion;
    eaxProperties.lRoom                   = prop->Room;
    eaxProperties.lRoomHF                 = prop->RoomHF;
    eaxProperties.flDecayTime             = prop->DecayTime;
    eaxProperties.flDecayHFRatio          = prop->DecayHFRatio;
    eaxProperties.lReflections            = prop->Reflections;
    eaxProperties.flReflectionsDelay      = prop->ReflectionsDelay;
    eaxProperties.lReverb                 = prop->Reverb;
    eaxProperties.flReverbDelay           = prop->ReverbDelay;
    eaxProperties.flAirAbsorptionHF       = prop->AirAbsorptionHF;
    eaxProperties.flRoomRolloffFactor     = prop->RoomRolloffFactor;
    eaxProperties.dwFlags                 = prop->Flags & 0xFF;     // Mask out FMOD specific flags, leaving only EAX flags

    mEAXSet(&listenerGUID, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, 0, &eaxProperties, sizeof(EAXLISTENERPROPERTIES));
    if (mOALFnTable.alGetError() != AL_NO_ERROR)
    {
        FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputOpenAL::setPropertiesEAX2", "Error setting reverb properties\n"));
        return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

	return FMOD_OK;
}

}       /* namespace FMOD */

#endif  /* FMOD_SUPPORT_EAX */
#endif  /* FMOD_SUPPORT_OPENAL */
