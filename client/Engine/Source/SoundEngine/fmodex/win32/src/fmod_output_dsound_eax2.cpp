#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_EAX

#include "fmod_output_dsound.h"
#include "fmod_eax2.h"
#include "fmod_localcriticalsection.h"
#include "fmod_sample_dsound.h"
#include "fmod_systemi.h"

namespace FMOD
{


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 5bac5638 auto
// OFFSET: 0x00950C50
bool OutputDSound::querySupportEAX2(unsigned int ulQuery)
{
	unsigned long ulSupport = 0;
    HRESULT       hr;
	GUID          guid;

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::querySupportEAX2", "check properties %08X\n", ulQuery));

    FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX20_ListenerProperties, sizeof(GUID));

    hr = mBufferReverb->QuerySupport(guid, ulQuery, &ulSupport);
	if ( FAILED(hr) ) 
    {
		return false;
    }

	if ( (ulSupport&(KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET)) == 
                    (KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET) )
	{
        mFeaturesReverb |= (DWORD)(1 << ulQuery);

		FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::querySupportEAX2", "success.\n"));

		return true;
	}

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::querySupportEAX2", "failed.\n"));

	return false;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: ab594de7 auto
// OFFSET: 0x00950CD0
bool OutputDSound::initEAX2()
{
    FMOD_RESULT             result;
	ULONG					support = 0;
	HRESULT					hr = DS_OK;
    GUID                    guid;
    FMOD_CODEC_WAVEFORMAT   waveformat;

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX2", "Querying EAX2\n"));

	if (mBufferReverb)
	{
		mBufferReverb->Release();
		mBufferReverb = 0;
	}

    /*
        Create EAX sample
    */

    FMOD_memset(&waveformat, 0, sizeof(FMOD_CODEC_WAVEFORMAT));
    waveformat.frequency = 44100;
    waveformat.lengthpcm = 1024;
    waveformat.channels  = 1;
    waveformat.format    = FMOD_SOUND_FORMAT_PCM16;

    result = createSample(FMOD_3D | FMOD_UNIQUE, &waveformat, (Sample **)&mSampleReverb);
    if (result != FMOD_OK)
    {
        FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX2", "could not create hardware sample.\n"));
        return false;
    }

    mSampleReverb->mSystem = mSystem;

    /*
	    OBTAIN EAX INTERFACE TO BUFFER 
	*/
    FMOD_memcpy(&guid, &FMOD_IID_IKsPropertySet, sizeof(GUID));

    if(mSampleReverb->mBuffer3D->QueryInterface(guid, (void **)&mBufferReverb) != DS_OK)
	{
		mBufferReverb = 0;		
		return false;
	}

	/*
	    QUERY FOR EAX support
	*/
    support = querySupportEAX2(DSPROPERTY_EAXLISTENER_NONE);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_ALLPARAMETERS);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_ENVIRONMENT);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_ROOM);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_ROOMHF);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_DECAYTIME);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_DECAYHFRATIO);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_REFLECTIONS);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_REVERB);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_REVERBDELAY);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR);
    if (support) support = querySupportEAX2(DSPROPERTY_EAXLISTENER_FLAGS);
    
	if (!mFeaturesReverb)
	{
		FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX2", "failed EAX check, releasing reverb interface\n"));
		return false;
	}

	/*
        Turn down reverb to start with.
    */
    if ((mFeaturesReverb & (DWORD)1 << DSPROPERTY_EAXLISTENER_ROOM))
    {
        DWORD Room = EAXLISTENER_MINROOM;

        FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX2", "setting reverb environment to OFF\n"));

        FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX20_ListenerProperties, sizeof(GUID));

        mBufferReverb->Set(guid, DSPROPERTY_EAXLISTENER_ROOM, 0, 0, &Room, sizeof(DWORD));
    }

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX2", "found!\n"));

	return true;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: ffdd1e70 auto
// OFFSET: 0x00950F20
FMOD_RESULT OutputDSound::closeEAX2()
{
	if (mBufferReverb) 
	{
	    /*
            Turn down reverb again
        */
        if ((mFeaturesReverb & (DWORD)1 << DSPROPERTY_EAXLISTENER_ROOM))
        {
            GUID    guid;
            DWORD   Room = EAXLISTENER_MINROOM;

            FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX20_ListenerProperties, sizeof(GUID));

    	    mBufferReverb->Set(guid, DSPROPERTY_EAXLISTENER_ROOM, 0, 0, &Room, sizeof(DWORD));
        }

		mBufferReverb->Release();
		mBufferReverb = NULL;
	}

    if (mSampleReverb)
    {
	    mSampleReverb->release();
        mSampleReverb = NULL;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: a244fd78 manual
// OFFSET: 0x00950FC0
FMOD_RESULT OutputDSound::setPropertiesEAX2(const FMOD_REVERB_PROPERTIES *prop)
{
    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

	FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::setPropertiesEAX2", "Entering \n"));

	if ( !(mFeaturesReverb & (DWORD)1 << DSPROPERTY_EAXLISTENER_ALLPARAMETERS) ) 
	{
        FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::setPropertiesEAX2", "failed - apparently DSPROPERTY_EAXLISTENER_ALLPARAMETERS isnt supported\n"));

		return FMOD_ERR_UNSUPPORTED;
	}

	if (mFeaturesReverb & (DWORD)1 << DSPROPERTY_EAXLISTENER_ALLPARAMETERS)
    {
		HRESULT hr;
        EAXLISTENERPROPERTIES   dsprop = {0};
        GUID    guid;

    	FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::setPropertiesEAX2", "setting listener properties\n"));

        dsprop.flEnvironmentSize        = prop->EnvSize;           
        dsprop.dwEnvironment            = prop->Environment < 0 ? 0 : prop->Environment;
        dsprop.flEnvironmentDiffusion   = prop->EnvDiffusion;      
        dsprop.lRoom                    = prop->Room;              
        dsprop.lRoomHF                  = prop->RoomHF;            
        dsprop.flDecayTime              = prop->DecayTime;         
        dsprop.flDecayHFRatio           = prop->DecayHFRatio;      
        dsprop.lReflections             = prop->Reflections;       
        dsprop.flReflectionsDelay       = prop->ReflectionsDelay;  
        dsprop.lReverb                  = prop->Reverb;            
        dsprop.flReverbDelay            = prop->ReverbDelay;       
        dsprop.flAirAbsorptionHF        = prop->AirAbsorptionHF;   
        dsprop.flRoomRolloffFactor      = prop->RoomRolloffFactor; 
        dsprop.dwFlags                  = prop->Flags & 0xFF;
        
        FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX20_ListenerProperties, sizeof(GUID));

        hr = mBufferReverb->Set(guid, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, 0, 0, &dsprop, sizeof(EAXLISTENERPROPERTIES));

        if (FAILED(hr))
        {
        	FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::setPropertiesEAX2", "setting listener properties FAILED\n"));

            return FMOD_ERR_INVALID_PARAM;
        }
    }

	return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
FMOD_RESULT	OutputDSound::getPropertiesEAX2(FMOD_REVERB_PROPERTIES *prop)
{
	ULONG   ulReceived;

    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

	if ( !(mFeaturesReverb & (DWORD)1 << DSPROPERTY_EAXLISTENER_ALLPARAMETERS) ) 
	{
		return FMOD_ERR_UNSUPPORTED;
	}

	if ((mFeaturesReverb & (DWORD)1 << DSPROPERTY_EAXLISTENER_ALLPARAMETERS))
    {
        GUID    guid;
    	HRESULT hr;
        EAXLISTENERPROPERTIES   dsprop = {0};

        FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX20_ListenerProperties, sizeof(GUID));

		hr = mBufferReverb->Get(guid, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, 0, 0, &dsprop, sizeof(EAXLISTENERPROPERTIES), &ulReceived);
        if (FAILED(hr))
        {
     		return FMOD_ERR_UNSUPPORTED;
        }

        prop->Environment       = dsprop.dwEnvironment;
        prop->EnvSize           = dsprop.flEnvironmentSize        ;
        prop->EnvDiffusion      = dsprop.flEnvironmentDiffusion   ;
        prop->Room              = dsprop.lRoom                    ;
        prop->RoomHF            = dsprop.lRoomHF                  ;
        prop->RoomLF            = 0;
        prop->DecayTime         = dsprop.flDecayTime              ;
        prop->DecayHFRatio      = dsprop.flDecayHFRatio           ;
        prop->DecayLFRatio      = 0;
        prop->Reflections       = dsprop.lReflections             ;
        prop->ReflectionsDelay  = dsprop.flReflectionsDelay       ;
        prop->ReflectionsPan[0] = 0;
        prop->ReflectionsPan[1] = 0;
        prop->ReflectionsPan[2] = 0;
        prop->Reverb            = dsprop.lReverb                  ;
        prop->ReverbDelay       = dsprop.flReverbDelay            ;
        prop->ReverbPan[0]      = 0;
        prop->ReverbPan[1]      = 0;
        prop->ReverbPan[2]      = 0;
        prop->EchoTime          = 0;
        prop->EchoDepth         = 0;
        prop->ModulationTime    = 0;
        prop->ModulationDepth   = 0;
        prop->AirAbsorptionHF   = dsprop.flAirAbsorptionHF        ;
        prop->HFReference       = 0;
        prop->LFReference       = 0;
        prop->RoomRolloffFactor = dsprop.flRoomRolloffFactor      ;
        prop->Flags             = dsprop.dwFlags                  ;
    }

	return FMOD_OK;
}

}

#endif  /* FMOD_SUPPORT_EAX */
