#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_EAX

#include "fmod_output_dsound.h"
#include "fmod_eax3.h"
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
// VERIFY: ba34dd52 auto
// OFFSET: 0x009506C0
bool OutputDSound::querySupportEAX3(unsigned int ulQuery)
{
    unsigned long ulSupport = 0;
    HRESULT       hr;
	GUID          guid;

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::querySupportEAX3", "check properties %08X\n", ulQuery));

    FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX30_ListenerProperties, sizeof(GUID));

    hr = mBufferReverb->QuerySupport(guid, ulQuery, &ulSupport);
	if ( FAILED(hr) ) 
    {
		return false;
    }

	if ( (ulSupport&(KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET)) == 
                    (KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET) )
	{
        mFeaturesReverb |= (DWORD)(1 << ulQuery);

		FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::querySupportEAX3", "success\n"));

		return true;
	}

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::querySupportEAX3", "failed.\n"));

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
// VERIFY: 1aa4b8e4 auto
// OFFSET: 0x00950740
bool OutputDSound::initEAX3()
{
    FMOD_RESULT             result;
	ULONG					support = 0;
	HRESULT					hr = DS_OK;
    GUID                    guid;
    FMOD_CODEC_WAVEFORMAT   waveformat;
 
    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX4", "Querying EAX3\n"));

    if (mBufferReverb)
	{
		mBufferReverb->Release();
		mBufferReverb = NULL;
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
        FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX3", "could not create hardware sample.\n"));
        return false;
    }

    mSampleReverb->mSystem = mSystem;

    /*
	    OBTAIN EAX INTERFACE TO BUFFER 
	*/
    FMOD_memcpy(&guid, &FMOD_IID_IKsPropertySet, sizeof(GUID));

    if(mSampleReverb->mBuffer3D->QueryInterface(guid, (void **)&mBufferReverb) != DS_OK)
    {
        mBufferReverb = NULL;
		return false;
	}

	/*
	    QUERY FOR EAX support
	*/
    support = querySupportEAX3(DSPROPERTY_EAXLISTENER_NONE);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_ALLPARAMETERS);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_ENVIRONMENT);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_ROOM);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_ROOMHF);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_ROOMLF);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_DECAYTIME);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_DECAYHFRATIO);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_DECAYLFRATIO);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_REFLECTIONS);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_REFLECTIONSPAN);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_REVERB);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_REVERBDELAY);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_REVERBPAN);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_ECHOTIME);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_ECHODEPTH);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_MODULATIONTIME);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_MODULATIONDEPTH);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_HFREFERENCE);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_LFREFERENCE);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR);
    if (support) support = querySupportEAX3(DSPROPERTY_EAXLISTENER_FLAGS);  

	if (!mFeaturesReverb)
	{
		FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX3", "failed EAX check, releasing reverb interface\n"));
		return false;
	}

	/*
        Turn down reverb to start with.
    */
    if ((mFeaturesReverb & (DWORD)1 << DSPROPERTY_EAXLISTENER_ROOM))
    {
        DWORD Room = EAXLISTENER_MINROOM;

        FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX3", "setting reverb environment to OFF\n"));

        FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX30_ListenerProperties, sizeof(GUID));

        mBufferReverb->Set(guid, DSPROPERTY_EAXLISTENER_ROOM, NULL, 0, &Room, sizeof(DWORD));
    }

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX3", "found!\n"));

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
// VERIFY: 44db8f51 auto
// OFFSET: 0x00950A50
FMOD_RESULT OutputDSound::closeEAX3()
{
	if (mBufferReverb)
    {
	    /*
            Turn down reverb again
        */
        if ((mFeaturesReverb & (DWORD)1 << DSPROPERTY_EAXLISTENER_ROOM))
        {
            GUID  guid;
            DWORD Room = EAXLISTENER_MINROOM;

            FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX30_ListenerProperties, sizeof(GUID));

            mBufferReverb->Set(guid, DSPROPERTY_EAXLISTENER_ROOM, NULL, 0, &Room, sizeof(DWORD));
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
// VERIFY: e6ed0a8c manual
// OFFSET: 0x00950AF0
FMOD_RESULT OutputDSound::setPropertiesEAX3(const FMOD_REVERB_PROPERTIES *prop)
{
    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

//	FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::setPropertiesEAX3", "Entering \n"));

	if ( !(mFeaturesReverb & (DWORD)1 << DSPROPERTY_EAXLISTENER_ALLPARAMETERS) ) 
	{
        FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::setPropertiesEAX3", "failed - apparently DSPROPERTY_EAXLISTENER_ALLPARAMETERS isnt supported\n"));

		return FMOD_ERR_UNSUPPORTED;
	}

	if (mFeaturesReverb & (DWORD)1 << DSPROPERTY_EAXLISTENER_ALLPARAMETERS)
    {
		HRESULT hr;
        EAXLISTENERPROPERTIES   dsprop = {0};
        GUID    guid;

//    	FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::setPropertiesEAX3", "setting listener properties\n"));

        dsprop.ulEnvironment            = prop->Environment < 0 ? 0 : prop->Environment;       
        dsprop.flEnvironmentSize        = prop->EnvSize;           
        dsprop.flEnvironmentDiffusion   = prop->EnvDiffusion;      
        dsprop.lRoom                    = prop->Room;              
        dsprop.lRoomHF                  = prop->RoomHF;            
        dsprop.lRoomLF                  = prop->RoomLF;            
        dsprop.flDecayTime              = prop->DecayTime;         
        dsprop.flDecayHFRatio           = prop->DecayHFRatio;      
        dsprop.flDecayLFRatio           = prop->DecayLFRatio;      
        dsprop.lReflections             = prop->Reflections;       
        dsprop.flReflectionsDelay       = prop->ReflectionsDelay;  
        dsprop.vReflectionsPan.x        = prop->ReflectionsPan[0]; 
        dsprop.vReflectionsPan.y        = prop->ReflectionsPan[1]; 
        dsprop.vReflectionsPan.z        = prop->ReflectionsPan[2]; 
        dsprop.lReverb                  = prop->Reverb;            
        dsprop.flReverbDelay            = prop->ReverbDelay;       
        dsprop.vReverbPan.x             = prop->ReverbPan[0];
        dsprop.vReverbPan.y             = prop->ReverbPan[1];
        dsprop.vReverbPan.z             = prop->ReverbPan[2];
        dsprop.flEchoTime               = prop->EchoTime;          
        dsprop.flEchoDepth              = prop->EchoDepth;         
        dsprop.flModulationTime         = prop->ModulationTime;    
        dsprop.flModulationDepth        = prop->ModulationDepth;   
        dsprop.flAirAbsorptionHF        = prop->AirAbsorptionHF;   
        dsprop.flHFReference            = prop->HFReference;       
        dsprop.flLFReference            = prop->LFReference;       
        dsprop.flRoomRolloffFactor      = prop->RoomRolloffFactor; 
        dsprop.ulFlags                  = prop->Flags & 0xFF; 
        
        FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX30_ListenerProperties, sizeof(GUID));

        hr = mBufferReverb->Set(guid, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, NULL, 0, &dsprop, sizeof(EAXLISTENERPROPERTIES));

        if (FAILED(hr))
        {
        	FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::setPropertiesEAX3", "setting listener properties FAILED\n"));

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
FMOD_RESULT	OutputDSound::getPropertiesEAX3(FMOD_REVERB_PROPERTIES *prop)
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

        FMOD_memcpy(&guid, &FMOD_DSPROPSETID_EAX30_ListenerProperties, sizeof(GUID));

		hr = mBufferReverb->Get(guid, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, NULL, 0, &dsprop, sizeof(EAXLISTENERPROPERTIES), &ulReceived);
        if (FAILED(hr))
        {
     		return FMOD_ERR_UNSUPPORTED;
        }

        prop->Environment       = dsprop.ulEnvironment            ;
        prop->EnvSize           = dsprop.flEnvironmentSize        ;
        prop->EnvDiffusion      = dsprop.flEnvironmentDiffusion   ;
        prop->Room              = dsprop.lRoom                    ;
        prop->RoomHF            = dsprop.lRoomHF                  ;
        prop->RoomLF            = dsprop.lRoomLF                  ;
        prop->DecayTime         = dsprop.flDecayTime              ;
        prop->DecayHFRatio      = dsprop.flDecayHFRatio           ;
        prop->DecayLFRatio      = dsprop.flDecayLFRatio           ;
        prop->Reflections       = dsprop.lReflections             ;
        prop->ReflectionsDelay  = dsprop.flReflectionsDelay       ;
        prop->ReflectionsPan[0] = dsprop.vReflectionsPan.x        ;
        prop->ReflectionsPan[1] = dsprop.vReflectionsPan.y        ;
        prop->ReflectionsPan[2] = dsprop.vReflectionsPan.z        ;
        prop->Reverb            = dsprop.lReverb                  ;
        prop->ReverbDelay       = dsprop.flReverbDelay            ;
        prop->ReverbPan[0]      = dsprop.vReverbPan.x             ;
        prop->ReverbPan[1]      = dsprop.vReverbPan.y             ;
        prop->ReverbPan[2]      = dsprop.vReverbPan.z             ;
        prop->EchoTime          = dsprop.flEchoTime               ;
        prop->EchoDepth         = dsprop.flEchoDepth              ;
        prop->ModulationTime    = dsprop.flModulationTime         ;
        prop->ModulationDepth   = dsprop.flModulationDepth        ;
        prop->AirAbsorptionHF   = dsprop.flAirAbsorptionHF        ;
        prop->HFReference       = dsprop.flHFReference            ;
        prop->LFReference       = dsprop.flLFReference            ;
        prop->RoomRolloffFactor = dsprop.flRoomRolloffFactor      ;
        prop->Flags             = dsprop.ulFlags                  ;
    }

	return FMOD_OK;
}

}

#endif  /* FMOD_SUPPORT_EAX */

