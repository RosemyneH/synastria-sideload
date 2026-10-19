#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SOFTWARE

#include "fmod_dsp_filter.h"
#include "fmod_localcriticalsection.h"

#ifdef PLATFORM_PS3_SPU
    #include "fmod_systemi_spu.h"
#else
    #include "fmod_systemi.h"
#endif


namespace FMOD
{


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
// VERIFY: 80b5790c auto
// OFFSET: 0x00919CE0
FMOD_RESULT DSPFilter::release(bool freethis)
{
    stopBuffering();

    return DSPI::release(freethis);
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
// VERIFY: 0fba4daf manual
// OFFSET: 0x00919720
FMOD_RESULT DSPFilter::execute(float *inbuffer, float **outbuffer, unsigned int *length, int inchannels, int *outchannels, FMOD_SPEAKERMODE speakermode)
{
    FMOD_RESULT      result = FMOD_OK;
    LinkedListNode  *current;
    float           *mixbuffer;
    float           *tempbuffer;
    int              tempbufferindex;
    int              count;

    if (*length > mSystem->mDSPBlockSize)
    {
        *length = mSystem->mDSPBlockSize;
    }

    *outbuffer   = inbuffer;
    *outchannels = inchannels;

    mFlags |= FMOD_DSP_FLAG_IDLE;

    if (mFlags & FMOD_DSP_FLAG_VISITED)
    {
        *outbuffer   = mBuffer;
        *outchannels = mBufferChannels;

        mFlags &= ~FMOD_DSP_FLAG_IDLE;

        return result;
    }

    if (mBuffer)
    {
        mixbuffer       = mBuffer;
        mBufferChannels = inchannels;
    }
    else
    {
        mixbuffer = inbuffer;
    }

    tempbufferindex = mSystem->mDSPTempBuffIndex;
    tempbuffer      = mSystem->mDSPTempBuff[tempbufferindex];

    count   = 0;
    current = mInputHead.getNext();

    if (current == &mInputHead)
    {
        FMOD_memset(mixbuffer, 0, *length * inchannels * sizeof(float));
    }

    while (current != &mInputHead)
    {
        DSPConnectionI *connection = (DSPConnectionI *)current->getData();
        DSPI           *input      = connection->mInputUnit;

        if ((input->mFlags & FMOD_DSP_FLAG_ACTIVE) && !(input->mFlags & FMOD_DSP_FLAG_FINISHED))
        {
            int nextbufferindex = 1 - tempbufferindex;

            mSystem->mDSPTempBuffIndex = nextbufferindex;

            result = input->execute(tempbuffer, outbuffer, length, inchannels, outchannels, speakermode);
            if (result != FMOD_OK)
            {
                break;
            }

            if (!(input->mFlags & FMOD_DSP_FLAG_IDLE))
            {
                bool unity = false;

                mFlags &= ~FMOD_DSP_FLAG_IDLE;

                if (mNumInputs <= 1 && connection->mVolume == 1.0f && ((!connection->mSetLevelsUsed && mDescription.mCategory != FMOD_DSP_CATEGORY_SOUNDCARD) || connection->checkUnity(*outchannels, inchannels) == FMOD_OK))
                {
                    unity = true;
                }

                if (unity && (!mDescription.read || !mDescription.channels || mDescription.channels == *outchannels))
                {
                    if (mNumOutputs > 1)
                    {
                        if (*outbuffer)
                        {
                            FMOD_memcpy(mBuffer, *outbuffer, *length * *outchannels * sizeof(float));
                        }
                        else
                        {
                            FMOD_memset(mBuffer, 0, *length * *outchannels * sizeof(float));
                        }

                        *outbuffer = mBuffer;
                        mFlags &= ~FMOD_DSP_FLAG_IDLE;
                        mBufferChannels = *outchannels;
                    }

                    input->mFlags |= FMOD_DSP_FLAG_VISITED;
                    break;
                }

                if (!connection->mSetLevelsUsed)
                {
                    if (inchannels == *outchannels && connection->mVolume == 1.0f)
                    {
                        connection->setUnity();
                    }
                    else
                    {
                        connection->setPan(0.0f, inchannels, *outchannels, speakermode);
                    }

                    connection->mSetLevelsUsed = true;
                }

                if (*outbuffer == mixbuffer)
                {
                    if (mixbuffer == mSystem->mDSPTempBuff[0])
                    {
                        mixbuffer = mSystem->mDSPTempBuff[1];
                    }
                    else
                    {
                        mixbuffer = mSystem->mDSPTempBuff[0];
                    }

                    tempbufferindex = 1 - tempbufferindex;
                    tempbuffer = mSystem->mDSPTempBuff[nextbufferindex];
                }

                if (!count)
                {
                    FMOD_memset(mixbuffer, 0, *length * inchannels * sizeof(float));
                }

                connection->mix(mixbuffer, *outbuffer, inchannels, *outchannels, *length);
            }
            else if (!count)
            {
                FMOD_memset(mixbuffer, 0, *length * inchannels * sizeof(float));
            }
        }
        else if (!count)
        {
            FMOD_memset(mixbuffer, 0, *length * inchannels * sizeof(float));
        }

        *outbuffer   = mixbuffer;
        *outchannels = inchannels;

        input->mFlags |= FMOD_DSP_FLAG_VISITED;

        current = current->getNext();
        count++;
    }

    if (mDescription.read && !(mFlags & FMOD_DSP_FLAG_BYPASS))
    {
        int channels = *outchannels;

        if (mDescription.channels)
        {
            *outchannels = mDescription.channels;
            FMOD_memset(inbuffer, 0, *length * mDescription.channels * sizeof(float));
        }
        else if (!mNumInputs)
        {
            FMOD_memset(*outbuffer, 0, *length * channels * sizeof(float));
        }

        instance = (FMOD_DSP *)this;

        mDescription.read((FMOD_DSP_STATE *)this, *outbuffer, inbuffer, *length, channels, *outchannels);

        *outbuffer = inbuffer;

        mFlags &= ~FMOD_DSP_FLAG_IDLE;
    }

    if (mHistoryBuffer)
    {
        unsigned int  len = *length;
        float        *src = *outbuffer;

        while (len)
        {
            unsigned int size = len;

            if (mHistoryPosition + size > mHistoryLength)
            {
                size = mHistoryLength - mHistoryPosition;
            }

            FMOD_memcpy(mHistoryBuffer + (mHistoryPosition * *outchannels), src, size * *outchannels * sizeof(float));

            mHistoryPosition += size;
            src += size * *outchannels;
            len -= size;

            if (mHistoryPosition >= mHistoryLength)
            {
                mHistoryPosition = 0;
            }
        }
    }

    return result;
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
// VERIFY: 61a8be49 manual
// OFFSET: 0x00919C00
FMOD_RESULT DSPFilter::startBuffering(unsigned int historylength)
{
    int channels;

    if (!mHistoryBuffer || historylength != mHistoryLength)
    {
        mSystem->getSoftwareFormat(0, 0, &channels, 0, 0, 0);

        if (mHistoryBuffer)
        {
            FMOD_Memory_Free(mHistoryBuffer);
        }

        mHistoryLength = historylength;

        mHistoryBuffer = (float *)FMOD_Memory_Calloc(historylength * channels * sizeof(float));
        if (!mHistoryBuffer)
        {
            return FMOD_ERR_MEMORY;
        }

        mHistoryPosition = 0;
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 7da7389f auto
// OFFSET: 0x00919CA0
FMOD_RESULT DSPFilter::getHistoryBuffer(float **buffer, unsigned int *position, unsigned int *length)
{
    if (buffer)
    {
        *buffer = mHistoryBuffer;
    }

    if (position)
    {
        *position = mHistoryPosition;
    }

    if (length)
    {
        *length = mHistoryLength;
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPFilter::stopBuffering()
{
    if (mHistoryBuffer)
    {
        FMOD_Memory_Free(mHistoryBuffer);
        mHistoryBuffer = 0;
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


}

#endif
