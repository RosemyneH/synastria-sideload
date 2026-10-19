#include "fmod_settings.h"

#include "fmod_channel_real.h"
#include "fmod_channel_software.h"
#include "fmod_channelpool.h"
#include "fmod_dspi.h"
#include "fmod_dsp_resampler_linear.h"
#include "fmod_localcriticalsection.h"
#include "fmod_memory.h"
#include "fmod_output_software.h"
#include "fmod_outputi.h"
#include "fmod_soundi.h"
#include "fmod_string.h"

#include "fmod_systemi.h"

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

	[SEE_ALSO]
]
*/
// VERIFY: 57a4c94c manual
// OFFSET: 0x00916EC0
Output::Output()
{
    mEnumerated             = false;
    mChannelPool            = 0;
    mChannelPool3D          = 0;
#ifdef FMOD_SUPPORT_RECORDING
    mRecordLoop             = false;
#endif
    mMusicChannelGroup      = 0;
    mNum2DChannelsFromCaps  = 0;
    mNum3DChannelsFromCaps  = 0;
    mTotalChannelsFromCaps  = 0;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/
// VERIFY: ac1cf3e7 auto
// OFFSET: 0x00916090
FMOD_RESULT Output::release()
{
    FMOD_Memory_Free(this);

    return FMOD_OK;
}


#ifdef FMOD_SUPPORT_RECORDING

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 32dd46d0 manual
// OFFSET: 0x00916120
FMOD_RESULT Output::recordRead(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    FMOD_RESULT          result        = FMOD_OK;
    char                *rptr1         = NULL;
    char                *rptr2         = NULL;
    float               *destptr       = outbuffer;
    unsigned int         rlen1         = 0;
    unsigned int         rlen2         = 0;
    unsigned int         roffsetbytes  = 0;
    unsigned int         rlenbytes     = 0;
    unsigned int         rblockalign   = 0;

    SoundI::getBytesFromSamples(mRecordLastCursorPos, &roffsetbytes, outchannels, mRecordFormat);
    SoundI::getBytesFromSamples(length,               &rlenbytes,    outchannels, mRecordFormat);
    SoundI::getBytesFromSamples(1,                    &rblockalign,  outchannels, mRecordFormat);

	result = mDescription.record_lock((FMOD_OUTPUT_STATE *)this, roffsetbytes, rlenbytes, (void **)&rptr1, (void **)&rptr2, &rlen1, &rlen2);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (mRecordFormat == FMOD_SOUND_FORMAT_PCM8)
    {
        unsigned char *rptr = NULL;

        if (rptr1 && rlen1)
        {
            rptr = (unsigned char *)rptr1;
            for (unsigned int count = 0; count < rlen1; count++)
            {
                *rptr++ ^= 128;
            }
        }
        if (rptr2 && rlen2)
        {
            rptr = (unsigned char *)rptr2;
            for (unsigned int count = 0; count < rlen2; count++)
            {
                *rptr++ ^= 128;
            }
        }
    }

    if (rptr1 && rlen1)
    {
        DSPI::convert(destptr, rptr1, FMOD_SOUND_FORMAT_PCMFLOAT, mRecordFormat, (rlen1 / rblockalign) * outchannels, 1, 1, 1.0f);
        destptr += ((rlen1 / rblockalign) * outchannels);
    }
    if (rptr2 && rlen2)
    {
        DSPI::convert(destptr, rptr2, FMOD_SOUND_FORMAT_PCMFLOAT, mRecordFormat, (rlen2 / rblockalign) * outchannels, 1, 1, 1.0f);
    }

    if (mDescription.record_unlock)
    {
	    result = mDescription.record_unlock((FMOD_OUTPUT_STATE *)this, rptr1, rptr2, rlen1, rlen2);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    mRecordLastCursorPos += length;
    if (mRecordLastCursorPos >= mRecordBufferLength)
    {
        mRecordLastCursorPos -= mRecordBufferLength;
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
// VERIFY: 2c9aa64b auto
// OFFSET: 0x009165A0
FMOD_RESULT F_CALLBACK Output::recordResamplerReadCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPResampler  *resampler  = (DSPResampler *)dsp_state;
    Output        *output     = NULL;

    resampler->getUserData((void **)&output);

    return output->recordRead(inbuffer, outbuffer, length, inchannels, outchannels);
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
// VERIFY: 83b217e1 manual
// OFFSET: 0x009165F0
FMOD_RESULT Output::recordFill(unsigned int length)
{
    FMOD_RESULT          result;
    int                  channels;
    char                *pptr1 = 0, *pptr2 = 0;
    unsigned int         plen1 = 0, plen2 = 0, poffsetbytes, soundlength, plength, plengthbytes, pblockalign, totalplength;
    FMOD_SOUND_FORMAT    pformat;

    result = mRecordSound->getFormat(0, &pformat, &channels, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mRecordSound->getLength(&soundlength, FMOD_TIMEUNIT_PCM);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = SoundI::getBytesFromSamples(1, &pblockalign, channels, pformat);
    if (result != FMOD_OK)
    {
        return result;
    }

    totalplength = (unsigned int)((length * mRecordSound->mDefaultFrequency) / mRecordRate);

    while (totalplength)
    {
        plength = totalplength;
        if (plength > mRecordTempBufferLength)
        {
            plength = mRecordTempBufferLength;
        }

        SoundI::getBytesFromSamples(mRecordOffset, &poffsetbytes, channels, pformat);
        SoundI::getBytesFromSamples(plength,       &plengthbytes, channels, pformat);

        if (mRecordResample)
        {
            result = mRecordResamplerDSP.execute(mRecordTempBuffer, &mRecordTempBuffer, &plength, channels, &channels, (FMOD_SPEAKERMODE)0);
        }
        else
        {
            result = recordRead(mRecordTempBuffer, mRecordTempBuffer, plength, channels, channels);
        }
        if (result != FMOD_OK)
        {
            return result;
        }

        result = mRecordSound->lock(poffsetbytes, plengthbytes, (void **)&pptr1, (void **)&pptr2, &plen1, &plen2);
        if (result != FMOD_OK)
        {
            return result;
        }

        {
            float *srcptr = mRecordTempBuffer;

            if (pptr1 && plen1)
            {
                DSPI::convert(pptr1, srcptr, pformat, FMOD_SOUND_FORMAT_PCMFLOAT, (plen1 / pblockalign) * channels, 1, 1, 1.0f);
                srcptr += ((plen1 / pblockalign) * channels);
            }
            if (pptr2 && plen2)
            {
                DSPI::convert(pptr2, srcptr, pformat, FMOD_SOUND_FORMAT_PCMFLOAT, (plen2 / pblockalign) * channels, 1, 1, 1.0f);
            }
        }

        result = mRecordSound->unlock(pptr1, pptr2, plen1, plen2);
        if (result != FMOD_OK)
        {
            return result;
        }

        mRecordOffset += plength;
        if (plen2 || mRecordOffset >= (int)soundlength)
        {
            if (!mRecordLoop)
            {
                mSystem->recordStop();
                return FMOD_OK;
            }

	        mRecordOffset -= soundlength;
            if (mRecordOffset < 0)
            {
                mRecordOffset = 0;
            }
        }

        totalplength -= plength;
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

	[SEE_ALSO]
]
*/
// VERIFY: 64f26d7e auto
// OFFSET: 0x00916B30
FMOD_RESULT Output::recordUpdate()
{
    FMOD_RESULT  result;
    unsigned int cursorpos = 0;
    int          length;

    if (!mDescription.record_getposition)
    {
        return FMOD_OK;
    }

    if (!mRecordBufferLength)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

	result = mDescription.record_getposition((FMOD_OUTPUT_STATE *)this, &cursorpos);
    if (result != FMOD_OK)
    {
        return result;
    }

	length = cursorpos - mRecordLastCursorPos;
	if (length < 0)
    {
		length += mRecordBufferLength;
    }
    if (length < 0 || length > (int)mRecordBufferLength)
    {
        length = 0;
    }

    if (mRecordResample)
    {
        int blocklength = mRecordResamplerDSP.mDescription.mResamplerBlockLength;

        if (length < blocklength * 3)
        {
            return FMOD_OK;
        }

        length /= blocklength;
        length -= 1;
        length *= blocklength;
    }
    else
    {
        if (length <= 0)
        {
            return FMOD_OK;
        }
    }

    return recordFill(length);
}


#endif  /* FMOD_SUPPORT_RECORDING */

#ifdef FMOD_SUPPORT_SOFTWARE
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/
// VERIFY: 3ccd6609 manual
// OFFSET: 0x00916BC0
FMOD_RESULT Output::mix(void *buffer, unsigned int numsamples)
{
    FMOD_RESULT           result;
    unsigned int          blockalign = 0;
    unsigned int          offset = 0;
    LocalCriticalSection  criticalsection(mSystem->mDSPCrit);
    LocalCriticalSection  lockdspcriticalsection(mSystem->mDSPLockCrit);
    FMOD_SOUND_FORMAT     outputformat;
    int                   outputchannels;
    DSPI                 *dsphead;

    if (!buffer || !numsamples)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    mSystem->getSoftwareFormat(0, &outputformat, &outputchannels, 0, 0, 0);

    result = SoundI::getBytesFromSamples(1, &blockalign, outputchannels, outputformat);
    if (result != FMOD_OK)
    {
        return result;
    }

    dsphead = mSystem->mDSPSoundCard;
    if (!dsphead)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    mSystem->flushDSPConnectionRequests(false);

    lockdspcriticalsection.enter();
    criticalsection.enter();

#ifdef FMOD_SUPPORT_RECORDING
    if (mRecording)
    {
        recordUpdate();
    }
#endif

    do
    {
        void *outbuffer = (char *)buffer + (offset * blockalign);
        void *mixbuffer = outbuffer;
        unsigned int len = numsamples;

        mSystem->mDSPActive = true;
        mSystem->mDSPTempBuffIndex = 0;

        dsphead->execute(outbuffer, &mixbuffer, &len, outputchannels, &outputchannels, mSystem->mSpeakerMode);

        mSystem->mDSPActive = false;

        if (mixbuffer != outbuffer)
        {
            FMOD_memcpy(outbuffer, mixbuffer, blockalign * len);
        }

        dsphead->resetVisited();

        offset += len;
        numsamples -= len;
    }
    while (numsamples);

	if (mDescription.postmixcallback)
	{
		mDescription.postmixcallback(this);
	}

    criticalsection.leave();
    lockdspcriticalsection.leave();

    {
        float delta = (float)mSystem->mDSPBlockSize / (float)mSystem->mOutputRate * 1000.0f;    // ms

	    FMOD::gGlobal->gDSPClock.mValue += (FMOD_UINT64)(delta * 4294967296.0f);
        FMOD_OS_Time_GetMs(&FMOD::gGlobal->gDSPClockTimeStamp);
    }

    return FMOD_OK;
}
#endif

#ifndef PLATFORM_PS3_SPU
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/
// VERIFY: 9606ec7c manual
// OFFSET: 0x009160B0
FMOD_RESULT Output::getFreeChannel(FMOD_MODE mode, ChannelReal **realchannel, int numchannels, int numsoundchannels, int *found)
{
    FMOD_RESULT result;

    if (!realchannel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (mode & FMOD_3D)
    {
        if (!mChannelPool3D)
        {
            return FMOD_ERR_CHANNEL_ALLOC;
        }

        result = mChannelPool3D->allocateChannel(realchannel, FMOD_CHANNEL_FREE, numchannels, found);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    else
    {
        if (!mChannelPool)
        {
            return FMOD_ERR_CHANNEL_ALLOC;
        }

        result = mChannelPool->allocateChannel(realchannel, FMOD_CHANNEL_FREE, numchannels, found);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    return FMOD_OK;
}
#endif

#ifdef FMOD_SUPPORT_SOFTWARE
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/
// VERIFY: 7db20d74 manual
// OFFSET: 0x00916E80
FMOD_RESULT F_CALLBACK Output::mixCallback(FMOD_OUTPUT_STATE *output, void *buffer, unsigned int length)
{
    Output *out = (Output *)output;

    return out->mix(buffer, length);
}
#endif


/*
[API]
[
	[DESCRIPTION]
    Called when the user calls System::getNumDrivers.

	[PARAMETERS]
    'output_state'  Pointer to the plugin state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
    'numdrivers'    Address of a variable to receive the number of output drivers in your plugin.

	[RETURN_VALUE]

	[REMARKS]
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.<br>
    Optional.  FMOD will assume 0 if this is not specified.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System::getNumDrivers
    System::getDriverInfo
    FMOD_OUTPUT_GETDRIVERNAMECALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_OUTPUT_GETNUMDRIVERSCALLBACK(FMOD_OUTPUT_STATE *output_state, int *numdrivers)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/

/*
[API]
[
	[DESCRIPTION]
    Called when the user calls System::getDriverInfo.

	[PARAMETERS]
    'output_state'  Pointer to the plugin state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
    'id'            Index into the total number of outputs possible, provided by the FMOD_OUTPUT_GETNUMDRIVERSCALLBACK callback.
    'name'          Address of a variable to receive the driver name relevant to the index passed in.  Fill this in.
    'namelen'       Length of name buffer being passed in by the user.

	[RETURN_VALUE]

	[REMARKS]
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System::getDriverInfo
    System::getNumDrivers
    FMOD_OUTPUT_GETNUMDRIVERSCALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_OUTPUT_GETDRIVERNAMECALLBACK(FMOD_OUTPUT_STATE *output_state, int id, char *name, int namelen)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Called when the user calls System::getDriverCaps.

	[PARAMETERS]
    'output_state'  Pointer to the plugin state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
    'id'            Index into the total number of outputs possible, provided by the FMOD_OUTPUT_GETNUMDRIVERSCALLBACK callback.
    'caps'          Address of a variable to receive the caps available by this output device.  See FMOD_CAPS.  Fill this in.

	[RETURN_VALUE]

	[REMARKS]
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System::getDriverCaps
    System::getDriverInfo
    System::getNumDrivers
    FMOD_OUTPUT_GETNUMDRIVERSCALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_OUTPUT_GETDRIVERCAPSCALLBACK(FMOD_OUTPUT_STATE *output_state, int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Initialization callback which is called when the user calls System::init.

	[PARAMETERS]
    'output_state'      Pointer to the plugin state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
    'selecteddriver'    This is the selected driver id that the user chose from calling System::setDriver.
    'flags'             Initialization flags passed in by the user.
    'outputrate'        Output rate selected by the user.  If not possible, change the rate to the closest match.
    'outputchannels'    Output channel count selected by the user.  For example 1 = mono output.  2 = stereo output.
    'outputformat'      Output format specified by the user.  If not possible to support, return FMOD_ERR_FORMAT.
    'dspbufferlength'   Size of the buffer fmod will mix to in one mix update.  This value is in PCM samples.
    'dspnumbuffers'     Number of buffers fmod will mix to in a circular fashion.  Multiply this by dspbufferlength to get the total size of the output sound buffer to allocate.
    'extradriverdata'   Data passed in by the user specific to this driver.  May be used for any purpose.

	[RETURN_VALUE]

	[REMARKS]
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    FMOD_RESULT
    System::init
    System::setDriver
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_OUTPUT_INITCALLBACK(FMOD_OUTPUT_STATE *output_state, int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Shut down callback which is called when the user calls System::close or System::release.  (System::release calls System::close internally)

	[PARAMETERS]
    'output_state'        Pointer to the plugin state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.

	[RETURN_VALUE]

	[REMARKS]
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System::release
    System::close 
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_OUTPUT_CLOSECALLBACK(FMOD_OUTPUT_STATE *output_state)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Called when the user calls System::update.

	[PARAMETERS]
    'output_state'        Pointer to the plugin state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.

	[RETURN_VALUE]

	[REMARKS]
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_OUTPUT_UPDATECALLBACK(FMOD_OUTPUT_STATE *output_state)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Called when the user calls System::getOutputHandle.

	[PARAMETERS]
    'output_state'        Pointer to the plugin state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
    'handle'        Address of a variable to receieve the current plugin's output 'handle'.  This is only if the plugin writer wants to allow the user access to the main handle behind the plugin (for example the file handle in a file writer plugin).  The pointer type must be published to the user somehow, as is done in fmod.h.

	[RETURN_VALUE]

	[REMARKS]
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_OUTPUT_GETHANDLECALLBACK(FMOD_OUTPUT_STATE *output_state, void **handle)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Returns the current PCM offset or playback position for the output stream.<br>
    Called from the mixer thread, only when the 'polling' member of FMOD_OUTPUT_DESCRIPTION is set to <b>true</b>.<br>
    The internal FMOD output thread calls this function periodically to determine if it should ask for a block of audio data or not.

	[PARAMETERS]
    'output_state'  Pointer to the plugin state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
    'position'      Address of a variable that receives the position of the output stream.

	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    FMOD_OUTPUT_DESCRIPTION
    FMOD_OUTPUT_LOCKCALLBACK
    FMOD_OUTPUT_UNLOCKCALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_OUTPUT_GETPOSITIONCALLBACK(FMOD_OUTPUT_STATE *output_state, unsigned int *pcm)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Called from the mixer thread, only when the 'polling' member of FMOD_OUTPUT_DESCRIPTION is set to true.

	[PARAMETERS]
    'output_state'  Pointer to the plugin state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
	'offset'	    Offset in <i>bytes</i> to the position the caller wants to lock in the sample buffer.
	'length'	    Number of <i>bytes</i> the caller want to lock in the sample buffer.
	'ptr1'		    Address of a pointer that will point to the first part of the locked data.
	'ptr2'		    Address of a pointer that will point to the second part of the locked data.  This will be null if the data locked hasn't wrapped at the end of the buffer.
	'len1'		    Length of data in <i>bytes</i> that was locked for ptr1
	'len2'		    Length of data in <i>bytes</i> that was locked for ptr2.  This will be 0 if the data locked hasn't wrapped at the end of the buffer.

	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    FMOD_OUTPUT_DESCRIPTION
    FMOD_OUTPUT_UNLOCKCALLBACK
    FMOD_OUTPUT_GETPOSITIONCALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_OUTPUT_LOCKCALLBACK(FMOD_OUTPUT_STATE *output_state, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Called from the mixer thread, only when the 'polling' member of FMOD_OUTPUT_DESCRIPTION is set to true.

	[PARAMETERS]
    'output_state'  Pointer to the plugin state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
	'ptr1'		    Pointer to the 1st locked portion of sample data, from Sound::lock.
	'ptr2'		    Pointer to the 2nd locked portion of sample data, from Sound::lock.
	'len1'		    Length of data in <i>bytes</i> that was locked for ptr1
	'len2'		    Length of data in <i>bytes</i> that was locked for ptr2.  This will be 0 if the data locked hasn't wrapped at the end of the buffer.

	[RETURN_VALUE]

	[REMARKS]
    This function is normally called after data has been read/written to from Sound::lock.  This function will do any post processing nescessary and if needed, send it to sound ram.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    FMOD_OUTPUT_DESCRIPTION
    FMOD_OUTPUT_LOCKCALLBACK
    FMOD_OUTPUT_GETPOSITIONCALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_OUTPUT_UNLOCKCALLBACK(FMOD_OUTPUT_STATE *output_state, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Called by the plugin, when the 'polling' member of FMOD_OUTPUT_DESCRIPTION is set to false.<br>
    Use this function from your own driver irq/timer to read some data from FMOD's DSP engine.  All of the resulting output caused by playing sounds and specifying effects by the user will be mixed here and written to the memory provided by the plugin writer.<br>

	[PARAMETERS]
    'output_state'  Pointer to the plugin state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
    'buffer'        Plugin-writer provided memory for the FMOD Ex mixer to write to.
    'length'        Length of the buffer in samples.

	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_OUTPUT_READFROMMIXER(FMOD_OUTPUT_STATE *output_state, void *buffer, unsigned int length)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/


}
