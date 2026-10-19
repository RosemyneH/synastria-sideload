#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL
#ifdef FMOD_SUPPORT_EAX

#include "fmod_output_openal.h"
#include "fmod_eax3.h"

namespace FMOD
{

/*
[
	[DESCRIPTION]
    Set properties for EAX3

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 8fd30f35 manual
// OFFSET: 0x0094AEF0
FMOD_RESULT OutputOpenAL::setPropertiesEAX3(const FMOD_REVERB_PROPERTIES *prop)
{
    GUID                   bufferGUID;
    GUID                   listenerGUID;
    EAXLISTENERPROPERTIES  eaxProperties;
    int                    room;

    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputOpenAL::setPropertiesEAX3", "Setting reverb properties\n"));

    /*
        Make sure all reverb effects are disabled on the mixer sources
    */
    if (!mMixerReverbDisabled)
    {
        room = -10000;
        FMOD_memcpy(&bufferGUID, &FMOD_DSPROPSETID_EAX30_BufferProperties, sizeof(GUID));

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

    FMOD_memcpy(&listenerGUID, &FMOD_DSPROPSETID_EAX30_ListenerProperties, sizeof(GUID));

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

    mEAXSet(&listenerGUID, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, 0, &eaxProperties, sizeof(EAXLISTENERPROPERTIES));
    if (mOALFnTable.alGetError() != AL_NO_ERROR)
    {
        FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputOpenAL::setPropertiesEAX3", "Error setting reverb properties\n"));
        return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

	return FMOD_OK;
}

}       /* namespace FMOD */

#endif  /* FMOD_SUPPORT_EAX */
#endif  /* FMOD_SUPPORT_OPENAL */
