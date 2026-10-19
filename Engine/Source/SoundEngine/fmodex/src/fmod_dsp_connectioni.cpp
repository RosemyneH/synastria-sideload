#include "fmod_settings.h"

#include "fmod_dspi.h"
#include "fmod_dsp_connectioni.h"
#include "fmod_globals.h"
#include "fmod_localcriticalsection.h"

#ifdef PLATFORM_PS3_SPU
    #include "fmod_systemi_spu.h"
#else
    #include "fmod_systemi.h"
#endif

namespace FMOD
{

#ifdef FMOD_SUPPORT_SIMD
extern "C"
{
    void FMOD_DSP_Connection_MixMonoToStereo_SIMD  (float *inbuffer, float *outbuffer, unsigned int length, float lvolume, float rvolume);
    void FMOD_DSP_Connection_MixStereoToStereo_SIMD(float *inbuffer, float *outbuffer, unsigned int length, float lvolume, float rvolume);
    void FMOD_DSP_Connection_MixMonoTo5_1_SIMD     (float *inbuffer, float *outbuffer, unsigned int length, float *volume0to3, float *volume4to1, float *volume2to5);
    void FMOD_DSP_Connection_Mix5_1To5_1_SIMD      (float *inbuffer, float *outbuffer, unsigned int length, float *volume0to3, float *volume4to1, float *volume2to5);
    void FMOD_DSP_Connection_MixMonoTo7_1_SIMD     (float *inbuffer, float *outbuffer, unsigned int length, float *volume0to3, float *volume4to7);
    void FMOD_DSP_Connection_Mix7_1To7_1_SIMD      (float *inbuffer, float *outbuffer, unsigned int length, float *volume0to3, float *volume4to7);
    void FMOD_DSP_Connection_MixStereoTo7_1_SIMD   (float *inbuffer, float *outbuffer, unsigned int length, float *volume0to3, float *volume01to31, float *volume4to7, float *volume41to71);
}
#endif



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
// VERIFY: 543af1c7 manual
// OFFSET: 0x00923020
FMOD_RESULT DSPConnectionI::init(DSP_LEVEL_TYPE * &levelmemory, int maxoutputlevels, int maxinputlevels)
{  
#ifdef FMOD_SUPPORT_SOFTWARE
    mMaxOutputLevels = maxoutputlevels;
    mMaxInputLevels  = maxinputlevels;

    if (maxoutputlevels < 2)
    {
        maxoutputlevels = 2;
    }

    if (mMaxInputLevels < mMaxOutputLevels)
    {
        mMaxInputLevels = mMaxOutputLevels;
    }

    #ifdef PLATFORM_PS3_PPU
    mMramAddress        = (unsigned int)this;
    mMramAddressLevels  = (unsigned int)levelmemory;
    #endif

    int count;
    for (count = 0; count < DSP_MAXLEVELS_OUT; count++)
    {
        if (count < mMaxOutputLevels)
        {
            mLevel[count]        = levelmemory;
            levelmemory += mMaxInputLevels;
            mLevelCurrent[count] = levelmemory;
            levelmemory += mMaxInputLevels;
            mLevelDelta[count]   = levelmemory;
            levelmemory += mMaxInputLevels;
        }
        else
        {
            mLevel[count]        = 0;
            mLevelCurrent[count] = 0;
            mLevelDelta[count]   = 0;
        }
    }

    return FMOD_OK;
#else
    return FMOD_ERR_NEEDSSOFTWARE;
#endif
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
FMOD_RESULT DSPConnectionI::getInput(DSPI **input)
{
#ifdef FMOD_SUPPORT_SOFTWARE
    if (!input)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!mInputUnit)
    {
        *input = 0;
        return FMOD_ERR_NOTREADY;
    }

    *input = mInputUnit;

    return FMOD_OK;
#else
    return FMOD_ERR_NEEDSSOFTWARE;
#endif
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
FMOD_RESULT DSPConnectionI::getOutput(DSPI **output)
{
    if (!output)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!mOutputUnit)
    {
        *output = 0;
        return FMOD_ERR_NOTREADY;
    }

    *output = mOutputUnit;

    return FMOD_OK;
}

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
// VERIFY: db23d968 manual
// OFFSET: 0x009230A0
FMOD_RESULT DSPConnectionI::reset()
{
    int count, count2;

    mVolume = 1.0f;

    for (count = 0; count < mMaxOutputLevels; count++)
    {
        for (count2 = 0; count2 < mMaxInputLevels; count2++)
        {
            mLevel       [count][count2] = 0;
            mLevelCurrent[count][count2] = 0;
            mLevelDelta  [count][count2] = 0;
        }
    }
    
    mSetLevelsUsed = false;

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
// VERIFY: 337f59d7 manual
// OFFSET: 0x00923130
FMOD_RESULT DSPConnectionI::setUnity()
{
    int count, count2;

    for (count = 0; count < mMaxOutputLevels; count++)
    {
        for (count2 = 0; count2 < mMaxInputLevels; count2++)
        {
            if (count == count2)
            {
                mLevelCurrent[count][count2] = DSP_LEVEL_COMPRESS(1.0f);
                mLevel[count][count2] = DSP_LEVEL_COMPRESS(1.0f);
            }
            else
            {
                mLevel[count][count2] = 0;
                mLevelCurrent[count][count2] = 0;
            }
        }
    }
    mVolume = 1.0f;
    mRampCount = 0;

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
// VERIFY: a98529b3 manual
// OFFSET: 0x00923D50
FMOD_RESULT DSPConnectionI::mix(float * FMOD_RESTRICT outbuffer, float * FMOD_RESTRICT inbuffer, int outchannels, int inchannels, unsigned int length)
{
    unsigned int count;
    float        l00, l10;
    float        l01, l11;
#if (DSP_MAXLEVELS_OUT > 2)
    float        l20, l30, l40, l50;
    float        l21, l31, l41, l51;
    float        l22, l33, l44, l55;
#endif    
#if (DSP_MAXLEVELS_OUT > 6)
    float        l60, l70;
    float        l61, l71;
    float        l66, l77;
#endif
    bool         supportssimd;

    if (!inbuffer)
    {
        return FMOD_OK;
    }

    supportssimd = FMOD_OS_SupportsSIMD();

    if (mRampCount)
    {
        FMOD_RESULT  result;
        unsigned int len = mRampCount;

        if (len > length)
        {
            len = length;
        }

        result = mixAndRamp(outbuffer, inbuffer, outchannels, inchannels, len);
        if (result != FMOD_OK)
        {
            return result;
        }

        length -= len;
        outbuffer += (len * outchannels);
        inbuffer += (len * inchannels);
    }

    if (!length)
    {
        return FMOD_OK;
    }

    if (mVolume == 0)
    {
        return FMOD_OK;
    }
    
    l00 = mLevelCurrent[0] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[0][0]) : 0;
    l10 = mLevelCurrent[1] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[1][0]) : 0;

#if (DSP_MAXLEVELS_OUT > 2)
    l20 = mLevelCurrent[2] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[2][0]) : 0;
    l30 = mLevelCurrent[3] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[3][0]) : 0;
    l40 = mLevelCurrent[4] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[4][0]) : 0;
    l50 = mLevelCurrent[5] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[5][0]) : 0;
#endif
#if (DSP_MAXLEVELS_OUT > 6)
    l60 = mLevelCurrent[6] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[6][0]) : 0;
    l70 = mLevelCurrent[7] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[7][0]) : 0;
#endif

    l01 = mLevelCurrent[0] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[0][1]) : 0;
    l11 = mLevelCurrent[1] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[1][1]) : 0;

#if (DSP_MAXLEVELS_OUT > 2)
    l21 = mLevelCurrent[2] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[2][1]) : 0;
    l31 = mLevelCurrent[3] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[3][1]) : 0;
    l41 = mLevelCurrent[4] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[4][1]) : 0;
    l51 = mLevelCurrent[5] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[5][1]) : 0;

    l22 = mLevelCurrent[2] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[2][2]) : 0;
    l33 = mLevelCurrent[3] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[3][3]) : 0;
    l44 = mLevelCurrent[4] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[4][4]) : 0;
    l55 = mLevelCurrent[5] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[5][5]) : 0;
#endif
#if (DSP_MAXLEVELS_OUT > 6)
    l61 = mLevelCurrent[6] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[6][1]) : 0;
    l71 = mLevelCurrent[7] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[7][1]) : 0;

    l66 = mLevelCurrent[6] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[6][6]) : 0;
    l77 = mLevelCurrent[7] ? DSP_LEVEL_DECOMPRESS(mLevelCurrent[7][7]) : 0;
#endif

    /*
        The most common configuration is mono or stereo coming in, and stereo going out, so we special case these for speed.
        Other combinations go through generic for loop code.
    */
    if (0)
    {
    }
#if !defined(PLATFORM_PS3_SPU)
    else if (outchannels == 2 && (inchannels == 1 || inchannels == 2))
    {
        if (inchannels == 1)
        {
            #ifdef FMOD_SUPPORT_SIMD
            if (supportssimd)
            {
                /*
                    First 16byte align destination for simd.
                */
                while ((unsigned int)outbuffer & 0xF && length)
                {
                    outbuffer[0] += (inbuffer[0] * l00);
                    outbuffer[1] += (inbuffer[0] * l10);
                    inbuffer  ++;
                    outbuffer += 2;
                    length    --;
                }
                FMOD_DSP_Connection_MixMonoToStereo_SIMD(inbuffer, outbuffer, length, l00, l10);
            }
            else
            #endif
            {
                unsigned int len;

                len = length >> 2;
                while (len)
                {
                    outbuffer[0] += (inbuffer[0] * l00);
                    outbuffer[1] += (inbuffer[0] * l10);
                    outbuffer[2] += (inbuffer[1] * l00);
                    outbuffer[3] += (inbuffer[1] * l10);
                    outbuffer[4] += (inbuffer[2] * l00);
                    outbuffer[5] += (inbuffer[2] * l10);
                    outbuffer[6] += (inbuffer[3] * l00);
                    outbuffer[7] += (inbuffer[3] * l10);
                    inbuffer  += 4;
                    outbuffer += 8;
                    len       --;
                }
                len = length & 3;
                while (len)
                {
                    outbuffer[0] += (inbuffer[0] * l00);
                    outbuffer[1] += (inbuffer[0] * l10);
                    inbuffer  ++;
                    outbuffer += 2;
                    len       --;
                }
            }
        }
        else if (inchannels == 2)
        {
            unsigned int len;

            if (l01 < DSP_LEVEL_SMALLVAL && l10 < DSP_LEVEL_SMALLVAL)     /* Pan matrix looks like [l 0] which is the normal way. */
            {                                       /*                       [0 r]                          */
                #ifdef FMOD_SUPPORT_SIMD
                if (supportssimd)
                {
                    /*
                        First 16byte align destination for simd.
                    */
                    while ((unsigned int)outbuffer & 0xF && length)
                    {
                        outbuffer[0] += (inbuffer[0] * l00);
                        outbuffer[1] += (inbuffer[1] * l11);
                        inbuffer  += 2;
                        outbuffer += 2;
                        length    --;
                    }
                    FMOD_DSP_Connection_MixStereoToStereo_SIMD(inbuffer, outbuffer, length, l00, l11);
                }
                else
                #endif
                {
                    len = length >> 2;
                    while (len)
                    {
                        outbuffer[0] += (inbuffer[0] * l00);
                        outbuffer[1] += (inbuffer[1] * l11);

                        outbuffer[2] += (inbuffer[2] * l00);
                        outbuffer[3] += (inbuffer[3] * l11);

                        outbuffer[4] += (inbuffer[4] * l00);
                        outbuffer[5] += (inbuffer[5] * l11);

                        outbuffer[6] += (inbuffer[6] * l00);
                        outbuffer[7] += (inbuffer[7] * l11);
                        inbuffer  += 8;
                        outbuffer += 8;
                        len       --;
                    }

                    len = length & 3;
                    while (len)
                    {
                        outbuffer[0] += (inbuffer[0] * l00);
                        outbuffer[1] += (inbuffer[1] * l11);
                        inbuffer  += 2;
                        outbuffer += 2;
                        len       --;
                    }
                }
            }
            else                                /* Pan matrix looks like [l ?] must have used setSpeakerMix. */
            {                                   /*                       [? r]                               */
                len = length >> 2;
                while (len)
                {
                    outbuffer[0] += (inbuffer[0] * l00);
                    outbuffer[0] += (inbuffer[1] * l01);
                    outbuffer[1] += (inbuffer[0] * l10);
                    outbuffer[1] += (inbuffer[1] * l11);

                    outbuffer[2] += (inbuffer[2] * l00);
                    outbuffer[2] += (inbuffer[3] * l01);
                    outbuffer[3] += (inbuffer[2] * l10);
                    outbuffer[3] += (inbuffer[3] * l11);

                    outbuffer[4] += (inbuffer[4] * l00);
                    outbuffer[4] += (inbuffer[5] * l01);
                    outbuffer[5] += (inbuffer[4] * l10);
                    outbuffer[5] += (inbuffer[5] * l11);

                    outbuffer[6] += (inbuffer[6] * l00);
                    outbuffer[6] += (inbuffer[7] * l01);
                    outbuffer[7] += (inbuffer[6] * l10);
                    outbuffer[7] += (inbuffer[7] * l11);
                    inbuffer  += 8;
                    outbuffer += 8;
                    len       --;
                }

                len = length & 3;
                while (len)
                {
                    outbuffer[0] += (inbuffer[0] * l00);
                    outbuffer[0] += (inbuffer[1] * l01);
                    outbuffer[1] += (inbuffer[0] * l10);
                    outbuffer[1] += (inbuffer[1] * l11);
                    inbuffer  += 2;
                    outbuffer += 2;
                    len       --;
                }

            }
        }
    }
#if (DSP_MAXLEVELS_OUT > 2) && !defined(PLATFORM_PS3_PPU)
    else if (outchannels == 6 && (inchannels == 1 || inchannels == 2 || inchannels == 6))//(inchannels == 6 && checkUnity(outchannels, inchannels) == FMOD_OK)))
    {
        if (inchannels == 1)
        {
            #ifdef FMOD_SUPPORT_SIMD   
            if (supportssimd)
            {
                float volume0to3[4] = { l00, l10, l20, l30 };
                float volume4to1[4] = { l40, l50, l00, l10 };
                float volume2to5[4] = { l20, l30, l40, l50 };

                /*
                    First 16byte align destination for simd.
                */
                while ((unsigned int)outbuffer & 0xF && length)
                {
                    outbuffer[0]  += (inbuffer[0] * l00);
                    outbuffer[1]  += (inbuffer[0] * l10);
                    outbuffer[2]  += (inbuffer[0] * l20);
                    outbuffer[3]  += (inbuffer[0] * l30);
                    outbuffer[4]  += (inbuffer[0] * l40);
                    outbuffer[5]  += (inbuffer[0] * l50);

                    inbuffer  ++;
                    outbuffer +=6;
                    length    --;
                }

                FMOD_DSP_Connection_MixMonoTo5_1_SIMD(inbuffer, outbuffer, length, volume0to3, volume4to1, volume2to5);
            }
            else
            #endif
            {
                unsigned int len;

                len = length >> 2;
                while (len)
                {               
                    outbuffer[0]  += (inbuffer[0] * l00);
                    outbuffer[1]  += (inbuffer[0] * l10);
                    outbuffer[2]  += (inbuffer[0] * l20);
                    outbuffer[3]  += (inbuffer[0] * l30);
                    
                    outbuffer[4]  += (inbuffer[0] * l40);
                    outbuffer[5]  += (inbuffer[0] * l50);
                    outbuffer[6]  += (inbuffer[1] * l00);
                    outbuffer[7]  += (inbuffer[1] * l10);
                    
                    outbuffer[8]  += (inbuffer[1] * l20);
                    outbuffer[9]  += (inbuffer[1] * l30);
                    outbuffer[10] += (inbuffer[1] * l40);
                    outbuffer[11] += (inbuffer[1] * l50);

                    outbuffer[12]  += (inbuffer[2] * l00);
                    outbuffer[13]  += (inbuffer[2] * l10);
                    outbuffer[14]  += (inbuffer[2] * l20);
                    outbuffer[15]  += (inbuffer[2] * l30);
                    
                    outbuffer[16]  += (inbuffer[2] * l40);
                    outbuffer[17]  += (inbuffer[2] * l50);
                    outbuffer[18]  += (inbuffer[3] * l00);
                    outbuffer[19]  += (inbuffer[3] * l10);
                    
                    outbuffer[20]  += (inbuffer[3] * l20);
                    outbuffer[21]  += (inbuffer[3] * l30);
                    outbuffer[22]  += (inbuffer[3] * l40);
                    outbuffer[23]  += (inbuffer[3] * l50);
                    
                    outbuffer += 24;
                    inbuffer  += 4;
                    
                    len--;
                }
                
                len = length & 3;
                if (len)
                {
                    while (len)
                    {
                        outbuffer[0]  += (inbuffer[0] * l00);
                        outbuffer[1]  += (inbuffer[0] * l10);
                        outbuffer[2]  += (inbuffer[0] * l20);
                        outbuffer[3]  += (inbuffer[0] * l30);
                        outbuffer[4]  += (inbuffer[0] * l40);
                        outbuffer[5]  += (inbuffer[0] * l50);

                        inbuffer  ++;
                        outbuffer +=6;
                        len       --;
                    }
                }
            }
        }
        else if (inchannels == 2)
        {
            unsigned int len;
   
            /* Pan matrix looks like [l 0] which is the normal way. */
            /*                       [0 r]                          */
            /*                       [0 0]                          */
            /*                       [0 0]                          */
            /*                       [0 0]                          */
            /*                       [0 0]                          */
            if (l01 < DSP_LEVEL_SMALLVAL && l10 < DSP_LEVEL_SMALLVAL && 
                l20 < DSP_LEVEL_SMALLVAL && l21 < DSP_LEVEL_SMALLVAL &&
                l30 < DSP_LEVEL_SMALLVAL && l31 < DSP_LEVEL_SMALLVAL &&
                l40 < DSP_LEVEL_SMALLVAL && l41 < DSP_LEVEL_SMALLVAL &&
                l50 < DSP_LEVEL_SMALLVAL && l51 < DSP_LEVEL_SMALLVAL)
            {                                       
                len = length >> 2;
                while (len)
                {
                    outbuffer[0]   += (inbuffer[0] * l00);
                    outbuffer[1]   += (inbuffer[1] * l11);
//                    outbuffer[2   += 0;
//                    outbuffer[3]  += 0;
//                    outbuffer[4]  += 0;
//                    outbuffer[5]  += 0;

                    outbuffer[6]   += (inbuffer[2] * l00);
                    outbuffer[7]   += (inbuffer[3] * l11);
//                    outbuffer[8]  += 0;
//                    outbuffer[9]  += 0;
//                    outbuffer[10] += 0;
//                    outbuffer[11] += 0;

                    outbuffer[12]  += (inbuffer[4] * l00);
                    outbuffer[13]  += (inbuffer[5] * l11);
//                    outbuffer[14] += 0;
//                    outbuffer[15] += 0;
//                    outbuffer[16] += 0;
//                    outbuffer[17] += 0;

                    outbuffer[18]  += (inbuffer[6] * l00);
                    outbuffer[19]  += (inbuffer[7] * l11);
//                   outbuffer[20] += 0;
//                   outbuffer[21] += 0;
//                   outbuffer[22] += 0;
//                   outbuffer[23] += 0;
                    
                    outbuffer += 24;
                    inbuffer  += 8;
                    len--;
                }

                len = length & 3;
                if (len)
                {               
                    while (len)
                    {
                        outbuffer[0]  += (inbuffer[0] * l00);
                        outbuffer[1]  += (inbuffer[1] * l11);
                        //outbuffer[2]  += 0;
                        //outbuffer[3]  += 0;
                        //outbuffer[4]  += 0;
                        //outbuffer[5]  += 0;

                        inbuffer  += 2;
                        outbuffer += 6;
                        len       --;
                    }
                }
            }
            else                                /* Pan matrix looks like [l ?] must have used setSpeakerMix. */
            {                                   /*                       [? r]                               */
                len = length >> 1;
                while (len)
                {
                    outbuffer[0] += (inbuffer[0] * l00);
                    outbuffer[0] += (inbuffer[1] * l01);
                    outbuffer[1] += (inbuffer[0] * l10);
                    outbuffer[1] += (inbuffer[1] * l11);
                    outbuffer[2] += (inbuffer[0] * l20);
                    outbuffer[2] += (inbuffer[1] * l21);
                    outbuffer[3] += (inbuffer[0] * l30);
                    outbuffer[3] += (inbuffer[1] * l31);
                    outbuffer[4] += (inbuffer[0] * l40);
                    outbuffer[4] += (inbuffer[1] * l41);
                    outbuffer[5] += (inbuffer[0] * l50);
                    outbuffer[5] += (inbuffer[1] * l51);

                    outbuffer[6] += (inbuffer[2] * l00);
                    outbuffer[6] += (inbuffer[3] * l01);
                    outbuffer[7] += (inbuffer[2] * l10);
                    outbuffer[7] += (inbuffer[3] * l11);
                    outbuffer[8] += (inbuffer[2] * l20);
                    outbuffer[8] += (inbuffer[3] * l21);
                    outbuffer[9] += (inbuffer[2] * l30);
                    outbuffer[9] += (inbuffer[3] * l31);
                    outbuffer[10] += (inbuffer[2] * l40);
                    outbuffer[10] += (inbuffer[3] * l41);
                    outbuffer[11] += (inbuffer[2] * l50);
                    outbuffer[11] += (inbuffer[3] * l51);

                    inbuffer  += 4;
                    outbuffer += 12;
                    len       --;
                }

                len = length & 1;
                while (len)
                {
                    outbuffer[0] += (inbuffer[0] * l00);
                    outbuffer[0] += (inbuffer[1] * l01);
                    outbuffer[1] += (inbuffer[0] * l10);
                    outbuffer[1] += (inbuffer[1] * l11);
                    outbuffer[2] += (inbuffer[0] * l20);
                    outbuffer[2] += (inbuffer[1] * l21);
                    outbuffer[3] += (inbuffer[0] * l30);
                    outbuffer[3] += (inbuffer[1] * l31);
                    outbuffer[4] += (inbuffer[0] * l40);
                    outbuffer[4] += (inbuffer[1] * l41);
                    outbuffer[5] += (inbuffer[0] * l50);
                    outbuffer[5] += (inbuffer[1] * l51);

                    inbuffer  += 2;
                    outbuffer += 6;
                    len       --;
                }
            }
        }
        else if (inchannels == 6)
        {
            if (checkUnity(outchannels, inchannels) == FMOD_OK)
            {
                float l[6];

                l[0] = l00;
                l[1] = l11;
                l[2] = l22;
                l[3] = l33;
                l[4] = l44;
                l[5] = l55;

                #ifdef FMOD_SUPPORT_SIMD
                if (supportssimd)
                {
                    float volume0to3[4] = { l[0], l[1], l[2], l[3] };
                    float volume4to1[4] = { l[4], l[5], l[0], l[1] };
                    float volume2to5[4] = { l[2], l[3], l[4], l[5] };

                    /*
                        First 16byte align destination for simd.
                    */
                    while ((unsigned int)outbuffer & 0xF && length)
                    {
                        outbuffer[0]  += (inbuffer[0] * l[0]);
                        outbuffer[1]  += (inbuffer[1] * l[1]);
                        outbuffer[2]  += (inbuffer[2] * l[2]);
                        outbuffer[3]  += (inbuffer[3] * l[3]);
                        outbuffer[4]  += (inbuffer[4] * l[4]);
                        outbuffer[5]  += (inbuffer[5] * l[5]);

                        inbuffer  +=6;
                        outbuffer +=6;
                        length    --;
                    }

                    FMOD_DSP_Connection_Mix5_1To5_1_SIMD(inbuffer, outbuffer, length, volume0to3, volume4to1, volume2to5);
                }
                else
                #endif
                {
                    unsigned int len;
                         
                    len = length >> 2;
                    while (len)
                    {                   
                        outbuffer[0]  += (inbuffer[0]  * l[0]);
                        outbuffer[1]  += (inbuffer[1]  * l[1]);
                        outbuffer[2]  += (inbuffer[2]  * l[2]);
                        outbuffer[3]  += (inbuffer[3]  * l[3]);
                        outbuffer[4]  += (inbuffer[4]  * l[4]);
                        outbuffer[5]  += (inbuffer[5]  * l[5]);
                        outbuffer[6]  += (inbuffer[6]  * l[0]);
                        outbuffer[7]  += (inbuffer[7]  * l[1]);
                        outbuffer[8]  += (inbuffer[8]  * l[2]);
                        outbuffer[9]  += (inbuffer[9]  * l[3]);
                        outbuffer[10] += (inbuffer[10] * l[4]);
                        outbuffer[11] += (inbuffer[11] * l[5]);

                        outbuffer[12]  += (inbuffer[12] * l[0]);
                        outbuffer[13]  += (inbuffer[13] * l[1]);
                        outbuffer[14]  += (inbuffer[14] * l[2]);
                        outbuffer[15]  += (inbuffer[15] * l[3]);
                        outbuffer[16]  += (inbuffer[16] * l[4]);
                        outbuffer[17]  += (inbuffer[17] * l[5]);
                        outbuffer[18]  += (inbuffer[18] * l[0]);
                        outbuffer[19]  += (inbuffer[19] * l[1]);
                        outbuffer[20]  += (inbuffer[20] * l[2]);
                        outbuffer[21]  += (inbuffer[21] * l[3]);
                        outbuffer[22]  += (inbuffer[22] * l[4]);
                        outbuffer[23]  += (inbuffer[23] * l[5]);
                    
                        outbuffer += 24;
                        inbuffer += 24;
                        len--;
                    }
                
                    len = length & 3;
                    if (len)
                    {
                        while (len)
                        {
                            outbuffer[0]  += (inbuffer[0] * l[0]);
                            outbuffer[1]  += (inbuffer[1] * l[1]);
                            outbuffer[2]  += (inbuffer[2] * l[2]);
                            outbuffer[3]  += (inbuffer[3] * l[3]);
                            outbuffer[4]  += (inbuffer[4] * l[4]);
                            outbuffer[5]  += (inbuffer[5] * l[5]);

                            inbuffer  +=6;
                            outbuffer +=6;
                            len       --;
                        }
                    }
                }
            }
            else
            {
                #if 0
                for (count = 0; count < length; count++)
                {
                    int count2;
            
                    for (count2 = 0; count2 < outchannels; count2++)
                    {
                        float srcval  = 0;
                        float destval = *outbuffer;
                        int count3;
 
                        for (count3 = 0; count3 < inchannels; count3++)
                        {
                            srcval += (inbuffer[count3] * mLevelCurrent[count2][count3]);
                        }
 
                        destval += srcval;
                        *outbuffer++ = destval;
                    }
                    
                    inbuffer += inchannels;
                }
                #else
                for (count = 0; count < length; count++)
                {
                    float srcval;
                    
                    srcval  = (inbuffer[0] * mLevelCurrent[0][0]);
                    srcval += (inbuffer[1] * mLevelCurrent[0][1]);
                    srcval += (inbuffer[2] * mLevelCurrent[0][2]);
                    srcval += (inbuffer[3] * mLevelCurrent[0][3]);
                    srcval += (inbuffer[4] * mLevelCurrent[0][4]);
                    srcval += (inbuffer[5] * mLevelCurrent[0][5]);
                    outbuffer[0] += srcval;

                    srcval  = (inbuffer[0] * mLevelCurrent[1][0]);
                    srcval += (inbuffer[1] * mLevelCurrent[1][1]);
                    srcval += (inbuffer[2] * mLevelCurrent[1][2]);
                    srcval += (inbuffer[3] * mLevelCurrent[1][3]);
                    srcval += (inbuffer[4] * mLevelCurrent[1][4]);
                    srcval += (inbuffer[5] * mLevelCurrent[1][5]);
                    outbuffer[1] += srcval;

                    srcval  = (inbuffer[0] * mLevelCurrent[2][0]);
                    srcval += (inbuffer[1] * mLevelCurrent[2][1]);
                    srcval += (inbuffer[2] * mLevelCurrent[2][2]);
                    srcval += (inbuffer[3] * mLevelCurrent[2][3]);
                    srcval += (inbuffer[4] * mLevelCurrent[2][4]);
                    srcval += (inbuffer[5] * mLevelCurrent[2][5]);
                    outbuffer[2] += srcval;

                    srcval  = (inbuffer[0] * mLevelCurrent[3][0]);
                    srcval += (inbuffer[1] * mLevelCurrent[3][1]);
                    srcval += (inbuffer[2] * mLevelCurrent[3][2]);
                    srcval += (inbuffer[3] * mLevelCurrent[3][3]);
                    srcval += (inbuffer[4] * mLevelCurrent[3][4]);
                    srcval += (inbuffer[5] * mLevelCurrent[3][5]);
                    outbuffer[3] += srcval;

                    srcval  = (inbuffer[0] * mLevelCurrent[4][0]);
                    srcval += (inbuffer[1] * mLevelCurrent[4][1]);
                    srcval += (inbuffer[2] * mLevelCurrent[4][2]);
                    srcval += (inbuffer[3] * mLevelCurrent[4][3]);
                    srcval += (inbuffer[4] * mLevelCurrent[4][4]);
                    srcval += (inbuffer[5] * mLevelCurrent[4][5]);
                    outbuffer[4] += srcval;

                    srcval  = (inbuffer[0] * mLevelCurrent[5][0]);
                    srcval += (inbuffer[1] * mLevelCurrent[5][1]);
                    srcval += (inbuffer[2] * mLevelCurrent[5][2]);
                    srcval += (inbuffer[3] * mLevelCurrent[5][3]);
                    srcval += (inbuffer[4] * mLevelCurrent[5][4]);
                    srcval += (inbuffer[5] * mLevelCurrent[5][5]);
                    outbuffer[5] += srcval;

                    outbuffer += 6;
                    inbuffer += 6;
                }
                #endif
            }
        }
    }
#endif    
#endif  /* #if (DSP_MAXLEVELS_OUT != 8) */

#if (DSP_MAXLEVELS_OUT > 6) && !defined(PLATFORM_PS3_PPU)
    else if (outchannels == 8 && (inchannels == 1 || inchannels == 2 || inchannels == 8)) // && checkUnity(outchannels, inchannels) == FMOD_OK)))
    {
        if (inchannels == 1)
        {
            #ifdef FMOD_SUPPORT_SIMD   
            if (supportssimd)
            {
                float volume0to3[4] = { l00, l10, l20, l30 };
                float volume4to7[4] = { l40, l50, l60, l70 };

                FMOD_DSP_Connection_MixMonoTo7_1_SIMD(inbuffer, outbuffer, length, volume0to3, volume4to7);
            }
            else
            #endif
            {
                unsigned int len;

                len = length >> 1;
                while (len)
                {               
                    outbuffer[0]  += (inbuffer[0] * l00);
                    outbuffer[1]  += (inbuffer[0] * l10);
                    outbuffer[2]  += (inbuffer[0] * l20);
                    outbuffer[3]  += (inbuffer[0] * l30);
                    outbuffer[4]  += (inbuffer[0] * l40);
                    outbuffer[5]  += (inbuffer[0] * l50);
                    outbuffer[6]  += (inbuffer[0] * l60);
                    outbuffer[7]  += (inbuffer[0] * l70);
                    
                    outbuffer[8]  += (inbuffer[1] * l00);
                    outbuffer[9]  += (inbuffer[1] * l10);
                    outbuffer[10] += (inbuffer[1] * l20);
                    outbuffer[11] += (inbuffer[1] * l30);
                    outbuffer[12] += (inbuffer[1] * l40);
                    outbuffer[13] += (inbuffer[1] * l50);
                    outbuffer[14] += (inbuffer[1] * l60);
                    outbuffer[15] += (inbuffer[1] * l70);
                                       
                    outbuffer += 16;
                    inbuffer  += 2;
                    
                    len--;
                }
                
                len = length & 1;
                if (len)
                {
                    while (len)
                    {
                        outbuffer[0]  += (inbuffer[0] * l00);
                        outbuffer[1]  += (inbuffer[0] * l10);
                        outbuffer[2]  += (inbuffer[0] * l20);
                        outbuffer[3]  += (inbuffer[0] * l30);
                        outbuffer[4]  += (inbuffer[0] * l40);
                        outbuffer[5]  += (inbuffer[0] * l50);
                        outbuffer[6]  += (inbuffer[0] * l60);
                        outbuffer[7]  += (inbuffer[0] * l70);

                        inbuffer  ++;
                        outbuffer +=8;
                        len       --;
                    }
                }
            }
        }
        else if (inchannels == 2)
        {
            unsigned int len;
   
            /* Pan matrix looks like [l 0] which is the normal way. */
            /*                       [0 r]                          */
            /*                       [0 0]                          */
            /*                       [0 0]                          */
            /*                       [0 0]                          */
            /*                       [0 0]                          */
            if (l01 < DSP_LEVEL_SMALLVAL && l10 < DSP_LEVEL_SMALLVAL && 
                l20 < DSP_LEVEL_SMALLVAL && l21 < DSP_LEVEL_SMALLVAL &&
                l30 < DSP_LEVEL_SMALLVAL && l31 < DSP_LEVEL_SMALLVAL &&
                l40 < DSP_LEVEL_SMALLVAL && l41 < DSP_LEVEL_SMALLVAL &&
                l50 < DSP_LEVEL_SMALLVAL && l51 < DSP_LEVEL_SMALLVAL &&
                l60 < DSP_LEVEL_SMALLVAL && l61 < DSP_LEVEL_SMALLVAL &&
                l70 < DSP_LEVEL_SMALLVAL && l71 < DSP_LEVEL_SMALLVAL)
            {                                       
                len = length >> 2;
                while (len)
                {
                    outbuffer[0]   += (inbuffer[0] * l00);
                    outbuffer[1]   += (inbuffer[1] * l11);
//                    outbuffer[2   += 0;
//                    outbuffer[3]  += 0;
//                    outbuffer[4]  += 0;
//                    outbuffer[5]  += 0;
//                    outbuffer[6]  += 0;
//                    outbuffer[7]  += 0;

                    outbuffer[8]   += (inbuffer[2] * l00);
                    outbuffer[9]   += (inbuffer[3] * l11);
//                    outbuffer[10]  += 0;
//                    outbuffer[11]  += 0;
//                    outbuffer[12] += 0;
//                    outbuffer[13] += 0;
//                    outbuffer[14] += 0;
//                    outbuffer[15] += 0;

                    outbuffer[16]  += (inbuffer[4] * l00);
                    outbuffer[17]  += (inbuffer[5] * l11);
//                    outbuffer[18] += 0;
//                    outbuffer[19] += 0;
//                    outbuffer[20] += 0;
//                    outbuffer[21] += 0;
//                    outbuffer[22] += 0;
//                    outbuffer[23] += 0;

                    outbuffer[24]  += (inbuffer[6] * l00);
                    outbuffer[25]  += (inbuffer[7] * l11);
//                   outbuffer[26] += 0;
//                   outbuffer[27] += 0;
//                   outbuffer[28] += 0;
//                   outbuffer[29] += 0;
//                   outbuffer[30] += 0;
//                   outbuffer[31] += 0;
                    
                    outbuffer += 32;
                    inbuffer  += 8;
                    len--;
                }

                len = length & 3;
                if (len)
                {               
                    while (len)
                    {
                        outbuffer[0]  += (inbuffer[0] * l00);
                        outbuffer[1]  += (inbuffer[1] * l11);
                        //outbuffer[2]  += 0;
                        //outbuffer[3]  += 0;
                        //outbuffer[4]  += 0;
                        //outbuffer[5]  += 0;
                        //outbuffer[6]  += 0;
                        //outbuffer[7]  += 0;

                        inbuffer  += 2;
                        outbuffer += 8;
                        len       --;
                    }
                }
            }
            else                                /* Pan matrix looks like [l ?] must have used setSpeakerMix. */
            {                                   /*                       [? r]                               */
#ifdef PLATFORM_XENON
                float volume0to3[4]   = { l00, l10, l20, l30 };
                float volume01to31[4] = { l01, l11, l21, l31 };
                float volume4to7[4]   = { l40, l50, l60, l70 };
                float volume41to71[4] = { l41, l51, l61, l71 };

                FMOD_DSP_Connection_MixStereoTo7_1_SIMD(inbuffer, outbuffer, length, volume0to3, volume01to31, volume4to7, volume41to71);
#else
                len = length >> 1;
                while (len)
                {
                    outbuffer[0] += (inbuffer[0] * l00);
                    outbuffer[0] += (inbuffer[1] * l01);
                    outbuffer[1] += (inbuffer[0] * l10);
                    outbuffer[1] += (inbuffer[1] * l11);
                    outbuffer[2] += (inbuffer[0] * l20);
                    outbuffer[2] += (inbuffer[1] * l21);
                    outbuffer[3] += (inbuffer[0] * l30);
                    outbuffer[3] += (inbuffer[1] * l31);
                    outbuffer[4] += (inbuffer[0] * l40);
                    outbuffer[4] += (inbuffer[1] * l41);
                    outbuffer[5] += (inbuffer[0] * l50);
                    outbuffer[5] += (inbuffer[1] * l51);
                    outbuffer[6] += (inbuffer[0] * l60);
                    outbuffer[6] += (inbuffer[1] * l61);
                    outbuffer[7] += (inbuffer[0] * l70);
                    outbuffer[7] += (inbuffer[1] * l71);

                    outbuffer[8] += (inbuffer[2] * l00);
                    outbuffer[8] += (inbuffer[3] * l01);
                    outbuffer[9] += (inbuffer[2] * l10);
                    outbuffer[9] += (inbuffer[3] * l11);
                    outbuffer[10] += (inbuffer[2] * l20);
                    outbuffer[10] += (inbuffer[3] * l21);
                    outbuffer[11] += (inbuffer[2] * l30);
                    outbuffer[11] += (inbuffer[3] * l31);
                    outbuffer[12] += (inbuffer[2] * l40);
                    outbuffer[12] += (inbuffer[3] * l41);
                    outbuffer[13] += (inbuffer[2] * l50);
                    outbuffer[13] += (inbuffer[3] * l51);
                    outbuffer[14] += (inbuffer[2] * l60);
                    outbuffer[14] += (inbuffer[3] * l61);
                    outbuffer[15] += (inbuffer[2] * l70);
                    outbuffer[15] += (inbuffer[3] * l71);

                    inbuffer  += 4;
                    outbuffer += 16;
                    len       --;
                }
#endif            

                len = length & 1;
                while (len)
                {
                    outbuffer[0] += (inbuffer[0] * l00);
                    outbuffer[0] += (inbuffer[1] * l01);
                    outbuffer[1] += (inbuffer[0] * l10);
                    outbuffer[1] += (inbuffer[1] * l11);
                    outbuffer[2] += (inbuffer[0] * l20);
                    outbuffer[2] += (inbuffer[1] * l21);
                    outbuffer[3] += (inbuffer[0] * l30);
                    outbuffer[3] += (inbuffer[1] * l31);
                    outbuffer[4] += (inbuffer[0] * l40);
                    outbuffer[4] += (inbuffer[1] * l41);
                    outbuffer[5] += (inbuffer[0] * l50);
                    outbuffer[5] += (inbuffer[1] * l51);
                    outbuffer[6] += (inbuffer[0] * l60);
                    outbuffer[6] += (inbuffer[1] * l61);
                    outbuffer[7] += (inbuffer[0] * l70);
                    outbuffer[7] += (inbuffer[1] * l71);

                    inbuffer  += 2;
                    outbuffer += 8;
                    len       --;
                }
            }
        }
        else if (inchannels == 8)
        {
            if (checkUnity(outchannels, inchannels) == FMOD_OK)
            {
                float l[8];

                l[0] = l00;
                l[1] = l11;
                l[2] = l22;
                l[3] = l33;
                l[4] = l44;
                l[5] = l55;
                l[6] = l66;
                l[7] = l77;

                #ifdef FMOD_SUPPORT_SIMD
                if (supportssimd)
                {
                    float volume0to3[4] = { l[0], l[1], l[2], l[3] };
                    float volume4to7[4] = { l[4], l[5], l[6], l[7] };

                    FMOD_DSP_Connection_Mix7_1To7_1_SIMD(inbuffer, outbuffer, length, volume0to3, volume4to7);
                }
                else
                #endif
                {
                    unsigned int len;
                             
                    len = length >> 1;
                    while (len)
                    {                   
                        outbuffer[0]  += (inbuffer[0]  * l[0]);
                        outbuffer[1]  += (inbuffer[1]  * l[1]);
                        outbuffer[2]  += (inbuffer[2]  * l[2]);
                        outbuffer[3]  += (inbuffer[3]  * l[3]);
                        outbuffer[4]  += (inbuffer[4]  * l[4]);
                        outbuffer[5]  += (inbuffer[5]  * l[5]);
                        outbuffer[6]  += (inbuffer[6]  * l[6]);
                        outbuffer[7]  += (inbuffer[7]  * l[7]);

                        outbuffer[8]  += (inbuffer[8]  * l[0]);
                        outbuffer[9]  += (inbuffer[9]  * l[1]);
                        outbuffer[10] += (inbuffer[10] * l[2]);
                        outbuffer[11] += (inbuffer[11] * l[3]);
                        outbuffer[12] += (inbuffer[12] * l[4]);
                        outbuffer[13] += (inbuffer[13] * l[5]);
                        outbuffer[14] += (inbuffer[14] * l[6]);
                        outbuffer[15] += (inbuffer[15] * l[7]);
                       
                        outbuffer += 16;
                        inbuffer += 16;
                        len--;
                    }
                    
                    len = length & 1;
                    if (len)
                    {
                        while (len)
                        {
                            outbuffer[0]  += (inbuffer[0] * l[0]);
                            outbuffer[1]  += (inbuffer[1] * l[1]);
                            outbuffer[2]  += (inbuffer[2] * l[2]);
                            outbuffer[3]  += (inbuffer[3] * l[3]);
                            outbuffer[4]  += (inbuffer[4] * l[4]);
                            outbuffer[5]  += (inbuffer[5] * l[5]);
                            outbuffer[6]  += (inbuffer[6] * l[6]);
                            outbuffer[7]  += (inbuffer[7] * l[7]);

                            inbuffer  +=8;
                            outbuffer +=8;
                            len       --;
                        }
                    }
                }
            }
            else
            {
                for (count = 0; count < length; count++)
                {
                    int count2;
                    
                    for (count2 = 0; count2 < outchannels; count2++)
                    {
                        float srcval  = 0;
                        float destval = *outbuffer;
                        int count3;

                        for (count3 = 0; count3 < inchannels; count3++)
                        {
                            srcval += (inbuffer[count3] * mLevelCurrent[count2][count3]);
                        }

                        destval += srcval;
                        *outbuffer++ = destval;
                    }
                    inbuffer += inchannels;
                }
            }
        }
    }
#endif    
    else
    {
        for (count = 0; count < length; count++)
        {
            int count2;
            
            for (count2 = 0; count2 < outchannels; count2++)
            {
                float srcval  = 0;
                float destval = *outbuffer;
                int count3;
 
                for (count3 = 0; count3 < inchannels; count3++)
                {
                    srcval += (inbuffer[count3] * mLevelCurrent[count2][count3]);
                }

                destval += srcval;
                *outbuffer++ = destval;
            }
            inbuffer += inchannels;
        }
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
// VERIFY: bc3a7e5c manual
// OFFSET: 0x009231B0
FMOD_RESULT DSPConnectionI::mixAndRamp(float * FMOD_RESTRICT outbuffer, float * FMOD_RESTRICT inbuffer, int outchannels, int inchannels, unsigned int length)
{
    unsigned int count;
    #if (DSP_LEVEL_PRECISION == 16)
    float        levelcurrent[DSP_MAXLEVELS_OUT][DSP_MAXLEVELS_IN];
    float        leveldelta[DSP_MAXLEVELS_OUT][DSP_MAXLEVELS_IN];

    for (count = 0; count < (unsigned int)outchannels; count++)
    {
        int count2;

        for (count2 = 0; count2 < inchannels; count2++)
        {
            levelcurrent[count][count2] = DSP_LEVEL_DECOMPRESS(mLevelCurrent[count][count2]);
            leveldelta[count][count2] = DSP_LEVEL_DECOMPRESS(mLevelDelta[count][count2]);
        }
    }
    #endif

    for (count = 0; count < length; count++)
    {
        int count2;
        
        for (count2 = 0; count2 < outchannels; count2++)
        {
            float srcval  = 0;
            float destval = *outbuffer;
            int count3;

            for (count3 = 0; count3 < inchannels; count3++)
            {
                #if (DSP_LEVEL_PRECISION == 32)
                srcval += (inbuffer[count3] * mLevelCurrent[count2][count3]);
                mLevelCurrent[count2][count3] += mLevelDelta[count2][count3];
                #else
                srcval += (inbuffer[count3] * levelcurrent[count2][count3]);
                levelcurrent[count2][count3] += leveldelta[count2][count3];
                #endif
            }

            destval += srcval;
            *outbuffer++ = destval;
        }

        inbuffer += inchannels;
    }

    #if (DSP_LEVEL_PRECISION == 16)
    for (count = 0; count < (unsigned int)outchannels; count++)
    {
        int count2;

        for (count2 = 0; count2 < inchannels; count2++)
        {
            mLevelCurrent[count][count2] = DSP_LEVEL_COMPRESS(levelcurrent[count][count2]);
            mLevelDelta[count][count2] = DSP_LEVEL_COMPRESS(leveldelta[count][count2]);
        }
    }
    #endif

    mRampCount -= length;

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
// VERIFY: 1d1c307a manual
// OFFSET: 0x00923300
FMOD_RESULT DSPConnectionI::rampTo()
{
    int   count;
    float oorampcount, delta = 0;

    oorampcount = 1.0f / DSP_RAMPCOUNT;

    if (mMaxInputLevels == 6)
    {
        for (count = 0; count < mMaxOutputLevels; count++)
        {
            float value; 
            
            value = ((DSP_LEVEL_DECOMPRESS(mLevel[count][0]) * mVolume) - DSP_LEVEL_DECOMPRESS(mLevelCurrent[count][0])) * oorampcount;
            delta += FMOD_FABS(value);
            mLevelDelta[count][0] = DSP_LEVEL_COMPRESS(value);
            value = ((DSP_LEVEL_DECOMPRESS(mLevel[count][1]) * mVolume) - DSP_LEVEL_DECOMPRESS(mLevelCurrent[count][1])) * oorampcount;
            delta += FMOD_FABS(value);
            mLevelDelta[count][1] = DSP_LEVEL_COMPRESS(value);
            value = ((DSP_LEVEL_DECOMPRESS(mLevel[count][2]) * mVolume) - DSP_LEVEL_DECOMPRESS(mLevelCurrent[count][2])) * oorampcount;
            delta += FMOD_FABS(value);
            mLevelDelta[count][2] = DSP_LEVEL_COMPRESS(value);
            value = ((DSP_LEVEL_DECOMPRESS(mLevel[count][3]) * mVolume) - DSP_LEVEL_DECOMPRESS(mLevelCurrent[count][3])) * oorampcount;
            delta += FMOD_FABS(value);
            mLevelDelta[count][3] = DSP_LEVEL_COMPRESS(value);
            value = ((DSP_LEVEL_DECOMPRESS(mLevel[count][4]) * mVolume) - DSP_LEVEL_DECOMPRESS(mLevelCurrent[count][4])) * oorampcount;
            delta += FMOD_FABS(value);
            mLevelDelta[count][4] = DSP_LEVEL_COMPRESS(value);
            value = ((DSP_LEVEL_DECOMPRESS(mLevel[count][5]) * mVolume) - DSP_LEVEL_DECOMPRESS(mLevelCurrent[count][5])) * oorampcount;
            delta += FMOD_FABS(value);
            mLevelDelta[count][5] = DSP_LEVEL_COMPRESS(value);
        }
    }
    else
    {
        int count2;
        
        for (count = 0; count < mMaxOutputLevels; count++)
        {
            for (count2 = 0; count2 < mMaxInputLevels; count2++)
            {
                float value; 

                value = ((DSP_LEVEL_DECOMPRESS(mLevel[count][count2]) *mVolume) - DSP_LEVEL_DECOMPRESS(mLevelCurrent[count][count2])) * oorampcount;
                delta += FMOD_FABS(value);
                mLevelDelta[count][count2] = DSP_LEVEL_COMPRESS(value);
            }
        }
    }

    if (DSP_LEVEL_COMPRESS(delta) > 0.000001f)
    {
        mRampCount = DSP_RAMPCOUNT;     /* Only trigger the ramp if anything has changed. */
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
// VERIFY: 4582eab4 auto
// OFFSET: 0x00923470
FMOD_RESULT DSPConnectionI::checkUnity(int outchannels, int inchannels)
{
    int count, count2;

    if (mVolume != 1.0f || inchannels != outchannels)
    {
        return FMOD_ERR_PAN;
    }

    for (count = 0; count < outchannels; count++)
    {
        for (count2 = 0; count2 < inchannels; count2++)
        {
            if (count == count2 && DSP_LEVEL_DECOMPRESS(mLevel[count][count2]) != 1.0f)
            {
                return FMOD_ERR_PAN;
            }
            else if (count != count2 && mLevel[count][count2] != 0)
            {
                return FMOD_ERR_PAN;
            }
        }
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
FMOD_RESULT DSPConnectionI::checkMono(int outchannels, int inchannels)
{
    int count, count2;

    if (inchannels == 1)
    {
        return FMOD_OK;
    }

    for (count = 0; count < outchannels; count++)
    {
        for (count2 = 1; count2 < inchannels; count2++)
        {
            if (mLevel[count][count2] != 0)
            {
                return FMOD_ERR_TOOMANYCHANNELS;
            }
        }
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
// VERIFY: 9ee92220 manual
// OFFSET: 0x00923500
FMOD_RESULT DSPConnectionI::setPan(float pan, int outchannels, int inchannels, FMOD_SPEAKERMODE speakermode)
{
    int count, count2;

#ifndef PLATFORM_PS3_SPU
    if (!mInputUnit->mSystem)
    {
        return FMOD_ERR_INTERNAL;
    }
#endif

    /*
        First clear out all levels to 0.
    */
    for (count = 0; count < outchannels; count++)
    {
        for (count2 = 0; count2 < inchannels; count2++)
        {
            mLevel[count][count2] = 0;
        }
    }    

    switch (speakermode)
    {
#ifdef FMOD_SUPPORT_SPEAKERMODE_RAW
        case FMOD_SPEAKERMODE_RAW:
        {
            /*
                Map 1 input channel to each output channel sequentially.  For example.

                1 0 0 0 0 0 0 0 or 1 0 0 0 or 1 0 or 1 or 1 0 0 0 0 0 0 0  or 1 0 0 0 or 1 0  or 1
                0 1 0 0 0 0 0 0    0 1 0 0    0 1    0    0 1 0 0 0 0 0 0     0 1 0 0    0 1     0
                0 0 1 0 0 0 0 0    0 0 1 0    0 0    0
                0 0 0 1 0 0 0 0    0 0 0 1    0 0    0 
                0 0 0 0 1 0 0 0    0 0 0 0    0 0    0
                0 0 0 0 0 1 0 0    0 0 0 0    0 0    0
                0 0 0 0 0 0 1 0    0 0 0 0    0 0    0
                0 0 0 0 0 0 0 1    0 0 0 0    0 0    0
            */
            for (count = 0; count < outchannels; count++)
            {
                for (count2 = 0; count2 < inchannels; count2++)
                {
                    if (count == count2)
                    {
                        mLevel[count][count2] = DSP_LEVEL_COMPRESS(1.0f);
                    }
                }
            }
            break;
        }
#endif
#ifdef FMOD_SUPPORT_SPEAKERMODE_MONO
        case FMOD_SPEAKERMODE_MONO:
        {
            for (count = 0; count < inchannels; count++)
            {
                mLevel[0][count] = DSP_LEVEL_COMPRESS(1.0f);
            }
            break;
        }
#endif
#ifdef FMOD_SUPPORT_SPEAKERMODE_STEREO
        case FMOD_SPEAKERMODE_STEREO:
        case FMOD_SPEAKERMODE_STEREO_LINEAR:
        {
            float l,r;
                
            pan = (pan + 1.0f) / 2.0f;

            if (inchannels == 1)
            {  
                l = 1.0f - pan;
                r = pan;

                if (speakermode == FMOD_SPEAKERMODE_STEREO)
                {
                    l = FMOD_SQRT(l);
                    r = FMOD_SQRT(r);
                }

                mLevel[0][0] = DSP_LEVEL_COMPRESS(l);
                mLevel[1][0] = DSP_LEVEL_COMPRESS(r);
            }
            else if (inchannels == 2 || inchannels == 4 || inchannels == 6 || inchannels == 8)
            {   
                /*
                    Stereo source panning is 0 = 1.0:0.0.  0.5 = 1.0:1.0.  1.0 = 0:1.0.
                */
                if (pan <= 0.5f)
                {
                    l = 1.0f;
                    r = pan * 2.0f;
                }
                else
                {
                    l = (1.0f - pan) * 2.0f;
                    r = 1.0f;
                }
    
                if (inchannels == 2)
                {
                    mLevel[0][0] = DSP_LEVEL_COMPRESS(l);
                    mLevel[1][1] = DSP_LEVEL_COMPRESS(r);
                }
                else if (inchannels == 4)
                {
                    /*
                        Prologic downmix.  Assuming LCRS
                    */
                    mLevel[0][0] = DSP_LEVEL_COMPRESS(  1.0f   * l);
                    mLevel[0][1] = DSP_LEVEL_COMPRESS(  0.0f   * l);
                    mLevel[0][2] = DSP_LEVEL_COMPRESS(  0.707f * l);
                    mLevel[0][3] = DSP_LEVEL_COMPRESS( -0.707f * l);     /* Discard LFE */

                    mLevel[1][0] = DSP_LEVEL_COMPRESS(  0.0f   * r);
                    mLevel[1][1] = DSP_LEVEL_COMPRESS(  1.0f   * r);
                    mLevel[1][2] = DSP_LEVEL_COMPRESS(  0.707f * r);
                    mLevel[1][3] = DSP_LEVEL_COMPRESS( -0.707f * r);     /* Discard LFE */
                }
                else if (inchannels == 6)
                {
                    /*
                        Prologic 2 downmix.
                    */
                    mLevel[0][0] = DSP_LEVEL_COMPRESS( 1.0f   * l);
                    mLevel[0][1] = DSP_LEVEL_COMPRESS( 0.0f   * l);
                    mLevel[0][2] = DSP_LEVEL_COMPRESS( 0.707f * l);
                    mLevel[0][3] = DSP_LEVEL_COMPRESS( 0.0f   * l);     /* Discard LFE */
                    mLevel[0][4] = DSP_LEVEL_COMPRESS(-0.872f * l);
                    mLevel[0][5] = DSP_LEVEL_COMPRESS(-0.49f  * l);

                    mLevel[1][0] = DSP_LEVEL_COMPRESS( 0.0f   * r);
                    mLevel[1][1] = DSP_LEVEL_COMPRESS( 1.0f   * r);
                    mLevel[1][2] = DSP_LEVEL_COMPRESS( 0.707f * r);
                    mLevel[1][3] = DSP_LEVEL_COMPRESS( 0.0f   * r);     /* Discard LFE */
                    mLevel[1][4] = DSP_LEVEL_COMPRESS( 0.49f  * r);
                    mLevel[1][5] = DSP_LEVEL_COMPRESS( 0.872f * r);
                }
                else if (inchannels == 8)
                {
                    /*
                        Prologic 2x downmix?
                    */
                    mLevel[0][0] = DSP_LEVEL_COMPRESS( 1.0f   * l);
                    mLevel[0][1] = DSP_LEVEL_COMPRESS( 0.0f   * l);
                    mLevel[0][2] = DSP_LEVEL_COMPRESS( 0.707f * l);
                    mLevel[0][3] = DSP_LEVEL_COMPRESS( 0.0f   * l);     /* Discard LFE */
                    mLevel[0][4] = DSP_LEVEL_COMPRESS(-0.872f * l);
                    mLevel[0][5] = DSP_LEVEL_COMPRESS(-0.49f  * l);

                    mLevel[0][6] = DSP_LEVEL_COMPRESS( 1.0f   * l);     /* Not sure here.  Need PLIIx coefficients for 7.1? */
                    mLevel[0][7] = DSP_LEVEL_COMPRESS( 0.0f   * l);     /* Not sure here.  Need PLIIx coefficients for 7.1? */

                    mLevel[1][0] = DSP_LEVEL_COMPRESS( 0.0f   * r);
                    mLevel[1][1] = DSP_LEVEL_COMPRESS( 1.0f   * r);
                    mLevel[1][2] = DSP_LEVEL_COMPRESS( 0.707f * r);
                    mLevel[1][3] = DSP_LEVEL_COMPRESS( 0.0f   * r);     /* Discard LFE */
                    mLevel[1][4] = DSP_LEVEL_COMPRESS( 0.49f  * r);
                    mLevel[1][5] = DSP_LEVEL_COMPRESS( 0.872f * r);

                    mLevel[1][6] = DSP_LEVEL_COMPRESS( 0.0f   * r);     /* Not sure here.  Need PLIIx coefficients for 7.1? */
                    mLevel[1][7] = DSP_LEVEL_COMPRESS( 1.0f   * r);     /* Not sure here.  Need PLIIx coefficients for 7.1? */
                }
            }
            else
            {
                /*
                    Map 1 input channel to each output channel sequentially.  For example.

                    1 0 0 0 0 0 0 0 or 1 0 0 0 or 1 0 or 1 or 1 0 0 0 0 0 0 0  or 1 0 0 0 or 1 0  or 1
                    0 1 0 0 0 0 0 0    0 1 0 0    0 1    0    0 1 0 0 0 0 0 0     0 1 0 0    0 1     0
                    0 0 1 0 0 0 0 0    0 0 1 0    0 0    0
                    0 0 0 1 0 0 0 0    0 0 0 1    0 0    0 
                    0 0 0 0 1 0 0 0    0 0 0 0    0 0    0
                    0 0 0 0 0 1 0 0    0 0 0 0    0 0    0
                    0 0 0 0 0 0 1 0    0 0 0 0    0 0    0
                    0 0 0 0 0 0 0 1    0 0 0 0    0 0    0
                */
                for (count = 0; count < outchannels; count++)
                {
                    for (count2 = 0; count2 < inchannels; count2++)
                    {
                        if (count == count2)
                        {
                            mLevel[count][count2] = DSP_LEVEL_COMPRESS(1.0f);
                        }
                    }
                }
            }
            break;
        }
#endif
        case FMOD_SPEAKERMODE_QUAD:
        case FMOD_SPEAKERMODE_SURROUND:
        case FMOD_SPEAKERMODE_5POINT1:
        case FMOD_SPEAKERMODE_7POINT1:
        {
            float l,r;
            
            pan = (pan + 1.0f) / 2.0f;

            if (inchannels == 1)
            {  
                l = 1.0f - pan;
                r = pan;

                l = FMOD_SQRT(l);
                r = FMOD_SQRT(r);

                mLevel[0][0] = DSP_LEVEL_COMPRESS(l);
                mLevel[1][0] = DSP_LEVEL_COMPRESS(r);
            }
            else if (inchannels == 2)
            {   
                /*
                    Stereo source panning is 0.0 = 1.0:0.0.  0.5 = 1.0:1.0.  1.0 = 0.0:1.0.
                */
                if (pan <= 0.5f)
                {
                    l = 1.0f;
                    r = pan * 2.0f;
                }
                else
                {
                    l = (1.0f - pan) * 2.0f;
                    r = 1.0f;
                }

                mLevel[0][0] = DSP_LEVEL_COMPRESS(l);
                mLevel[1][1] = DSP_LEVEL_COMPRESS(r);
            }
            else
            {
                /*
                    Map 1 input channel to each output channel sequentially.  For example.

                    1 0 0 0 0 0 0 0 or 1 0 0 0 or 1 0 or 1 or 1 0 0 0 0 0 0 0  or 1 0 0 0 or 1 0  or 1
                    0 1 0 0 0 0 0 0    0 1 0 0    0 1    0    0 1 0 0 0 0 0 0     0 1 0 0    0 1     0
                    0 0 1 0 0 0 0 0    0 0 1 0    0 0    0
                    0 0 0 1 0 0 0 0    0 0 0 1    0 0    0 
                    0 0 0 0 1 0 0 0    0 0 0 0    0 0    0
                    0 0 0 0 0 1 0 0    0 0 0 0    0 0    0
                    0 0 0 0 0 0 1 0    0 0 0 0    0 0    0
                    0 0 0 0 0 0 0 1    0 0 0 0    0 0    0
                */
                for (count = 0; count < outchannels; count++)
                {
                    for (count2 = 0; count2 < inchannels; count2++)
                    {
                        if (count == count2)
                        {
                            mLevel[count][count2] = DSP_LEVEL_COMPRESS(1.0f);
                        }
                    }
                }
            }
            break;
        }
#ifdef FMOD_SUPPORT_SPEAKERMODE_PROLOGIC
        case FMOD_SPEAKERMODE_PROLOGIC:
        {
            float l,r;
            
            pan = (pan + 1.0f) / 2.0f;

            if (inchannels == 1)
            {  
                l = 1.0f - pan;
                r = pan;

                l = FMOD_SQRT(l);
                r = FMOD_SQRT(r);

                mLevel[0][0] = DSP_LEVEL_COMPRESS(l);
                mLevel[1][0] = DSP_LEVEL_COMPRESS(r);
            }
            else if (inchannels == 2)
            {   
                /*
                    Stereo source panning is 0.0 = 1.0:0.0.  0.5 = 1.0:1.0.  1.0 = 0:1.0.
                */
                if (pan < 0.5f)
                {
                    l = 1.0f;
                    r = pan * 2.0f;
                }
                else
                {
                    l = (1.0f - pan) * 2.0f;
                    r = 1.0f;
                }
    
                mLevel[0][0] = DSP_LEVEL_COMPRESS(l);
                mLevel[1][1] = DSP_LEVEL_COMPRESS(r);
            }
            else
            { 
                /*
                    Map 1 input channel to each output channel sequentially.  For example.

                    1 0 0 0 0 0 0 0 or 1 0 0 0 or 1 0 or 1 or 1 0 0 0 0 0 0 0  or 1 0 0 0 or 1 0  or 1
                    0 1 0 0 0 0 0 0    0 1 0 0    0 1    0    0 1 0 0 0 0 0 0     0 1 0 0    0 1     0
                    0 0 1 0 0 0 0 0    0 0 1 0    0 0    0
                    0 0 0 1 0 0 0 0    0 0 0 1    0 0    0 
                    0 0 0 0 1 0 0 0    0 0 0 0    0 0    0
                    0 0 0 0 0 1 0 0    0 0 0 0    0 0    0
                    0 0 0 0 0 0 1 0    0 0 0 0    0 0    0
                    0 0 0 0 0 0 0 1    0 0 0 0    0 0    0
                */
                for (count = 0; count < outchannels; count++)
                {
                    for (count2 = 0; count2 < inchannels; count2++)
                    {
                        if (count == count2)
                        {
                            mLevel[count][count2] = DSP_LEVEL_COMPRESS(1.0f);
                        }
                    }
                }
            }
            break;
        }
#endif
        default:
        {
            break;
        }
    };

    mSetLevelsUsed = true;

    return rampTo();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
1	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 01217287 manual
// OFFSET: 0x00923A70
FMOD_RESULT DSPConnectionI::setLevels(float *levels, int numinputlevels)
{
    int count, count2;

    if (!levels)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!numinputlevels)
    {
        return FMOD_OK;
    }

#if (DSP_MAXLEVELS_OUT > 2)
    if (mMaxOutputLevels == 6 && numinputlevels < 3)
    {
        if (numinputlevels == 1)
        {
            mLevel[0][0] = DSP_LEVEL_COMPRESS(levels[0]);
            mLevel[1][0] = DSP_LEVEL_COMPRESS(levels[1]);
            mLevel[2][0] = DSP_LEVEL_COMPRESS(levels[2]);
            mLevel[3][0] = DSP_LEVEL_COMPRESS(levels[3]);
            mLevel[4][0] = DSP_LEVEL_COMPRESS(levels[4]);
            mLevel[5][0] = DSP_LEVEL_COMPRESS(levels[5]);
        }
        else
        {
            mLevel[0][0] = DSP_LEVEL_COMPRESS(levels[0]);
            mLevel[0][1] = DSP_LEVEL_COMPRESS(levels[1]);
            mLevel[1][0] = DSP_LEVEL_COMPRESS(levels[2]);
            mLevel[1][1] = DSP_LEVEL_COMPRESS(levels[3]);
            mLevel[2][0] = DSP_LEVEL_COMPRESS(levels[4]);
            mLevel[2][1] = DSP_LEVEL_COMPRESS(levels[5]);
            mLevel[3][0] = DSP_LEVEL_COMPRESS(levels[6]);
            mLevel[3][1] = DSP_LEVEL_COMPRESS(levels[7]);
            mLevel[4][0] = DSP_LEVEL_COMPRESS(levels[8]);
            mLevel[4][1] = DSP_LEVEL_COMPRESS(levels[9]);
            mLevel[5][0] = DSP_LEVEL_COMPRESS(levels[10]);
            mLevel[5][1] = DSP_LEVEL_COMPRESS(levels[11]);            
        }
    }
    else
#endif
    {
        for (count = 0; count < mMaxOutputLevels; count++)
        {
            for (count2 = 0; count2 < mMaxInputLevels; count2++)
            {
                if (count2 < numinputlevels)
                {
                    mLevel[count][count2] = DSP_LEVEL_COMPRESS(levels[(count * numinputlevels) + count2]);
                }
                else
                {
                    mLevel[count][count2] = 0;
                }
            }
        }
    }

    mSetLevelsUsed = true;

    return rampTo();
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
// VERIFY: 5a43afdb auto
// OFFSET: 0x00923BC0
FMOD_RESULT DSPConnectionI::getLevels(float *levels, int numinputlevels)
{
    int count, count2;

    if (!levels)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    for (count = 0; count < mMaxOutputLevels; count++)
    {
        for (count2 = 0; count2 < numinputlevels; count2++)
        {
            if (count2 < mMaxInputLevels)
            {
                levels[(count * numinputlevels) + count2] = DSP_LEVEL_DECOMPRESS(mLevel[count][count2]);
            }
            else
            {
                levels[(count * numinputlevels) + count2] = 0;
            }
        }
    }

    return FMOD_OK;
}

#endif /* #ifdef FMOD_SUPPORT_SOFTWARE */


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
// VERIFY: e19d1c73 auto
// OFFSET: 0x00923A20
FMOD_RESULT DSPConnectionI::setMix(float volume)
{
#ifdef FMOD_SUPPORT_SOFTWARE
    if (volume < -1.0f)
    {
        volume = -1.0f;
    }
    if (volume > 1.0f)
    {
        volume = 1.0f;
    }

    if (mVolume == volume)
    {
        return FMOD_OK;
    }

    mVolume = volume;

    return rampTo();
#else
    return FMOD_ERR_NEEDSSOFTWARE;
#endif
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
FMOD_RESULT DSPConnectionI::getMix(float *volume)
{
#ifdef FMOD_SUPPORT_SOFTWARE
    if (!volume)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *volume = mVolume;

    return FMOD_OK;
#else
    return FMOD_ERR_NEEDSSOFTWARE;
#endif
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
FMOD_RESULT DSPConnectionI::setLevels(FMOD_SPEAKER speaker, float *levels, int numinputlevels)
{
#ifdef FMOD_SUPPORT_SOFTWARE
    int count;

    if (!levels)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!numinputlevels)
    {
        return FMOD_OK;
    }

    if (speaker >= mMaxOutputLevels)
    {
        return FMOD_ERR_INVALID_SPEAKER;
    }

    for (count = 0; count < mMaxInputLevels; count++)
    {
        if (count < numinputlevels)
        {
            mLevel[speaker][count] = DSP_LEVEL_COMPRESS(levels[count]);
        }
        else
        {
            mLevel[speaker][count] = 0;
        }
    }

    mSetLevelsUsed = true;

    return rampTo();
#else
    return FMOD_ERR_NEEDSSOFTWARE;
#endif
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
FMOD_RESULT DSPConnectionI::getLevels(FMOD_SPEAKER speaker, float *levels, int numinputlevels)
{
#ifdef FMOD_SUPPORT_SOFTWARE
    int count;

    if (!levels)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    for (count = 0; count < numinputlevels; count++)
    {
        if (count < mMaxInputLevels)
        {
            levels[count] = DSP_LEVEL_DECOMPRESS(mLevel[speaker][count]);
        }
        else
        {
            levels[count] = 0;
        }
    }

    return FMOD_OK;
#else
    return FMOD_ERR_NEEDSSOFTWARE;
#endif
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
#ifdef FMOD_SUPPORT_SOFTWARE
// VERIFY: faf33a20 manual
// OFFSET: 0x00923C50
FMOD_RESULT DSPConnectionI::copy(DSPConnectionI *source)
{
    int count, count2;
    
    mVolume = source->mVolume;

    for (count = 0; count < mMaxOutputLevels; count++)
    {
        for (count2 = 0; count2 < mMaxInputLevels; count2++)
        {
            mLevel       [count][count2] = source->mLevel       [count][count2];
            mLevelCurrent[count][count2] = source->mLevelCurrent[count][count2];
            mLevelDelta  [count][count2] = source->mLevelDelta  [count][count2];
        }
    }
    
    mRampCount     = source->mRampCount;
    mSetLevelsUsed = source->mSetLevelsUsed;
    
    return FMOD_OK;
}
#endif



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


#if (DSP_LEVEL_PRECISION == 16) && !defined(PLATFORM_XENON)
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
unsigned short FMOD_DSP_ConnectionI_Single2HalfP(float source)
{
    typedef union
    {
        float f;
        unsigned int i;
    } floatdata;

    floatdata        x;
    unsigned short   hs, he, hm;
    unsigned int     xs, xe, xm;
    int hes; 
    
    x.f = source;

    if( (x.i & 0x7FFFFFFFu) == 0 ) 
    {  // Signed zero
        return (unsigned short) (x.i >> 16);  // Return the signed zero
    } 
    else 
    { // Not zero
        xs = x.i & 0x80000000u;  // Pick off sign bit
        xe = x.i & 0x7F800000u;  // Pick off exponent bits
        xm = x.i & 0x007FFFFFu;  // Pick off mantissa bits
        if( xe == 0 ) 
        {  // Denormal will underflow, return a signed zero
            return (unsigned short) (xs >> 16);
        } else if( xe == 0x7F800000u ) 
        {  // Inf or NaN (all the exponent bits are set)
            if( xm == 0 ) 
            { // If mantissa is zero ...
                return (unsigned short) ((xs >> 16) | 0x7C00u); // Signed Inf
            } else 
            {
                return (unsigned short) 0xFE00u; // NaN, only 1st mantissa bit set
            }
        } else 
        { // Normalized number
            hs = (unsigned short) (xs >> 16); // Sign bit
            hes = ((int)(xe >> 23)) - 127 + 15; // Exponent unbias the single, then bias the halfp
            if( hes >= 0x1F ) 
            {  // Overflow
                return (unsigned short) ((xs >> 16) | 0x7C00u); // Signed Inf
            } else if( hes <= 0 ) 
            {  // Underflow
                if( (14 - hes) > 24 ) 
                {  // Mantissa shifted all the way off & no rounding possibility
                    hm = (unsigned short) 0u;  // Set mantissa to zero
                } 
                else 
                {
                    xm |= 0x00800000u;  // Add the hidden leading bit
                    hm = (unsigned short) (xm >> (14 - hes)); // Mantissa
                    if( (xm >> (13 - hes)) & 0x00000001u ) // Check for rounding
                        hm += (unsigned short) 1u; // Round, might overflow into exp bit, but this is OK
                }
                return (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
            } else 
            {
                he = (unsigned short) (hes << 10); // Exponent
                hm = (unsigned short) (xm >> 13); // Mantissa
                if( xm & 0x00001000u ) // Check for rounding
                    return (hs | he | hm) + (unsigned short) 1u; // Round, might overflow to inf, this is OK
                else
                    return (hs | he | hm);  // No rounding
            }
        }
    }
    
    return 0;
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
float FMOD_DSP_ConnectionI_HalfP2Single(unsigned short h)
{
    typedef union
    {
        float f;
        unsigned int i;
    } floatdata;

    unsigned short hs, he, hm;
    unsigned int xs, xe, xm;
    signed int xes;
    floatdata xp;
    int e;

    if( (h & 0x7FFFu) == 0 ) 
    {  // Signed zero
        xp.i = ((unsigned int) h) << 16;  // Return the signed zero
    } 
    else 
    { // Not zero
        hs = h & 0x8000u;  // Pick off sign bit
        he = h & 0x7C00u;  // Pick off exponent bits
        hm = h & 0x03FFu;  // Pick off mantissa bits
        if( he == 0 ) 
        {  // Denormal will convert to normalized
            e = -1; // The following loop figures out how much extra to adjust the exponent
            do 
            {
                e++;
                hm <<= 1;
            } while( (hm & 0x0400u) == 0 ); // Shift until leading bit overflows into exponent bit
            xs = ((unsigned int) hs) << 16; // Sign bit
            xes = ((signed int) (he >> 10)) - 15 + 127 - e; // Exponent unbias the halfp, then bias the single
            xe = (unsigned int) (xes << 23); // Exponent
            xm = ((unsigned int) (hm & 0x03FFu)) << 13; // Mantissa
            xp.i = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
        } 
        else if( he == 0x7C00u ) 
        {  // Inf or NaN (all the exponent bits are set)
            if( hm == 0 ) 
            { // If mantissa is zero ...
                xp.i = (((unsigned int) hs) << 16) | ((unsigned int) 0x7F800000u); // Signed Inf
            } else 
            {
                xp.i = (unsigned int) 0xFFC00000u; // NaN, only 1st mantissa bit set
            }
        } 
        else 
        { // Normalized number
            xs = ((unsigned int) hs) << 16; // Sign bit
            xes = ((signed int) (he >> 10)) - 15 + 127; // Exponent unbias the halfp, then bias the single
            xe = (unsigned int) (xes << 23); // Exponent
            xm = ((unsigned int) hm) << 13; // Mantissa
            xp.i = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
        }
    }
    return xp.f;
}
#endif

}
