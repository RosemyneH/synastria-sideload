#include "fmod_settings.h"

#include "fmod_3d.h"
#include "fmod_channel_real.h"
#include "fmod_channeli.h"
#include "fmod_channelpool.h"
#include "fmod_output.h"
#include "fmod_systemi.h"
#include "fmod_soundi.h"
#include "fmod_speakermap.h"

#include <string.h>

namespace FMOD
{


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 38bec188 auto
// OFFSET: 0x00943900
ChannelReal::ChannelReal()
{
    int count;

    for (count = 0; count < FMOD_CHANNEL_MAXREALSUBCHANNELS; count++)
    {
        mRealChannel[count] = 0;
    }

    mMinFrequency       = 100.0f;
    mSound              = 0;
    mMaxFrequency       = 1000000.0f;
    mSystem             = 0;
    mOutput             = 0;
    mPool               = 0;
    mLoopCount          = -1;
    mNumRealChannels    = 1;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: e9d163c6 auto
// OFFSET: 0x00943970
FMOD_RESULT ChannelReal::init(int index, SystemI *system, Output *output, DSPI *dspmixtarget)
{
    mSound              = 0;
    mFlags              = (CHANNELREAL_FLAG)0;
    mMode               = (FMOD_MODE)0;
    mPosition           = 0;
    mDirection          = CHANNELREAL_PLAYDIR_FORWARDS;
    mDelayStart         = 0;
    mDelayEnd           = 0;
    mOutput             = output;
    mIndex              = index;
    mLoopCount          = -1;
    mSystem             = system;
    mNumRealChannels    = 1;

    return FMOD_OK;
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: f6743d6c auto
// OFFSET: 0x009439C0
FMOD_RESULT ChannelReal::close()
{
    return stop(true, true);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: a863b723 auto
// OFFSET: 0x009439D0
FMOD_RESULT ChannelReal::alloc()
{
    mPosition = 0;

    if (mPool)
    {
        mPool->mChannelsUsed++;
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
    
	[SEE_ALSO]
]
*/
// VERIFY: 1e1959cf manual
// OFFSET: 0x009439F0
FMOD_RESULT ChannelReal::alloc(DSPI *dsp)
{
    if (mPool)
    {      
        mPool->mChannelsUsed++;
    }
    
    mPosition = 0;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelReal::set2DFreqVolumePanFor3D()
{
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelReal::moveChannelGroup(ChannelGroupI *oldchannelgroup, ChannelGroupI *newchannelgroup)
{
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelReal::update(int delta)
{
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelReal::updateStream()
{  
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelReal::start()
{
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 3c5da353 auto
// OFFSET: 0x0091E5A0
FMOD_RESULT ChannelReal::stop(bool ignoreenddelay, bool resetstate, bool force)
{
    return stop(ignoreenddelay, resetstate);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]

	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/
// VERIFY: 211cd794 manual
// OFFSET: 0x00943A10
FMOD_RESULT ChannelReal::stop(bool ignoreenddelay, bool resetstate)
{
    if (!ignoreenddelay && mDelayEnd)
    {
        mFlags |= CHANNELREAL_FLAG_ENDDELAY;
        return FMOD_OK;
    }

    if (mPool)
    {
        mPool->mChannelsUsed--;
    }

    if (resetstate)
    {
        mFlags = (CHANNELREAL_FLAG)(mFlags & ~(CHANNELREAL_FLAG_ALLOCATED | CHANNELREAL_FLAG_PAUSED | CHANNELREAL_FLAG_PLAYING | CHANNELREAL_FLAG_PAUSEDFORSETPOS | CHANNELREAL_FLAG_ENDDELAY));
        mFlags = (CHANNELREAL_FLAG)(mFlags | CHANNELREAL_FLAG_STOPPED);
    }

    mDelayStart = 0;
    mDelayEnd   = 0;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 8ebeb7ca auto
// OFFSET: 0x00943A70
FMOD_RESULT ChannelReal::setPaused(bool paused)
{
    if (paused)
    {
        mFlags = (CHANNELREAL_FLAG)(mFlags | CHANNELREAL_FLAG_PAUSED);
    }
    else
    {
        mFlags = (CHANNELREAL_FLAG)(mFlags & ~CHANNELREAL_FLAG_PAUSED);
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
    
	[SEE_ALSO]
]
*/
// VERIFY: 27f73680 auto
// OFFSET: 0x00943A90
FMOD_RESULT ChannelReal::getPaused(bool *paused)
{
    if (!paused)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *paused = mFlags & CHANNELREAL_FLAG_PAUSED ? true : false;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelReal::setVolume(float volume)
{
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelReal::setFrequency(float frequency)
{
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelReal::setPan(float pan, float fbpan)
{
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: e312c22e auto
// OFFSET: 0x00943AC0
FMOD_RESULT ChannelReal::setDelay(unsigned int startdelay, unsigned int enddelay)
{
    mDelayStart = startdelay;
    mDelayEnd   = enddelay;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: ee64c30e auto
// OFFSET: 0x00943AE0
FMOD_RESULT ChannelReal::setSpeakerMix(float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright)
{
    int numsubchannels = 0;

    if (mSound)
    {
        if (mSound->mSubSampleParent)
        {
            numsubchannels = mSound->mSubSampleParent->mChannels;
        }
        else
        {
            numsubchannels = mSound->mChannels;
        }
    }
    else if (mDSP)
    {
        numsubchannels = mDSP->mDescription.channels;
    }

    if (numsubchannels > 1)
    {
        switch (mSubChannelIndex)
        {
            case 0:
            {
                setPan(-1.0f, 1.0f);
                setVolume(frontleft);
                break;
            }
            case 1:
            {
                setPan(1.0f, 1.0f);
                setVolume(frontright);
                break;
            }
            case 2:
            {
                setPan(-1.0f, 1.0f);
                setVolume(center);
                break;
            }
            case 3:
            {
                setPan(1.0f, 1.0f);
                setVolume(lfe);
                break;
            }
            case 4:
            {
                setPan(-1.0f, 1.0f);
                setVolume(backleft);
                break;
            }
            case 5:
            {
                setPan(1.0f, 1.0f);
                setVolume(backright);
                break;
            }
            case 6:
            {
                setPan(-1.0f, 1.0f);
                setVolume(sideleft);
                break;
            }
            case 7:
            {
                setPan(1.0f, 1.0f);
                setVolume(sideright);
                break;
            }
        }
    }
    else
    {
        float vol = 0, lrpan = 0, fbpan = 0;

        vol += frontleft;
        vol += frontright;
        vol += center;
        vol += lfe;
        vol += backleft;
        vol += backright;
        vol += sideleft;
        vol += sideright;

        lrpan -= frontleft;
        lrpan += frontright;
        lrpan -= backleft;
        lrpan += backright;
        lrpan -= sideleft;
        lrpan += sideright;

        fbpan += frontleft;
        fbpan += frontright;
        fbpan += center;
        fbpan -= backleft;
        fbpan -= backright;

        vol = vol > 1.0f ? 1.0f : vol;

        setVolume(vol * mParent->mVolume);
        setPan(lrpan < -1.0f ? -1.0f : lrpan > 1.0f ? 1.0f : lrpan,
               fbpan < -1.0f ? -1.0f : fbpan > 1.0f ? 1.0f : fbpan);
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
    
	[SEE_ALSO]
]
*/
// VERIFY: 6bb8e636 auto
// OFFSET: 0x00943DA0
FMOD_RESULT ChannelReal::setSpeakerLevels(int speaker, float *levels, int numlevels)
{
    if (!mParent)
    {
        return FMOD_OK;
    }

    return updateSpeakerLevels(mParent->mVolume);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: b8bc3f80 manual
// OFFSET: 0x00943DC0
FMOD_RESULT ChannelReal::updateSpeakerLevels(float volume)
{
    if (mParent && mParent->mLevels)
    {
        int count;
        float vol = 0, pan = 0;

        for (count = 0; count < mSystem->mMaxOutputChannels; count++)
        {
            float val = FMOD_FABS(mParent->mLevels[(count * mSystem->mMaxInputChannels) + mSubChannelIndex]);

            vol += val;

            if (count == FMOD_SPEAKER_FRONT_LEFT || count == FMOD_SPEAKER_BACK_LEFT || count == FMOD_SPEAKER_SIDE_LEFT)
            {
                pan -= val;
            }
            else if (count == FMOD_SPEAKER_FRONT_RIGHT || count == FMOD_SPEAKER_BACK_RIGHT || count == FMOD_SPEAKER_SIDE_RIGHT)
            {
                pan += val;
            }
        }

        vol = vol > 1.0f ? 1.0f : vol;

        setVolume(vol * volume);
        setPan(pan < -1.0f ? -1.0f : pan > 1.0f ? 1.0f : pan);
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
    
	[SEE_ALSO]
]
*/
// VERIFY: e4380c90 manual
// OFFSET: 0x009442D0
FMOD_RESULT ChannelReal::setPosition(unsigned int position, FMOD_TIMEUNIT postype)
{
    if (postype != FMOD_TIMEUNIT_MS && postype != FMOD_TIMEUNIT_PCM && postype != FMOD_TIMEUNIT_PCMBYTES)
    {
        return FMOD_ERR_FORMAT;
    }
        
    if (mSound)
    {
        FMOD_RESULT     result;
        unsigned int    pcm = 0, lengthpcm;

        result = mSound->getLength(&lengthpcm, FMOD_TIMEUNIT_PCM);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (postype == FMOD_TIMEUNIT_PCM)
        {
            pcm = position;
        }
        else if (postype == FMOD_TIMEUNIT_PCMBYTES)
        {
            SoundI::getSamplesFromBytes(position, &pcm, mSound->mChannels, mSound->mFormat);
        }
        else if (postype == FMOD_TIMEUNIT_MS)
        {
            pcm = (unsigned int)((float)position * mSound->mDefaultFrequency * 0.001f);
        }
        
        if (pcm >= lengthpcm)
        {
            pcm = lengthpcm;
        }

        mPosition = pcm;
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
    
	[SEE_ALSO]
]
*/
// VERIFY: cb1c617d manual
// OFFSET: 0x009444A0
FMOD_RESULT ChannelReal::getPosition(unsigned int *position, FMOD_TIMEUNIT postype)
{   
    bool getsubsoundtime = false;
    int currentsubsoundid = 0;
    int currentsentenceid = 0;
    unsigned int pcmcurrent;

    if (!position)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!mSound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    postype = (FMOD_TIMEUNIT)(postype & ~FMOD_TIMEUNIT_BUFFERED); 

    if (postype == FMOD_TIMEUNIT_SENTENCE_MS)
    {
        postype = FMOD_TIMEUNIT_MS;
        getsubsoundtime = true;
    }
    else if (postype == FMOD_TIMEUNIT_SENTENCE_PCM)
    {
        postype = FMOD_TIMEUNIT_PCM;
        getsubsoundtime = true;
    }
    else if (postype == FMOD_TIMEUNIT_SENTENCE_PCMBYTES)
    {
        postype = FMOD_TIMEUNIT_PCMBYTES;
        getsubsoundtime = true;
    }
    else if (postype == FMOD_TIMEUNIT_SENTENCE || postype == FMOD_TIMEUNIT_SENTENCE_SUBSOUND)
    {
        getsubsoundtime = true;
    }

    if (getsubsoundtime && !mSound->mSubSoundList)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (postype != FMOD_TIMEUNIT_MS && postype != FMOD_TIMEUNIT_PCM && postype != FMOD_TIMEUNIT_PCMBYTES && postype != FMOD_TIMEUNIT_SENTENCE && postype != FMOD_TIMEUNIT_SENTENCE_SUBSOUND)
    {
        return FMOD_ERR_FORMAT;
    }

    pcmcurrent = mPosition;

    if (getsubsoundtime)
    {
        for (currentsubsoundid = 0; currentsubsoundid < mSound->mSubSoundListNum; currentsubsoundid++)
        {
            SoundI *sound = mSound->mSubSound[mSound->mSubSoundList[currentsubsoundid]];

            if (pcmcurrent < sound->mLength)
            {
                break;
            }

            currentsentenceid++;
            pcmcurrent -= sound->mLength;
        }
    }

    if (postype == FMOD_TIMEUNIT_SENTENCE)
    {
        *position = currentsentenceid;
    }
    else if (postype == FMOD_TIMEUNIT_SENTENCE_SUBSOUND)
    {
        *position = currentsubsoundid;
    }
    else if (postype == FMOD_TIMEUNIT_PCM)
    {
        *position = pcmcurrent;
    }
    else if (postype == FMOD_TIMEUNIT_PCMBYTES)
    {
        SoundI::getBytesFromSamples(pcmcurrent, position, mSound->mChannels, mSound->mFormat);
    }
    else
    {
        *position = (unsigned int)((float)pcmcurrent / mSound->mDefaultFrequency * 1000.0f);
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
    
	[SEE_ALSO]
]
*/
// VERIFY: b58cbc88 manual
// OFFSET: 0x00943FF0
FMOD_RESULT ChannelReal::setLoopPoints(unsigned int loopstart, unsigned int looplength)
{
    if (!mSound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (loopstart >= mSound->mLength)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (loopstart + looplength > mSound->mLength)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    mLoopLength = looplength;
    mLoopStart  = loopstart;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 2a387531 auto
// OFFSET: 0x00944040
FMOD_RESULT ChannelReal::setLoopCount(int loopcount)
{
    mLoopCount = loopcount;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelReal::set3DAttributes()
{
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelReal::set3DMinMaxDistance()
{
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 3b1a5c67 auto
// OFFSET: 0x00944050
FMOD_RESULT ChannelReal::set3DOcclusion(float directOcclusion, float reverbOcclusion)
{
    if (!mParent)
    {
        return FMOD_OK;
    }

    return setVolume(mParent->mVolume);
}



/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: dadd5551 auto
// OFFSET: 0x00944070
FMOD_RESULT ChannelReal::setReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!mParent)
    {
        return FMOD_OK;
    }

    return mSystem->mReverbGlobal.setChanProperties(mParent->mIndex, prop, 0);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: b7cdb91d auto
// OFFSET: 0x009440B0
FMOD_RESULT ChannelReal::getReverbProperties(FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!mParent)
    {
        return FMOD_OK;
    }

    return mSystem->mReverbGlobal.getChanProperties(mParent->mIndex, prop, 0);
}
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 9b419852 manual
// OFFSET: 0x009440F0
FMOD_RESULT ChannelReal::isPlaying(bool *isplaying, bool includethreadlatency)
{
    if (!isplaying)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (mFlags & (CHANNELREAL_FLAG_ALLOCATED | CHANNELREAL_FLAG_PLAYING) || mFlags & CHANNELREAL_FLAG_ENDDELAY)
    {
        *isplaying = true;
    }
    else
    {
        *isplaying = false;
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
    
	[SEE_ALSO]
]
*/
// VERIFY: 67e9f329 auto
// OFFSET: 0x00944130
FMOD_RESULT ChannelReal::isVirtual(bool *isvirtual)
{
    if (!isvirtual)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *isvirtual = false;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 08d4dc51 auto
// OFFSET: 0x00944150
FMOD_RESULT ChannelReal::getSpectrum(float *spectrumarray, int numentries, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype)
{
    return FMOD_ERR_NEEDSSOFTWARE;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 26337a2b auto
// OFFSET: 0x00944160
FMOD_RESULT ChannelReal::getWaveData(float *wavearray, int numvalues, int channeloffset)
{
    return FMOD_ERR_NEEDSSOFTWARE;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: c0fa54ea auto
// OFFSET: 0x00944170
FMOD_RESULT ChannelReal::getDSPHead(DSPI **dsp)
{   
    *dsp = 0;
    return FMOD_ERR_DSP_NOTFOUND;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 16afbd51 auto
// OFFSET: 0x00944190
FMOD_RESULT ChannelReal::setMode(FMOD_MODE mode)
{
    /*
        Allow switching between loop modes.
    */
    if (mode & (FMOD_LOOP_OFF | FMOD_LOOP_NORMAL | FMOD_LOOP_BIDI))
    {
        mMode = mMode & ~(FMOD_LOOP_OFF | FMOD_LOOP_NORMAL | FMOD_LOOP_BIDI);

        if (mode & FMOD_LOOP_OFF)
        {
            mMode |= FMOD_LOOP_OFF;
        }
        else if (mode & FMOD_LOOP_NORMAL)
        {
            mMode |= FMOD_LOOP_NORMAL;
        }
        else if (mode & FMOD_LOOP_BIDI)
        {
            mMode |= FMOD_LOOP_BIDI;
        }
    }

    /*
        Allow switching between head relative and world relative.
    */
    if (mode & FMOD_3D_HEADRELATIVE)
    {
        mMode &= ~FMOD_3D_WORLDRELATIVE;
        mMode |=  FMOD_3D_HEADRELATIVE;
    }
    else if (mode & FMOD_3D_WORLDRELATIVE)
    {
        mMode &= ~FMOD_3D_HEADRELATIVE;
        mMode |=  FMOD_3D_WORLDRELATIVE;
    }

    /*
        Allow switching between linear/log/custom rolloff.
    */
    if (mode & FMOD_3D_LOGROLLOFF)
    {
        mMode &= ~FMOD_3D_LINEARROLLOFF;
        mMode &= ~FMOD_3D_CUSTOMROLLOFF;
        mMode |=  FMOD_3D_LOGROLLOFF;
    }
    else if (mode & FMOD_3D_LINEARROLLOFF)
    {
        mMode &= ~FMOD_3D_LOGROLLOFF;
        mMode &= ~FMOD_3D_CUSTOMROLLOFF;
        mMode |=  FMOD_3D_LINEARROLLOFF;
    }
    else if (mode & FMOD_3D_CUSTOMROLLOFF)
    {
        mMode &= ~FMOD_3D_LOGROLLOFF;
        mMode &= ~FMOD_3D_LINEARROLLOFF;
        mMode |=  FMOD_3D_CUSTOMROLLOFF;
    }

    if (mode & FMOD_3D_IGNOREGEOMETRY)
    {
        mMode |= FMOD_3D_IGNOREGEOMETRY;
    }
    else
    {
        mMode &= ~FMOD_3D_IGNOREGEOMETRY;
    }

    if (mode & FMOD_VIRTUAL_PLAYFROMSTART)
    {
        mMode |= FMOD_VIRTUAL_PLAYFROMSTART;
    }
    else
    {
        mMode &= ~FMOD_VIRTUAL_PLAYFROMSTART;
    }

    /*
        Allow switching between 2D and 3D only in software.
    */
#if !defined( PLATFORM_GC ) && !defined( PLATFORM_WII ) && !defined( PLATFORM_PSP ) && !defined( PLATFORM_PS2 )
    if (!(mMode & FMOD_HARDWARE))
#endif
    {
        if (mode & FMOD_2D)
        {
            if (!mParent)
            {
                return FMOD_OK;
            }

            mMode &= ~FMOD_3D;
            mMode |=  FMOD_2D;
            mParent->mVolume3D              = 1.0f;
            mParent->mDirectOcclusionVolume = 1.0f;
            mParent->mReverbDryVolume       = 1.0f;
            mParent->mConeVolume3D          = 1.0f;
            mParent->mPitch3D               = 1.0f;
        }
        else if (mode & FMOD_3D)
        {
            mMode &= ~FMOD_2D;
            mMode |=  FMOD_3D;
        }
    }

    return FMOD_OK;
}


#ifdef PLATFORM_WII
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT  ChannelReal::setLowPassFilter(int cutoff)
{
    mLPFCutoff = cutoff;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT  ChannelReal::getLowPassFilter(int *cutoff)
{
    *cutoff = mLPFCutoff;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT  ChannelReal::setBiquadFilter(bool active, unsigned short b0, unsigned short b1, unsigned short b2, unsigned short a1, unsigned short a2)
{
    mBiquadActive = active;
    mBiquadB0     = b0;
    mBiquadB1     = b1;
    mBiquadB2     = b2;
    mBiquadA1     = a1;
    mBiquadA2     = a2;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT  ChannelReal::getBiquadFilter(bool *active, unsigned short *b0, unsigned short *b1, unsigned short *b2, unsigned short *a1, unsigned short *a2)
{
    *active = mBiquadActive;
    *b0     = mBiquadB0; 
    *b1     = mBiquadB1;
    *b2     = mBiquadB2;
    *a1     = mBiquadA1;
    *a2     = mBiquadA2;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT  ChannelReal::setControllerSpeaker(unsigned int controllerspeaker, int subchannel)
{
    mControllerSpeaker = controllerspeaker;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    
	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT  ChannelReal::getControllerSpeaker(unsigned int *controllerspeaker)
{
    *controllerspeaker = mControllerSpeaker;

    return FMOD_OK;
}
#endif

}


