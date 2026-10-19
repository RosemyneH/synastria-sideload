#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_IMAADPCM

#include "fmod_codec_wav_imaadpcm.h"

namespace FMOD
{

#undef LOWORD
#undef HIWORD

#undef LOWORD
#undef HIWORD
#undef MAKEULONG
#undef MAKELONG

#define LOWORD(_l)      ((unsigned short)(_l))
#define HIWORD(_l)      ((unsigned short)(((unsigned int)(_l) >> 16) & 0xFFFF))

#ifdef PLATFORM_ENDIAN_BIG
	#define MAKEULONG(l, h) ((unsigned int)(((unsigned short)(h)) | ((unsigned int)((unsigned short)(l))) << 16))
#else
	#define MAKEULONG(l, h) ((unsigned int)(((unsigned short)(l)) | ((unsigned int)((unsigned short)(h))) << 16))
#endif

#define MAKELONG(l, h)	((int)MAKEULONG(l, h))

#define NUMELMS(a)      (sizeof(a) / sizeof(a[0]))

static const int IMAAdpcm_IndexTab[16] = 
{ 
    -1, -1, -1, -1, 2, 4, 6, 8, 
    -1, -1, -1, -1, 2, 4, 6, 8 
};

//
// This array contains the array of step sizes used to encode the ADPCM
// samples.  The step index in each ADPCM block is an index to this array.
//
static const short IMAAdpcm_StepTab[89] =
{
        7,     8,     9,    10,    11,    12,    13,
       14,    16,    17,    19,    21,    23,    25,
       28,    31,    34,    37,    41,    45,    50,
       55,    60,    66,    73,    80,    88,    97,
      107,   118,   130,   143,   157,   173,   190,
      209,   230,   253,   279,   307,   337,   371,
      408,   449,   494,   544,   598,   658,   724,
      796,   876,   963,  1060,  1166,  1282,  1411,
     1552,  1707,  1878,  2066,  2272,  2499,  2749,
     3024,  3327,  3660,  4026,  4428,  4871,  5358,
     5894,  6484,  7132,  7845,  8630,  9493, 10442,
    11487, 12635, 13899, 15289, 16818, 18500, 20350,
    22385, 24623, 27086, 29794, 32767
};


FMOD_INLINE int IMAAdpcm_NextStepIndex(int nEncodedSample, int nStepIndex)
{
    nStepIndex += IMAAdpcm_IndexTab[nEncodedSample];

    if(nStepIndex < 0)
    {
        nStepIndex = 0;
    }
    else if (nStepIndex >= (int)NUMELMS(IMAAdpcm_StepTab))
    {
        nStepIndex = NUMELMS(IMAAdpcm_StepTab) - 1;
    }

    return nStepIndex;
}

FMOD_INLINE signed char ValidStepIndex(int nStepIndex)
{
    return (nStepIndex >= 0) && (nStepIndex < (int)NUMELMS(IMAAdpcm_StepTab));
}


/****************************************************************************
 *
 *  IMAAdpcm_DecodeSample
 *
 *  Description:
 *      Decodes an encoded sample.
 *
 *  Arguments:
 *      int [in]: the sample to be decoded.
 *      int [in]: the predicted value of the sample.
 *      int [i]: the quantization step size used to encode the sample.
 *
 *  Returns:  
 *      int: the decoded PCM sample.
 *
 ****************************************************************************/
FMOD_INLINE int IMAAdpcm_DecodeSample(int nEncodedSample, int nPredictedSample, int nStepSize)
{
    int lDifference;
    int lNewSample;

    lDifference = nStepSize >> 3;

#if 1

    switch (nEncodedSample)
    {
        case 0:
        {
            break;
        }
        case 1:
        {
            lDifference += nStepSize >> 2;
            break;
        }
        case 2:
        {
            lDifference += nStepSize >> 1;
            break;
        }
        case 3:
        {
            lDifference += nStepSize >> 1;
            lDifference += nStepSize >> 2;
            break;
        }
        case 4:
        {
            lDifference += nStepSize;
            break;
        }
        case 5:
        {
            lDifference += nStepSize;
            lDifference += nStepSize >> 2;
            break;
        }
        case 6:
        {
            lDifference += nStepSize;
            lDifference += nStepSize >> 1;
            break;
        }
        case 7:
        {
            lDifference += nStepSize;
            lDifference += nStepSize >> 1;
            lDifference += nStepSize >> 2;
            break;
        }
        case 8:
        {
            lDifference = -lDifference;
            break;
        }
        case 9:
        {
            lDifference += nStepSize >> 2;
            lDifference = -lDifference;
            break;
        }
        case 10:
        {
            lDifference += nStepSize >> 1;
            lDifference = -lDifference;
            break;
        }
        case 11:
        {
            lDifference += nStepSize >> 1;
            lDifference += nStepSize >> 2;
            lDifference = -lDifference;
            break;
        }
        case 12:
        {
            lDifference += nStepSize;
            lDifference = -lDifference;
            break;
        }
        case 13:
        {
            lDifference += nStepSize;
            lDifference += nStepSize >> 2;
            lDifference = -lDifference;
            break;
        }
        case 14:
        {
            lDifference += nStepSize;
            lDifference += nStepSize >> 1;
            lDifference = -lDifference;
            break;
        }
        case 15:
        {
            lDifference += nStepSize;
            lDifference += nStepSize >> 1;
            lDifference += nStepSize >> 2;
            lDifference = -lDifference;
            break;
        }
        default:
        {
            break;
        }
    }
#else
    if(nEncodedSample & 4) 
    {
        lDifference += nStepSize;
    }

    if(nEncodedSample & 2) 
    {
        lDifference += nStepSize >> 1;
    }

    if(nEncodedSample & 1) 
    {
        lDifference += nStepSize >> 2;
    }

    if(nEncodedSample & 8)
    {
        lDifference = -lDifference;
    }
#endif

    lNewSample = nPredictedSample + lDifference;

    if((int)(short)lNewSample != lNewSample)
    {
        if(lNewSample < -32768)
        {
            lNewSample = -32768;
        }
        else
        {
            lNewSample = 32767;
        }
    }

    return (int)lNewSample;
}



// VERIFY: 8dd38d39 auto
// OFFSET: 0x009356E0
FMOD_RESULT IMAAdpcm_DecodeM16(unsigned char *pbSrc, signed short *pbDst, unsigned int cBlocks,  unsigned int nBlockAlignment, unsigned int cSamplesPerBlock, int channels)
{
    FMOD_RESULT    result = FMOD_OK;
    unsigned char *pbBlock;
    unsigned int   cSamples;
    unsigned char  bSample;
    int            nStepSize;
    int            nEncSample;
    int            nPredSample;
    int            nStepIndex;
    unsigned int   dwHeader;

    //
    // Enter the main loop
    //
    
    while(cBlocks--)
    {
        pbBlock = pbSrc;
        cSamples = cSamplesPerBlock - 1;
        
        //
        // Block header
        //

        dwHeader = *(unsigned int *)pbBlock;
#ifdef PLATFORM_ENDIAN_BIG
        dwHeader = FMOD_SWAPENDIAN_DWORD(dwHeader);
#endif
        pbBlock += sizeof(unsigned int);

        nPredSample = (int)(short)LOWORD(dwHeader);
        nStepIndex = (int)(unsigned char)HIWORD(dwHeader);

        if(!ValidStepIndex(nStepIndex))
        {
            //
            // The step index is out of range - this is considered a fatal
            // error as the input stream is corrupted.  We fail by returning
            // zero chars converted.
            //

            result = FMOD_ERR_FILE_BAD;
            break;
        }
        
        //
        // Write out first sample
        //

        *pbDst = (short)nPredSample;
        pbDst += channels;

        //
        // Enter the block loop
        //

        while(cSamples)
        {
            bSample = *pbBlock++;

            //
            // Sample 1
            //

            nEncSample  = (bSample & (unsigned char)0x0F);
            nStepSize   = IMAAdpcm_StepTab[nStepIndex];
            nPredSample = IMAAdpcm_DecodeSample(nEncSample, nPredSample, nStepSize);
            nStepIndex  = IMAAdpcm_NextStepIndex(nEncSample, nStepIndex);

            *pbDst = (short)nPredSample;
            pbDst += channels;

            cSamples--;

            //
            // Sample 2
            //

            if(cSamples)
            {
                nEncSample  = (bSample >> 4);
                nStepSize   = IMAAdpcm_StepTab[nStepIndex];
                nPredSample = IMAAdpcm_DecodeSample(nEncSample, nPredSample, nStepSize);
                nStepIndex  = IMAAdpcm_NextStepIndex(nEncSample, nStepIndex);

                *pbDst = (short)nPredSample;
                pbDst += channels;

                cSamples--;
            }
        }

        //
        // Skip padding
        //

        pbSrc += nBlockAlignment;
    }

    return result;
}


// VERIFY: bd3bbc1d manual
// OFFSET: 0x00935A60
FMOD_RESULT IMAAdpcm_DecodeS16(unsigned char *pbSrc, signed short *pbDst, unsigned int cBlocks, unsigned int nBlockAlignment, unsigned int cSamplesPerBlock)
{
    FMOD_RESULT    result = FMOD_OK;
    unsigned char *pbBlock;
    unsigned int   cSamples;
    unsigned int   cSubSamples;
    int            nStepSize;
    unsigned int   dwHeader;
    unsigned int   dwLeft;
    unsigned int   dwRight;
    int            nEncSampleL;
    int            nPredSampleL;
    int            nStepIndexL;
    int            nEncSampleR;
    int            nPredSampleR;
    int            nStepIndexR;
    unsigned int   i;

    //
    // Enter the main loop
    //
    
    while(cBlocks--)
    {
        pbBlock = pbSrc;
        cSamples = cSamplesPerBlock - 1;

        //
        // LEFT channel header
        //

        dwHeader = *(unsigned int *)pbBlock;
#ifdef PLATFORM_ENDIAN_BIG
        dwHeader = FMOD_SWAPENDIAN_DWORD(dwHeader);
#endif
        pbBlock += sizeof(unsigned int);
        
        nPredSampleL = (int)(short)LOWORD(dwHeader);
        nStepIndexL = (int)(unsigned char)HIWORD(dwHeader);

        if(!ValidStepIndex(nStepIndexL)) 
        {
            //
            // The step index is out of range - this is considered a fatal
            // error as the input stream is corrupted.  We fail by returning
            // zero bytes converted.
            //

            result = FMOD_ERR_FILE_BAD;
            break;
        }
        
        //
        // RIGHT channel header
        //

        dwHeader = *(unsigned int *)pbBlock;
#ifdef PLATFORM_ENDIAN_BIG
        dwHeader = FMOD_SWAPENDIAN_DWORD(dwHeader);
#endif
        pbBlock += sizeof(unsigned int);
        
        nPredSampleR = (int)(short)LOWORD(dwHeader);
        nStepIndexR = (int)(unsigned char)HIWORD(dwHeader);

        if(!ValidStepIndex(nStepIndexR))
        {
            //
            // The step index is out of range - this is considered a fatal
            // error as the input stream is corrupted.  We fail by returning
            // zero bytes converted.
            //

            result = FMOD_ERR_FILE_BAD;
            break;
        }

        //
        // Write out first sample
        //

        *pbDst++ = nPredSampleL;
        *pbDst++ = nPredSampleR;

        //
        // The first DWORD contains 4 left samples, the second DWORD
        // contains 4 right samples.  We process the source in 8-byte
        // chunks to make it easy to interleave the output correctly.
        //

        while(cSamples)
        {
            dwLeft   = *(unsigned int *)pbBlock;
#ifdef PLATFORM_ENDIAN_BIG
	        dwLeft   = FMOD_SWAPENDIAN_DWORD(dwLeft);
#endif
            pbBlock += sizeof(unsigned int);
            dwRight  = *(unsigned int *)pbBlock;
#ifdef PLATFORM_ENDIAN_BIG
    	    dwRight = FMOD_SWAPENDIAN_DWORD(dwRight);
#endif
            pbBlock += sizeof(unsigned int);

//            cSubSamples = min(cSamples, 8);
            cSubSamples = 8;
            if (cSamples < cSubSamples)
            {
                cSubSamples = cSamples;
            }
            
            for(i = 0; i < cSubSamples; i++)
            {
                //
                // LEFT channel
                //

                nEncSampleL  = (dwLeft & 0x0F);
                nStepSize    = IMAAdpcm_StepTab[nStepIndexL];
                nPredSampleL = IMAAdpcm_DecodeSample(nEncSampleL, nPredSampleL, nStepSize);
                nStepIndexL  = IMAAdpcm_NextStepIndex(nEncSampleL, nStepIndexL);

                //
                // RIGHT channel
                //

                nEncSampleR  = (dwRight & 0x0F);
                nStepSize    = IMAAdpcm_StepTab[nStepIndexR];
                nPredSampleR = IMAAdpcm_DecodeSample(nEncSampleR, nPredSampleR, nStepSize);
                nStepIndexR  = IMAAdpcm_NextStepIndex(nEncSampleR, nStepIndexR);

                //
                // Write out sample
                //

                *pbDst++ = nPredSampleL;
                *pbDst++ = nPredSampleR;

                //
                // Shift the next input sample into the low-order 4 bits.
                //

                dwLeft >>= 4;
                dwRight >>= 4;
            }

            cSamples -= cSubSamples;
        }

        //
        // Skip padding
        //

        pbSrc += nBlockAlignment;
    }

    return result;
}


FMOD_RESULT IMAAdpcm_DecodeM16(unsigned char *pbSrc, float *pbDst, unsigned int cBlocks,  unsigned int nBlockAlignment, unsigned int cSamplesPerBlock, int channels)
{
    FMOD_RESULT    result = FMOD_OK;
    unsigned char *pbBlock;
    unsigned int   cSamples;
    unsigned char  bSample;
    int            nStepSize;
    int            nEncSample;
    int            nPredSample;
    int            nStepIndex;
    unsigned int   dwHeader;

    //
    // Enter the main loop
    //
    
    while(cBlocks--)
    {
        pbBlock = pbSrc;
        cSamples = cSamplesPerBlock - 1;
        
        //
        // Block header
        //

        dwHeader = *(unsigned int *)pbBlock;
#ifdef PLATFORM_ENDIAN_BIG
        dwHeader = FMOD_SWAPENDIAN_DWORD(dwHeader);
#endif
        pbBlock += sizeof(unsigned int);

        nPredSample = (int)(short)LOWORD(dwHeader);
        nStepIndex = (int)(unsigned char)HIWORD(dwHeader);

        if(!ValidStepIndex(nStepIndex))
        {
            //
            // The step index is out of range - this is considered a fatal
            // error as the input stream is corrupted.  We fail by returning
            // zero chars converted.
            //

            result = FMOD_ERR_FILE_BAD;
            break;
        }
        
        //
        // Write out first sample
        //

        *pbDst = nPredSample * (1.0f / 32768.0f);
        pbDst += channels;

        //
        // Enter the block loop
        //
        while(cSamples > 1)
        {
            bSample = *pbBlock++;

            //
            // Sample 1
            //

            nEncSample  = (bSample & (unsigned char)0x0F);
            nStepSize   = IMAAdpcm_StepTab[nStepIndex];
            nPredSample = IMAAdpcm_DecodeSample(nEncSample, nPredSample, nStepSize);
            nStepIndex  = IMAAdpcm_NextStepIndex(nEncSample, nStepIndex);

            pbDst[0] = nPredSample * (1.0f / 32768.0f);

            //
            // Sample 2
            //
            nEncSample  = (bSample >> 4);
            nStepSize   = IMAAdpcm_StepTab[nStepIndex];
            nPredSample = IMAAdpcm_DecodeSample(nEncSample, nPredSample, nStepSize);
            nStepIndex  = IMAAdpcm_NextStepIndex(nEncSample, nStepIndex);

            pbDst[channels] = nPredSample * (1.0f / 32768.0f);
             
            cSamples -=2;
            pbDst += channels*2;
        }
        
        if (cSamples)
        {
            bSample = *pbBlock++;

            //
            // Sample 1
            //

            nEncSample  = (bSample & (unsigned char)0x0F);
            nStepSize   = IMAAdpcm_StepTab[nStepIndex];
            nPredSample = IMAAdpcm_DecodeSample(nEncSample, nPredSample, nStepSize);
            nStepIndex  = IMAAdpcm_NextStepIndex(nEncSample, nStepIndex);

            pbDst[0] = nPredSample * (1.0f / 32768.0f);        
            cSamples --;
            pbDst += channels;
        }

        //
        // Skip padding
        //

        pbSrc += nBlockAlignment;
    }

    return result;
}


FMOD_RESULT IMAAdpcm_DecodeS16(unsigned char *pbSrc, float *pbDst, unsigned int cBlocks, unsigned int nBlockAlignment, unsigned int cSamplesPerBlock)
{
    FMOD_RESULT    result = FMOD_OK;
    unsigned char *pbBlock;
    unsigned int   cSamples;
    unsigned int   cSubSamples;
    int            nStepSize;
    unsigned int   dwHeader;
    unsigned int   dwLeft;
    unsigned int   dwRight;
    int            nEncSampleL;
    int            nPredSampleL;
    int            nStepIndexL;
    int            nEncSampleR;
    int            nPredSampleR;
    int            nStepIndexR;
    unsigned int   i;

    //
    // Enter the main loop
    //
    
    while(cBlocks--)
    {
        pbBlock = pbSrc;
        cSamples = cSamplesPerBlock - 1;

        //
        // LEFT channel header
        //

        dwHeader = *(unsigned int *)pbBlock;
#ifdef PLATFORM_ENDIAN_BIG
        dwHeader = FMOD_SWAPENDIAN_DWORD(dwHeader);
#endif
        pbBlock += sizeof(unsigned int);
        
        nPredSampleL = (int)(short)LOWORD(dwHeader);
        nStepIndexL = (int)(unsigned char)HIWORD(dwHeader);

        if(!ValidStepIndex(nStepIndexL)) 
        {
            //
            // The step index is out of range - this is considered a fatal
            // error as the input stream is corrupted.  We fail by returning
            // zero bytes converted.
            //

            result = FMOD_ERR_FILE_BAD;
            break;
        }
        
        //
        // RIGHT channel header
        //

        dwHeader = *(unsigned int *)pbBlock;
#ifdef PLATFORM_ENDIAN_BIG
        dwHeader = FMOD_SWAPENDIAN_DWORD(dwHeader);
#endif
        pbBlock += sizeof(unsigned int);
        
        nPredSampleR = (int)(short)LOWORD(dwHeader);
        nStepIndexR = (int)(unsigned char)HIWORD(dwHeader);

        if(!ValidStepIndex(nStepIndexR))
        {
            //
            // The step index is out of range - this is considered a fatal
            // error as the input stream is corrupted.  We fail by returning
            // zero bytes converted.
            //

            result = FMOD_ERR_FILE_BAD;
            break;
        }

        //
        // Write out first sample
        //

        *pbDst++ = nPredSampleL * (1.0f / 32768.0f);
        *pbDst++ = nPredSampleR * (1.0f / 32768.0f);

        //
        // The first DWORD contains 4 left samples, the second DWORD
        // contains 4 right samples.  We process the source in 8-byte
        // chunks to make it easy to interleave the output correctly.
        //

        while(cSamples)
        {
            dwLeft   = *(unsigned int *)pbBlock;
#ifdef PLATFORM_ENDIAN_BIG
	        dwLeft   = FMOD_SWAPENDIAN_DWORD(dwLeft);
#endif
            pbBlock += sizeof(unsigned int);
            dwRight  = *(unsigned int *)pbBlock;
#ifdef PLATFORM_ENDIAN_BIG
    	    dwRight = FMOD_SWAPENDIAN_DWORD(dwRight);
#endif
            pbBlock += sizeof(unsigned int);

//            cSubSamples = min(cSamples, 8);
            cSubSamples = 8;
            if (cSamples < cSubSamples)
            {
                cSubSamples = cSamples;
            }
            
            for(i = 0; i < cSubSamples; i++)
            {
                //
                // LEFT channel
                //

                nEncSampleL  = (dwLeft & 0x0F);
                nStepSize    = IMAAdpcm_StepTab[nStepIndexL];
                nPredSampleL = IMAAdpcm_DecodeSample(nEncSampleL, nPredSampleL, nStepSize);
                nStepIndexL  = IMAAdpcm_NextStepIndex(nEncSampleL, nStepIndexL);

                //
                // RIGHT channel
                //

                nEncSampleR  = (dwRight & 0x0F);
                nStepSize    = IMAAdpcm_StepTab[nStepIndexR];
                nPredSampleR = IMAAdpcm_DecodeSample(nEncSampleR, nPredSampleR, nStepSize);
                nStepIndexR  = IMAAdpcm_NextStepIndex(nEncSampleR, nStepIndexR);

                //
                // Write out sample
                //

                *pbDst++ = nPredSampleL * (1.0f / 32768.0f);
                *pbDst++ = nPredSampleR * (1.0f / 32768.0f);

                //
                // Shift the next input sample into the low-order 4 bits.
                //

                dwLeft >>= 4;
                dwRight >>= 4;
            }

            cSamples -= cSubSamples;
        }

        //
        // Skip padding
        //

        pbSrc += nBlockAlignment;
    }

    return result;
}

}
#endif

