#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_EAX

#include "fmod_output_dsound.h"
#include "fmod_eax4.h"
#include "fmod_localcriticalsection.h"
#include "fmod_sample_dsound.h"
#include "fmod_systemi.h"


#include <stdio.h>

namespace FMOD
{

bool gFXslot2set = false;
bool gFXslot3set = false;


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 7ff3efa2 auto
// OFFSET: 0x0094FEB0
bool OutputDSound::querySupportEAX4(FMOD_GUID guid, unsigned long ulProperty)
{
    HRESULT       hr;
    unsigned long ulSupport;
    GUID          g;

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::querySupportEAX4", "check properties %08X\n", ulProperty));

    FMOD_memcpy(&g, &guid, sizeof(GUID));

    hr = mBufferReverb->QuerySupport(g, ulProperty, &ulSupport);
	if ( FAILED(hr) ) 
    {
		return false;
    }

	if ( (ulSupport&(KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET)) == 
                    (KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET) )
	{
		FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::querySupportEAX4", "success.\n"));

		return true;
	}

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::querySupportEAX4", "failed.\n"));

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
// VERIFY: efc83e19 auto
// OFFSET: 0x0094FF10
bool OutputDSound::initEAX4()
{
    FMOD_RESULT             result;
	ULONG					support = 0;
	HRESULT					hr = DS_OK;
    GUID                    guid;
    FMOD_CODEC_WAVEFORMAT   waveformat;

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX4", "Querying EAX4\n"));

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
        FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX4", "could not create hardware sample.\n"));
        return false;
    }

    mSampleReverb->mSystem = mSystem;


    /*
	    OBTAIN EAX INTERFACE TO BUFFER 
	*/
    FMOD_memcpy(&guid, &FMOD_IID_IKsPropertySet, sizeof(GUID));

    if(mSampleReverb->mBuffer3D->QueryInterface(guid, (void **)&mBufferReverb) != DS_OK)
    {
        FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX4", "failed (1).\n"));

        mBufferReverb = NULL;
		return false;
	}

	/*
	    QUERY FOR EAX support
	*/
    support = querySupportEAX4(FMOD_EAXPROPERTYID_EAX40_Context, EAXCONTEXT_ALLPARAMETERS);
    if (support) support = querySupportEAX4(FMOD_EAXPROPERTYID_EAX40_FXSlot0, EAXFXSLOT_ALLPARAMETERS);  
    if (support) support = querySupportEAX4(FMOD_EAXPROPERTYID_EAX40_FXSlot1, EAXFXSLOT_ALLPARAMETERS);  
    if (support) support = querySupportEAX4(FMOD_EAXPROPERTYID_EAX40_FXSlot2, EAXFXSLOT_ALLPARAMETERS);  
	if (support) support = querySupportEAX4(FMOD_EAXPROPERTYID_EAX40_FXSlot3, EAXFXSLOT_ALLPARAMETERS);  
	if (support) support = querySupportEAX4(FMOD_EAXPROPERTYID_EAX40_Source, EAXSOURCE_ALLPARAMETERS);

    if (support) 
	{
		FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX4", "EAX 4.0 - success\n"));
	}
    else
    {
        FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX4", "failed (2).\n"));

        return false;
    }


    /*
        Turn down reverb to start with
    */
    {
        long room = EAXREVERB_MINROOM;

        FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX4", "setting reverb environment to OFF\n"));

        FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX40_FXSlot0, sizeof(GUID));
        hr = mBufferReverb->Set(guid, EAXREVERB_ROOM, 0, 0, &room, sizeof(long));
        if (FAILED(hr))
        {
            FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX4", "failed (3).\n"));
            return false;
        }
    }
    
    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputDSound::initEAX4", "found!\n"));

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
// VERIFY: 9bf7fcd8 auto
// OFFSET: 0x009501A0
FMOD_RESULT OutputDSound::closeEAX4()
{
    GUID guid;
    long lock = EAXFXSLOT_UNLOCKED;

	if (mBufferReverb)
    {
        /*
            Unlock FX slots
        */
        FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX40_FXSlot2, sizeof(GUID));
        mBufferReverb->Set(guid, EAXFXSLOT_LOCKED, 0, 0, &lock, sizeof(long));

        FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX40_FXSlot3, sizeof(GUID));
        mBufferReverb->Set(guid, EAXFXSLOT_LOCKED, 0, 0, &lock, sizeof(long));

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
// VERIFY: c8679ea7 manual
// OFFSET: 0x00950280
FMOD_RESULT OutputDSound::setPropertiesEAX4(const FMOD_REVERB_PROPERTIES *prop)
{
    HRESULT hr;
    EAXREVERBPROPERTIES   eaxprop = {0};
    EAXFXSLOTPROPERTIES   fxslot;
    GUID    guid;

    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    switch(prop->Instance)
    {
        case 1:
        {
            FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX40_FXSlot2, sizeof(GUID));

            if (!gFXslot2set)
            {
                FMOD_memcpy(&fxslot.guidLoadEffect, &FMOD_EAX_REVERB_EFFECT, sizeof(GUID));
                fxslot.lLock   = EAXFXSLOT_LOCKED;
                fxslot.lVolume = 0;
                fxslot.ulFlags = EAXFXSLOTFLAGS_ENVIRONMENT;
    
                hr = mBufferReverb->Set(guid, EAXFXSLOT_ALLPARAMETERS, NULL, 0, &fxslot, sizeof(EAXFXSLOTPROPERTIES));
                if (FAILED(hr))
                {
                    ULONG   ulReceived;
                    GUID    effect;

                    /*
                        Check if the slot contains a reverb. If so, we can still use that slot.
                    */
                    hr = mBufferReverb->Get(guid, EAXFXSLOT_LOADEFFECT, NULL, 0, &effect, sizeof(GUID), &ulReceived);
                    if (FAILED(hr))
                    {
                        return FMOD_ERR_REVERB_INSTANCE;
                    }
                    
                    if (memcmp(&effect, &fxslot.guidLoadEffect, sizeof(GUID)))
                    {
                        return FMOD_ERR_REVERB_INSTANCE;
                    }
                }

                gFXslot2set = true;
            }

            break;
        }
        case 2:
        {
            FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX40_FXSlot3, sizeof(GUID));

            if (!gFXslot3set)
            {
                FMOD_memcpy(&fxslot.guidLoadEffect, &FMOD_EAX_REVERB_EFFECT, sizeof(GUID));
                fxslot.lLock   = EAXFXSLOT_LOCKED;
                fxslot.lVolume = 0;
                fxslot.ulFlags = EAXFXSLOTFLAGS_ENVIRONMENT;
    
                hr = mBufferReverb->Set(guid, EAXFXSLOT_ALLPARAMETERS, NULL, 0, &fxslot, sizeof(EAXFXSLOTPROPERTIES));
                if (FAILED(hr))
                {
                    ULONG   ulReceived;
                    GUID    effect;

                    /*
                        Check if the slot contains a reverb. If so, we can still use that slot.
                    */
                    hr = mBufferReverb->Get(guid, EAXFXSLOT_LOADEFFECT, NULL, 0, &effect, sizeof(GUID), &ulReceived);
                    if (FAILED(hr))
                    {
                        return FMOD_ERR_REVERB_INSTANCE;
                    }
                    
                    if (memcmp(&effect, &fxslot.guidLoadEffect, sizeof(GUID)))
                    {
                        return FMOD_ERR_REVERB_INSTANCE;
                    }
                }

                gFXslot3set = true;
            }

            break;
        }
        default:
            FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX40_FXSlot0, sizeof(GUID));
            break;
    }

    eaxprop.ulEnvironment            = prop->Environment < 0 ? 0 : prop->Environment;
    eaxprop.flEnvironmentSize        = prop->EnvSize;           
    eaxprop.flEnvironmentDiffusion   = prop->EnvDiffusion;      
    eaxprop.lRoom                    = prop->Room;              
    eaxprop.lRoomHF                  = prop->RoomHF;            
    eaxprop.lRoomLF                  = prop->RoomLF;            
    eaxprop.flDecayTime              = prop->DecayTime;         
    eaxprop.flDecayHFRatio           = prop->DecayHFRatio;      
    eaxprop.flDecayLFRatio           = prop->DecayLFRatio;      
    eaxprop.lReflections             = prop->Reflections;       
    eaxprop.flReflectionsDelay       = prop->ReflectionsDelay;  
    eaxprop.vReflectionsPan.x        = prop->ReflectionsPan[0]; 
    eaxprop.vReflectionsPan.y        = prop->ReflectionsPan[1]; 
    eaxprop.vReflectionsPan.z        = prop->ReflectionsPan[2]; 
    eaxprop.lReverb                  = prop->Reverb;            
    eaxprop.flReverbDelay            = prop->ReverbDelay;       
    eaxprop.vReverbPan.x             = prop->ReverbPan[0];
    eaxprop.vReverbPan.y             = prop->ReverbPan[1];
    eaxprop.vReverbPan.z             = prop->ReverbPan[2];
    eaxprop.flEchoTime               = prop->EchoTime;          
    eaxprop.flEchoDepth              = prop->EchoDepth;         
    eaxprop.flModulationTime         = prop->ModulationTime;    
    eaxprop.flModulationDepth        = prop->ModulationDepth;   
    eaxprop.flAirAbsorptionHF        = prop->AirAbsorptionHF;   
    eaxprop.flHFReference            = prop->HFReference;       
    eaxprop.flLFReference            = prop->LFReference;       
    eaxprop.flRoomRolloffFactor      = prop->RoomRolloffFactor; 
    eaxprop.ulFlags                  = prop->Flags & 0xFF; 
    
    hr = mBufferReverb->Set(guid, EAXREVERB_ALLPARAMETERS, NULL, 0, &eaxprop, sizeof(EAXREVERBPROPERTIES));
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

	[REMARKS]

	[SEE_ALSO]
]
*/
FMOD_RESULT	OutputDSound::getPropertiesEAX4(FMOD_REVERB_PROPERTIES *prop)
{
    HRESULT hr;
    EAXREVERBPROPERTIES   eaxprop;
	ULONG   ulReceived;
    GUID    guid;

    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    switch(prop->Instance)
    {
        case 1:
            FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX40_FXSlot2, sizeof(GUID));
            break;
        case 2:
            FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX40_FXSlot3, sizeof(GUID));
            break;
        default:
            FMOD_memcpy(&guid, &FMOD_EAXPROPERTYID_EAX40_FXSlot0, sizeof(GUID));
            break;
    }

	hr = mBufferReverb->Get(guid, EAXREVERB_ALLPARAMETERS, NULL, 0, &eaxprop, sizeof(EAXREVERBPROPERTIES), &ulReceived);
    if (FAILED(hr))
    {
     	return FMOD_ERR_UNSUPPORTED;
    }

    prop->Environment       = eaxprop.ulEnvironment            ;
    prop->EnvSize           = eaxprop.flEnvironmentSize        ;
    prop->EnvDiffusion      = eaxprop.flEnvironmentDiffusion   ;
    prop->Room              = eaxprop.lRoom                    ;
    prop->RoomHF            = eaxprop.lRoomHF                  ;
    prop->RoomLF            = eaxprop.lRoomLF                  ;
    prop->DecayTime         = eaxprop.flDecayTime              ;
    prop->DecayHFRatio      = eaxprop.flDecayHFRatio           ;
    prop->DecayLFRatio      = eaxprop.flDecayLFRatio           ;
    prop->Reflections       = eaxprop.lReflections             ;
    prop->ReflectionsDelay  = eaxprop.flReflectionsDelay       ;
    prop->ReflectionsPan[0] = eaxprop.vReflectionsPan.x        ;
    prop->ReflectionsPan[1] = eaxprop.vReflectionsPan.y        ;
    prop->ReflectionsPan[2] = eaxprop.vReflectionsPan.z        ;
    prop->Reverb            = eaxprop.lReverb                  ;
    prop->ReverbDelay       = eaxprop.flReverbDelay            ;
    prop->ReverbPan[0]      = eaxprop.vReverbPan.x             ;
    prop->ReverbPan[1]      = eaxprop.vReverbPan.y             ;
    prop->ReverbPan[2]      = eaxprop.vReverbPan.z             ;
    prop->EchoTime          = eaxprop.flEchoTime               ;
    prop->EchoDepth         = eaxprop.flEchoDepth              ;
    prop->ModulationTime    = eaxprop.flModulationTime         ;
    prop->ModulationDepth   = eaxprop.flModulationDepth        ;
    prop->AirAbsorptionHF   = eaxprop.flAirAbsorptionHF        ;
    prop->HFReference       = eaxprop.flHFReference            ;
    prop->LFReference       = eaxprop.flLFReference            ;
    prop->RoomRolloffFactor = eaxprop.flRoomRolloffFactor      ;
    prop->Flags             = eaxprop.ulFlags                  ;

	return FMOD_OK;
}

}

#endif  /* FMOD_SUPPORT_EAX */

