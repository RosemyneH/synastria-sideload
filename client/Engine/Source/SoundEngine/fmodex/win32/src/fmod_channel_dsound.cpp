#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_DSOUND

#include "fmod_channel_dsound.h"
#include "fmod_debug.h"
#include "fmod_output_dsound.h"
#include "fmod_sample_dsound.h"
#include "fmod_soundi.h"
#include "fmod_systemi.h"
#include "fmod_3d.h"

#include <dsound.h>
#include <stdio.h>

namespace FMOD
{

static const FMOD_GUID IID_IDirectSound3DBuffer = { 0x279AFA86, 0x4981, 0x11CE, { 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60} };
static const FMOD_GUID IID_KSPROPSETID_Audio    = { 0x45FFAAA0, 0x6E1B, 0x11D0, { 0xBC, 0xF2, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00} };

typedef enum {
    KSPROPERTY_AUDIO_LATENCY = 1,
    KSPROPERTY_AUDIO_COPY_PROTECTION,
    KSPROPERTY_AUDIO_CHANNEL_CONFIG,
    KSPROPERTY_AUDIO_VOLUMELEVEL,
    KSPROPERTY_AUDIO_POSITION,
    KSPROPERTY_AUDIO_DYNAMIC_RANGE,
    KSPROPERTY_AUDIO_QUALITY,
    KSPROPERTY_AUDIO_SAMPLING_RATE,
    KSPROPERTY_AUDIO_DYNAMIC_SAMPLING_RATE,
    KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,
    KSPROPERTY_AUDIO_MIX_LEVEL_CAPS,
    KSPROPERTY_AUDIO_MUX_SOURCE,
    KSPROPERTY_AUDIO_MUTE,
    KSPROPERTY_AUDIO_BASS,
    KSPROPERTY_AUDIO_MID,
    KSPROPERTY_AUDIO_TREBLE,
    KSPROPERTY_AUDIO_BASS_BOOST,
    KSPROPERTY_AUDIO_EQ_LEVEL,
    KSPROPERTY_AUDIO_NUM_EQ_BANDS,
    KSPROPERTY_AUDIO_EQ_BANDS,
    KSPROPERTY_AUDIO_AGC,
    KSPROPERTY_AUDIO_DELAY,
    KSPROPERTY_AUDIO_LOUDNESS,
    KSPROPERTY_AUDIO_WIDE_MODE,
    KSPROPERTY_AUDIO_WIDENESS,
    KSPROPERTY_AUDIO_REVERB_LEVEL,
    KSPROPERTY_AUDIO_CHORUS_LEVEL,
    KSPROPERTY_AUDIO_DEV_SPECIFIC,
    KSPROPERTY_AUDIO_DEMUX_DEST,
    KSPROPERTY_AUDIO_STEREO_ENHANCE,
    KSPROPERTY_AUDIO_MANUFACTURE_GUID,
    KSPROPERTY_AUDIO_PRODUCT_GUID,
    KSPROPERTY_AUDIO_CPU_RESOURCES,
    KSPROPERTY_AUDIO_STEREO_SPEAKER_GEOMETRY,
    KSPROPERTY_AUDIO_SURROUND_ENCODE,
    KSPROPERTY_AUDIO_3D_INTERFACE,
    KSPROPERTY_AUDIO_PEAKMETER,
    KSPROPERTY_AUDIO_ALGORITHM_INSTANCE,
    KSPROPERTY_AUDIO_FILTER_STATE,
    KSPROPERTY_AUDIO_PREFERRED_STATUS
} KSPROPERTY_AUDIO;

// Audio quality constants
#define KSAUDIO_QUALITY_WORST               0x0
#define KSAUDIO_QUALITY_PC                  0x1
#define KSAUDIO_QUALITY_BASIC               0x2
#define KSAUDIO_QUALITY_ADVANCED            0x3

//#define FMOD_CHANNEL_DSOUND_QUALITY KSAUDIO_QUALITY_PC

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 444a402b auto
// OFFSET: 0x0094DCA0
FMOD_RESULT ChannelDSound::init(int index, SystemI *system, Output *output, DSPI *dspmixtarget)
{
    FMOD_RESULT result;

    result = ChannelReal::init(index, system, output, dspmixtarget);
    if (result != FMOD_OK)
    {
        return result;
    }
   
    mBuffer       = 0;
    mBuffer3D     = 0;
    mOutputDSound = (OutputDSound *)output;
    mMinFrequency = DSBFREQUENCY_MIN;
    mMaxFrequency = DSBFREQUENCY_MAX;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 565da013 manual
// OFFSET: 0x0094DD00
FMOD_RESULT ChannelDSound::alloc()
{
    FMOD_MODE       mode;
	HRESULT         hr;
    SampleDSound   *sampledsound;
    
    sampledsound = SAFE_CAST(SampleDSound, mSound);
    if (!sampledsound)
    {
    	return FMOD_ERR_INVALID_PARAM;
    }

    if (!mOutputDSound)
    {
    	return FMOD_ERR_UNINITIALIZED;
    }

	if (!sampledsound->mBuffer)
	{
		return FMOD_ERR_INVALID_PARAM;
	}

    mode = sampledsound->mMode;

	if (mode & FMOD_UNIQUE)
    {
		mBuffer = sampledsound->mBuffer;
    }
	else
	{
		hr = mOutputDSound->mDirectSound->DuplicateSoundBuffer(sampledsound->mBuffer, (IDirectSoundBuffer **)&mBuffer);
		if (hr != DS_OK)
		{
			return FMOD_ERR_OUTPUT_DRIVERCALL; 
		}
	}

    mLOCSoftware = sampledsound->mLOCSoftware;

    if (!mLOCSoftware)
    {
        mMinFrequency = mOutputDSound->mMinFrequency;
        mMaxFrequency = mOutputDSound->mMaxFrequency;
    }

	if (mOutputDSound->mChannelPool3D && mode & FMOD_3D)
	{
		/*
			OBTAIN DS3D INTERFACE TO BUFFER 
		*/
        if (mode & FMOD_UNIQUE)
        {
            mBuffer3D = sampledsound->mBuffer3D;
        }
        else
        {
            GUID guid;

            FMOD_memcpy(&guid, &IID_IDirectSound3DBuffer, sizeof(GUID));

		    if (mBuffer->QueryInterface(guid, (LPVOID *)&mBuffer3D) != DS_OK)
		    {
			    return FMOD_ERR_OUTPUT_DRIVERCALL;
		    }
        }

		/*
			SET DS3D PROPERTIES FOR THIS BUFFER
		*/

		if (mBuffer3D)
		{
			DS3DBUFFER buffparams;

			FMOD_memset(&buffparams, 0, sizeof(DS3DBUFFER));
			buffparams.dwSize = sizeof(DS3DBUFFER);

			mBuffer3D->GetAllParameters(&buffparams);
			
			buffparams.flMinDistance      = mParent->mMinDistance;
			buffparams.flMaxDistance      = mParent->mMaxDistance;
            buffparams.dwInsideConeAngle  = (DWORD)mParent->mConeInsideAngle;
            buffparams.dwOutsideConeAngle = (DWORD)mParent->mConeOutsideAngle;
            if (mParent->mConeOutsideVolume < 0.00001f)
            {
                buffparams.lConeOutsideVolume = DSBVOLUME_MIN;
            }
            else if (mParent->mConeOutsideVolume > 0.9999f)
            {
                buffparams.lConeOutsideVolume = DSBVOLUME_MAX;
            }
            else
            {
                buffparams.lConeOutsideVolume = (int)(100.0f * 20.0f * FMOD_LOG10(mParent->mConeOutsideVolume));
            }
            buffparams.vConeOrientation.x = mParent->mConeOrientation.x;
            buffparams.vConeOrientation.y = mParent->mConeOrientation.y;
            buffparams.vConeOrientation.z = mParent->mConeOrientation.z;
            if (mSystem->mFlags & FMOD_INIT_3D_RIGHTHANDED)
            {
                buffparams.vConeOrientation.z = -buffparams.vConeOrientation.z;
            }
            buffparams.dwMode             = (mode & FMOD_3D_HEADRELATIVE) ? DS3DMODE_HEADRELATIVE : DS3DMODE_NORMAL;

			mBuffer3D->SetAllParameters(&buffparams, DS3D_IMMEDIATE);
        }
    }

    /*
        Set the quality for this buffer.  (Default quality is horribly clicky.
    */
#ifdef FMOD_CHANNEL_DSOUND_QUALITY
    if (mLOCSoftware)
    {
        GUID guid;

        FMOD_memcpy(&guid, &FMOD_IID_IKsPropertySet, sizeof(GUID));
        
        HRESULT hr = mBuffer->QueryInterface(guid, (void**)&mBufferQuality);
    }
    else
    {
        mBufferQuality = 0;
    }
#endif

	return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 6e987adf auto
// OFFSET: 0x0094DF50
FMOD_RESULT ChannelDSound::start()
{
    SampleDSound   *sampledsound;
	HRESULT         hr;

    sampledsound = SAFE_CAST(SampleDSound, mSound);
    if (!sampledsound)
    {
    	return FMOD_ERR_INVALID_PARAM;
    }

	if (!sampledsound->mBuffer || !mBuffer)
	{
		FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "ChannelDSound::start", "Not a hardware channel or sample\n"));

		return FMOD_ERR_INVALID_PARAM;
	}

    if (!mOutputDSound)
    {
    	return FMOD_ERR_UNINITIALIZED;
    }


	/*
	    START BUFFER PLAYING
	*/
	if (!(mFlags & CHANNELREAL_FLAG_PAUSED))
	{
        FMOD_MODE   smode;
		DWORD       playmode = 0;
        
        smode = sampledsound->mMode;
	
		if (!(smode & FMOD_UNIQUE))
        {
            if (mLOCSoftware)
            {
	        	playmode |= DSBPLAY_LOCSOFTWARE;
			}
            else
            {
    			playmode |= DSBPLAY_LOCHARDWARE;
            }
        }

        if (mMode & (FMOD_LOOP_NORMAL | FMOD_LOOP_BIDI) && mLoopCount)
        {
            playmode |= DSBPLAY_LOOPING;
        }

		hr = mBuffer->Play(0,0, playmode);
		if (hr != DS_OK)
		{
			FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "ChannelDSound::start", "IDirectSoundBuffer::Play returned errcode %08X\n", hr));
            return FMOD_ERR_OUTPUT_DRIVERCALL;
		}

#ifdef FMOD_CHANNEL_DSOUND_QUALITY
        if (mBufferQuality)
        {
            GUID guid;
            DWORD dwQuality = FMOD_CHANNEL_DSOUND_QUALITY;

            FMOD_memcpy(&guid, &IID_KSPROPSETID_Audio, sizeof(GUID));

            mBufferQuality->Set(guid, KSPROPERTY_AUDIO_QUALITY, PVOID(&dwQuality), sizeof(dwQuality), PVOID(&dwQuality), sizeof(dwQuality));
        }
#endif
	}

	return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 73028d4a manual
// OFFSET: 0x0094DFD0
FMOD_RESULT ChannelDSound::stop(bool ignoreenddelay, bool resetstate)
{
    bool releasebuffers = true;

    if (!(mFlags & CHANNELREAL_FLAG_ENDDELAY) || ignoreenddelay)
    {
	    if (mBuffer)
        {
	        mBuffer->Stop();
	        mBuffer->SetCurrentPosition(0);
        }

	    if (mBufferReverb)
        {
		    mBufferReverb->Release();
            mBufferReverb = 0;
        }

        if (mSound && mSound->mMode & FMOD_UNIQUE)
        {
            releasebuffers = false;
        }

        if (releasebuffers)
        {
	        if (mBuffer3D)
            {
		        mBuffer3D->Release();
            }
	        if (mBuffer)
            {
		        mBuffer->Release();
            }
            #ifdef FMOD_CHANNEL_DSOUND_QUALITY
            if (mBufferQuality)
            {
                mBufferQuality->Release();
            }
            #endif
        }

        mBuffer = 0;
        mBuffer3D = 0;
        mBufferQuality = 0;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 398d61a5 auto
// OFFSET: 0x0094E070
FMOD_RESULT ChannelDSound::setPaused(bool paused)
{
    FMOD_RESULT result = FMOD_OK;

	if (!mBuffer || !mSound)
	{
		return FMOD_ERR_INVALID_PARAM;
	}

    if (!mOutputDSound)
    {
    	return FMOD_ERR_UNINITIALIZED;
    }

	if (paused)
	{
		mBuffer->Stop();

        return ChannelReal::setPaused(paused);
	}
	else
	{
		if (mFlags & CHANNELREAL_FLAG_PAUSED)
		{
			DWORD       locflags = 0;
            HRESULT     hr;
    
			if (!(mMode & FMOD_UNIQUE))
            {
                if (mLOCSoftware)
                {
	        		locflags |= DSBPLAY_LOCSOFTWARE;
				}
                else
                {
    			    locflags |= DSBPLAY_LOCHARDWARE;
                }
            }
			hr = mBuffer->Play(0,0,locflags | (mMode & (FMOD_LOOP_NORMAL | FMOD_LOOP_BIDI) && mLoopCount ? DSBPLAY_LOOPING : 0));
            if (hr != DS_OK)
            {
                switch (hr)
                {
                    case DSERR_BUFFERLOST:
                        hr = hr;
                        break;
                    case DSERR_INVALIDCALL:
                        hr = hr;
                        break;
                    case DSERR_INVALIDPARAM:
                        hr = hr;
                        break;
                    case DSERR_PRIOLEVELNEEDED:
                        hr = hr;
                        break;
                };

                if (hr == E_OUTOFMEMORY)
                {
                    result = FMOD_ERR_MEMORY;
                }
                else
                {
                    result = FMOD_ERR_OUTPUT_DRIVERCALL;
                }
            }
#ifdef FMOD_CHANNEL_DSOUND_QUALITY
            else if (mBufferQuality)
            {
                GUID guid;
                DWORD dwQuality = FMOD_CHANNEL_DSOUND_QUALITY;

                FMOD_memcpy(&guid, &IID_KSPROPSETID_Audio, sizeof(GUID));

                mBufferQuality->Set(guid, KSPROPERTY_AUDIO_QUALITY, PVOID(&dwQuality), sizeof(dwQuality), PVOID(&dwQuality), sizeof(dwQuality));
            }
#endif
		}
	}

    if (result != FMOD_OK)
    {
        return result;
    }

	return ChannelReal::setPaused(paused);	
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: a9005dc8 auto
// OFFSET: 0x0094E130
FMOD_RESULT ChannelDSound::setVolume(float volume)
{  
	if (mBuffer) 
	{
        HRESULT hr;

        if (mOutputDSound->mReverbVersion == REVERB_VERSION_NONE)
        {
			volume = mParent->mChannelGroup->mRealDirectOcclusionVolume * mParent->mDirectOcclusionVolume * volume;
		}

        volume *= mParent->mChannelGroup->mRealVolume;
        if (mMode & FMOD_3D)
        {
            if (mMode & (FMOD_3D_LINEARROLLOFF | FMOD_3D_CUSTOMROLLOFF) || mSystem->mRolloffCallback)
            {
                volume *= mParent->mVolume3D;
            }

            volume *= mParent->mConeVolume3D;
        }
        if (mSound && mSound->mSoundGroup)
        {
            volume *= mSound->mSoundGroup->mVolume;
        }

        if (volume == 1.0f)
        {
            volume = 0;
        }
        else if (volume == 0)
        {
            if (mSound && mSound->mFormat == FMOD_SOUND_FORMAT_PCMFLOAT)
            {
                volume = -8000;     /* DirectSound bug workaround.  Silence on a float directsound buffer apparently means a horrible buzzing noise.  This is as close to silence as we can get before it buzzes. */
            }
            else
            {
                volume = -10000;
            }
        }
        else
        {
            if (volume < 0.00001f)
            {
                volume = 0.00001f;
            }

            /* 
                Convert from linear amplitude to hundredths of decibels 
            */
            volume = 100.0f * 20.0f * FMOD_LOG10(volume);
        }

		hr = mBuffer->SetVolume((int)volume);
	}

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 056be2c7 manual
// OFFSET: 0x0094E230
FMOD_RESULT ChannelDSound::setFrequency(float frequency)
{
    if (mSound && !mOutputDSound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (mBuffer)
    {
        HRESULT hr;

        frequency *= mParent->mChannelGroup->mRealPitch;

		hr = mBuffer->SetFrequency((int)frequency);
        if (hr != DS_OK)
        {
            return FMOD_ERR_OUTPUT_DRIVERCALL;
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

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: f0c2d3a0 auto
// OFFSET: 0x0094E2B0
FMOD_RESULT ChannelDSound::setPan(float pan, float fbpan)
{
	int hwpan;

	if (mBuffer3D)
    {
    	return FMOD_ERR_FORMAT;
    }

	if (mBuffer) 
	{
        float gain;

        pan = (pan + 1.0f) / 2.0f;  // convert from -1 to 1, to 0 to 1.

		if (pan < 0.5f)
        {
            gain = pan * 2.0f;
            if (gain < 0.00001f)
            {
                gain = 0.00001f;
            }

            /* 
                Convert from linear amplitude to hundredths of decibels 
            */
            hwpan = (int)(100.0f * 20.0f * FMOD_LOG10(gain));
        }
		else
        {
            gain = (1.0f-pan) * 2.0f;
            if (gain < 0.00001f)
            {
                gain = 0.00001f;
            }

            /* 
                Convert from linear amplitude to hundredths of decibels 
            */
            hwpan = -(int)(100.0f * 20.0f * FMOD_LOG10(gain));
        }
		
		mBuffer->SetPan(hwpan);
	}

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: ccdc4143 manual
// OFFSET: 0x0094E910
FMOD_RESULT ChannelDSound::setPosition(unsigned int position, FMOD_TIMEUNIT postype)
{
    HRESULT         hr;
    unsigned int    pcmbytes;

    if (postype != FMOD_TIMEUNIT_MS && postype != FMOD_TIMEUNIT_PCM && postype != FMOD_TIMEUNIT_PCMBYTES)
    {
        return FMOD_ERR_FORMAT;
    }

	if (!mBuffer || !mSound)
	{
		return FMOD_ERR_INVALID_PARAM;
	}

    if (postype == FMOD_TIMEUNIT_PCM)
    {
        SoundI::getBytesFromSamples(position, &pcmbytes, mSound->mChannels, mSound->mFormat);
    }
    else if (postype == FMOD_TIMEUNIT_PCMBYTES)
    {
        pcmbytes = position;
    }
    else if (postype == FMOD_TIMEUNIT_MS)
    {
        pcmbytes = (unsigned int)((float)position * mSound->mDefaultFrequency / 1000.0f);
        SoundI::getBytesFromSamples(pcmbytes, &pcmbytes, mSound->mChannels, mSound->mFormat);
    }

	hr = mBuffer->SetCurrentPosition(pcmbytes);
    if (hr != DS_OK)
	{
        return FMOD_ERR_OUTPUT_DRIVERCALL;
	}
    
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: abd15884 manual
// OFFSET: 0x0094EC00
FMOD_RESULT ChannelDSound::getPosition(unsigned int *position, FMOD_TIMEUNIT postype)
{
	DWORD             dwPlay, dwWrite;
    HRESULT           hr;
    bool              playing;

    if (!position)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *position = 0;

    if (postype != FMOD_TIMEUNIT_MS && postype != FMOD_TIMEUNIT_PCM && postype != FMOD_TIMEUNIT_PCMBYTES)
    {
        return FMOD_ERR_FORMAT;
    }

	if (!mBuffer || !mSound)
	{
		return FMOD_ERR_INVALID_PARAM;
	}

    isPlaying(&playing);

    if (!playing || mFlags & CHANNELREAL_FLAG_ENDDELAY)
    {
        mSound->getLength(position, FMOD_TIMEUNIT_PCM);
        return FMOD_OK;
    }

    hr = mBuffer->GetCurrentPosition(&dwPlay, &dwWrite);
	if (hr != DS_OK)
	{
        *position = 0;
		return FMOD_ERR_OUTPUT_DRIVERCALL;
	}

    if (postype == FMOD_TIMEUNIT_PCM)
    {
        SoundI::getSamplesFromBytes(dwPlay, position, mSound->mChannels, mSound->mFormat);
    }
    else if (postype == FMOD_TIMEUNIT_PCMBYTES)
    {
        *position = dwPlay;
    }
    else if (postype == FMOD_TIMEUNIT_MS)
    {
        SoundI::getSamplesFromBytes(dwPlay, position, mSound->mChannels, mSound->mFormat);
        *position = (unsigned int)((float)*position / mSound->mDefaultFrequency * 1000.0f);
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: db0bf2ad manual
// OFFSET: 0x0094E370
FMOD_RESULT ChannelDSound::set3DAttributes()
{
    FMOD_RESULT   result;
    int           numlisteners;
	DS3DBUFFER    buffparams;
    int           deferred = DS3D_IMMEDIATE;

    if (!mBuffer3D)
    {
        return FMOD_OK;
    }

    if (!mOutputDSound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = mSystem->get3DNumListeners(&numlisteners);
    if (result != FMOD_OK)
    {
        return result;
    }

    FMOD_memset(&buffparams, 0, sizeof(DS3DBUFFER));
    buffparams.dwSize = sizeof(DS3DBUFFER);
    mBuffer3D->GetAllParameters(&buffparams);

    if (numlisteners == 1)
    {
        buffparams.vPosition.x = mParent->mPosition3D.x;
        buffparams.vPosition.y = mParent->mPosition3D.y;
        buffparams.vPosition.z = mParent->mPosition3D.z;

        if (mSystem->mFlags & FMOD_INIT_3D_RIGHTHANDED)
        {
            buffparams.vPosition.z = -buffparams.vPosition.z;
        }
    }
    else
    {
        buffparams.vPosition.x = 0;
        buffparams.vPosition.y = 0;      
        buffparams.vPosition.z = mParent->mDistance;
    }

    if (numlisteners == 1)
    {
        if (mParent->m3DDopplerLevel != 1.0f)
        {
            buffparams.vVelocity.x = (mParent->mVelocity3D.x * mParent->m3DDopplerLevel) + (mSystem->mListener[0].mVelocity.x * (1.0f - mParent->m3DDopplerLevel));
            buffparams.vVelocity.y = (mParent->mVelocity3D.y * mParent->m3DDopplerLevel) + (mSystem->mListener[0].mVelocity.y * (1.0f - mParent->m3DDopplerLevel));
            buffparams.vVelocity.z = (mParent->mVelocity3D.z * mParent->m3DDopplerLevel) + (mSystem->mListener[0].mVelocity.z * (1.0f - mParent->m3DDopplerLevel));
        }
        else
        {
            buffparams.vVelocity.x = mParent->mVelocity3D.x;
            buffparams.vVelocity.y = mParent->mVelocity3D.y;
            buffparams.vVelocity.z = mParent->mVelocity3D.z;
        }

        if (mSystem->mFlags & FMOD_INIT_3D_RIGHTHANDED)
        {
            buffparams.vVelocity.z = -buffparams.vVelocity.z;
        }
    }
    else
    {
        buffparams.vVelocity.x = 0;
        buffparams.vVelocity.y = 0;
        buffparams.vVelocity.z = 0;
    }

    if (mMode & (FMOD_3D_LINEARROLLOFF | FMOD_3D_CUSTOMROLLOFF) || mSystem->mRolloffCallback)
    {
        FMOD_VECTOR diff;
        float distance;
        FMOD_VECTOR pos, listenerpos;

        pos.x = mParent->mPosition3D.x;
        pos.y = mParent->mPosition3D.y;
        pos.z = mParent->mPosition3D.z;

        listenerpos = mSystem->mListener[0].mPosition;

        if (mSystem->mFlags & FMOD_INIT_3D_RIGHTHANDED)
        {
            pos.z = -pos.z;
            listenerpos.z = -listenerpos.z;
        }

    	buffparams.flMinDistance = 1.0f;
	    buffparams.flMaxDistance = 1.0f;
       
        FMOD_Vector_Subtract(&pos, &listenerpos, &diff);

        distance = FMOD_Vector_GetLengthFast(&diff);
        
        if (distance > 0.0f)
        {
            FMOD_Vector_Scale(&diff, (1.0f / distance) * mSystem->mDistanceScale, &diff);
        }
        else
        {
            FMOD_Vector_Set(&diff, 0, 0, 0);
        }

        FMOD_Vector_Add(&listenerpos, &diff, (FMOD_VECTOR *)(&buffparams.vPosition));

        setVolume(mParent->mVolume);
    }
    else
    {
    	buffparams.flMinDistance = mParent->mMinDistance;
	    buffparams.flMaxDistance = mParent->mMaxDistance;

        if (mParent->mConeOutsideAngle < 360.0f || mParent->mConeInsideAngle < 360.0f)    /* Because the first part of this if statement calls setvolume all the time, we'll just call it here if cones are used. */
        {
            setVolume(mParent->mVolume);
        }
    }

    // if (mOutputDSound->mInitFlags & FMOD_INIT_DSOUND_DEFERRED)
    {
        if (mFlags & (CHANNELREAL_FLAG_ALLOCATED | CHANNELREAL_FLAG_PAUSED) || mFlags & CHANNELREAL_FLAG_ENDDELAY)
        {
            deferred = DS3D_IMMEDIATE;
        }
        else
        {
            deferred = DS3D_DEFERRED;
            mOutputDSound->mNeedToCommit = true;
        }
    }

    if (mBuffer3D->SetAllParameters(&buffparams, deferred) != DS_OK)
    {
        return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: b40135ae manual
// OFFSET: 0x0094E670
FMOD_RESULT ChannelDSound::set3DMinMaxDistance()
{
    int  deferred = DS3D_IMMEDIATE;

    if (!mBuffer3D)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!mOutputDSound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }
    
    if (mMode & (FMOD_3D_LINEARROLLOFF | FMOD_3D_CUSTOMROLLOFF) || mSystem->mRolloffCallback)
    {        
        return FMOD_OK;
    }
    else
    {
        //if (mOutputDSound->mInitFlags & FMOD_INIT_DSOUND_DEFERRED)
        {
            if (mFlags & (CHANNELREAL_FLAG_ALLOCATED | CHANNELREAL_FLAG_PAUSED) || mFlags & CHANNELREAL_FLAG_ENDDELAY)
            {
                deferred = DS3D_IMMEDIATE;
            }
            else
            {
                deferred = DS3D_DEFERRED;
                mOutputDSound->mNeedToCommit = true;
            }
        }

        if (mBuffer3D->SetMinDistance(mParent->mMinDistance, deferred) != DS_OK)
        {
            return FMOD_ERR_OUTPUT_DRIVERCALL;
        }

        if (mBuffer3D->SetMaxDistance(mParent->mMaxDistance, deferred) != DS_OK)
        {
            return FMOD_ERR_OUTPUT_DRIVERCALL;
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

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: b428f22c auto
// OFFSET: 0x0094E720
FMOD_RESULT ChannelDSound::set3DOcclusion(float directOcclusion, float reverbOcclusion)
{
    if (mOutputDSound->mReverbVersion != REVERB_VERSION_NONE)
    {
        float directTransmission = (1.0f - directOcclusion) *
                                   mParent->mChannelGroup->mRealDirectOcclusionVolume;

        if (directTransmission < 0.00001f)
        {
            directTransmission = 0.00001f;
        }
       
        int directGain = int(100.0f * 20.0f * FMOD_LOG10(directTransmission));

        float reverbTransmission = (1.0f - reverbOcclusion) *
                                   mParent->mChannelGroup->mRealReverbOcclusionVolume;
        if (reverbTransmission < 0.00001f)
        {
            reverbTransmission = 0.00001f;
        }
        int reverbGain = int(100.0f * 20.0f * FMOD_LOG10(reverbTransmission));

        FMOD_REVERB_CHANNELPROPERTIES prop;	
        
        getReverbProperties(&prop);	

        prop.Direct   = directGain >> 2;    // quarter occlusion for low frequencies
        prop.DirectHF = directGain;         // full occlusion for high frequencies
        prop.Room     = reverbGain;
        
        return setReverbProperties(&prop);
    }
    else
    {
        return setVolume(mParent->mVolume);
    }
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 36df60ff manual
// OFFSET: 0x0094E800
FMOD_RESULT ChannelDSound::isPlaying(bool *isplaying, bool includethreadlatency)
{
    if (mFlags & (CHANNELREAL_FLAG_ALLOCATED | CHANNELREAL_FLAG_PAUSED) || mFlags & CHANNELREAL_FLAG_ENDDELAY)
    {
        if (isplaying)
        {
            *isplaying = true;
        }
        return FMOD_OK;
    }

    if (mBuffer)
    {
    	DWORD flags;

	    if (mBuffer->GetStatus(&flags) != DS_OK)
	    {
		    return FMOD_ERR_OUTPUT_DRIVERCALL;
	    }

    	*isplaying = (flags & DSBSTATUS_PLAYING) ? true : false;
    }
    else
    {
        *isplaying = false;
    }

    if (!*isplaying)
    {
        if (mDelayEnd)
        {
            mFlags |= CHANNELREAL_FLAG_ENDDELAY;
            *isplaying = true;
        }
        else
        {
            mFlags &= ~(CHANNELREAL_FLAG_ALLOCATED | CHANNELREAL_FLAG_PLAYING);
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

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: b617afb6 manual
// OFFSET: 0x0094E890
FMOD_RESULT ChannelDSound::setMode(FMOD_MODE mode)
{
    int deferred = DS3D_IMMEDIATE;
    FMOD_RESULT result;

    result = ChannelReal::setMode(mode);
    if (result != FMOD_OK)
    {
        return result;
    }

	if (mBuffer3D)
	{
        HRESULT       hr;
        DWORD         d3dmode;

        if (!mOutputDSound)
        {
    	    return FMOD_ERR_INVALID_PARAM;
        }

        /*
            Turn head-relative on/off
        */
        if (mMode & FMOD_3D_HEADRELATIVE)
        {
            d3dmode = DS3DMODE_HEADRELATIVE;
        }
        else 
        {
            d3dmode = DS3DMODE_NORMAL;
        }

        //if (mOutputDSound->mInitFlags & FMOD_INIT_DSOUND_DEFERRED)
        {
            if (mFlags & (CHANNELREAL_FLAG_ALLOCATED | CHANNELREAL_FLAG_PAUSED) || mFlags & CHANNELREAL_FLAG_ENDDELAY)
            {
                deferred = DS3D_IMMEDIATE;
            }
            else
            {
                deferred = DS3D_DEFERRED;
                mOutputDSound->mNeedToCommit = true;
            }
        }

        hr = mBuffer3D->SetMode(d3dmode, deferred);
        if (FAILED(hr))
        {
            return FMOD_ERR_OUTPUT_DRIVERCALL;
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

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelDSound::getBuffer3D(IDirectSound3DBuffer **buffer3d)
{
    if (!buffer3d)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *buffer3d = mBuffer3D;

    return FMOD_OK;
}


FMOD_RESULT ChannelDSound::setSpeakerLevels(int speaker, float *levels, int numlevels)
{
    return FMOD_ERR_NEEDSSOFTWARE;
}


FMOD_RESULT ChannelDSound::setSpeakerMix(float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright)
{
    return FMOD_ERR_NEEDSSOFTWARE;
}


}

#endif /* FMOD_SUPPORT_DSOUND */
