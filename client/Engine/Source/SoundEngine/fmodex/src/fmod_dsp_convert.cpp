#include "fmod_settings.h"

#include "fmod_dspi.h"
#include "fmod_dsp_filter.h"
#include "fmod_dsp_resampler.h"
#include "fmod_dsp_soundcard.h"
#include "fmod_localcriticalsection.h"
#include "fmod_memory.h"
#include "fmod_soundi.h"
#include "fmod_string.h"
#include "fmod_systemi.h"

#ifdef FMOD_SUPPORT_SIMD
extern "C"
{
    void FMOD_DSP_Convert_FloatToPCM16(short *outbuffer, float *inbuffer, unsigned int length, int destchannelstep, int srcchannelstep, float volume);
}
#endif

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
// VERIFY: 93c2e1c7 manual
// OFFSET: 0x0093CD10
FMOD_RESULT DSPI::convert(void *outbuffer, void *inbuffer, FMOD_SOUND_FORMAT outformat, FMOD_SOUND_FORMAT informat, unsigned int length, int destchannelstep, int srcchannelstep, float volume)
{  
    bool  supportssimd;

    supportssimd = FMOD_OS_SupportsSIMD();
    
    if (outformat == FMOD_SOUND_FORMAT_PCMFLOAT)
    {
        float *destptr = (float *)outbuffer;

        switch (informat)
        {
            case FMOD_SOUND_FORMAT_PCM8:
            {
                signed char *srcptr = (signed char *)inbuffer;
                unsigned int len;

                volume /= (float)(1<<7);

                len = length >> 3;
                while (len)
                {
                    destptr[0 * destchannelstep] = (float)srcptr[0 * srcchannelstep] * volume;
                    destptr[1 * destchannelstep] = (float)srcptr[1 * srcchannelstep] * volume;
                    destptr[2 * destchannelstep] = (float)srcptr[2 * srcchannelstep] * volume;
                    destptr[3 * destchannelstep] = (float)srcptr[3 * srcchannelstep] * volume;
                    destptr[4 * destchannelstep] = (float)srcptr[4 * srcchannelstep] * volume;
                    destptr[5 * destchannelstep] = (float)srcptr[5 * srcchannelstep] * volume;
                    destptr[6 * destchannelstep] = (float)srcptr[6 * srcchannelstep] * volume;
                    destptr[7 * destchannelstep] = (float)srcptr[7 * srcchannelstep] * volume;
                    destptr += (8 * destchannelstep);
                    srcptr  += (8 * srcchannelstep);
                    len--;
                }
                len = length & 7;
                while (len)
                {
                    destptr[0] = (float)srcptr[0 * srcchannelstep] * volume;
                    destptr += destchannelstep;
                    srcptr  += srcchannelstep;
                    len--;
                }
                break;
            }
            case FMOD_SOUND_FORMAT_PCM16:
            {
                signed short *srcptr = (signed short *)inbuffer;
                unsigned int len;

                volume /= (float)(1<<15);

                len = length >> 3;
                while (len)
                {
                    destptr[0 * destchannelstep] = (float)srcptr[0 * srcchannelstep] * volume;
                    destptr[1 * destchannelstep] = (float)srcptr[1 * srcchannelstep] * volume;
                    destptr[2 * destchannelstep] = (float)srcptr[2 * srcchannelstep] * volume;
                    destptr[3 * destchannelstep] = (float)srcptr[3 * srcchannelstep] * volume;
                    destptr[4 * destchannelstep] = (float)srcptr[4 * srcchannelstep] * volume;
                    destptr[5 * destchannelstep] = (float)srcptr[5 * srcchannelstep] * volume;
                    destptr[6 * destchannelstep] = (float)srcptr[6 * srcchannelstep] * volume;
                    destptr[7 * destchannelstep] = (float)srcptr[7 * srcchannelstep] * volume;
                    destptr += (8 * destchannelstep);
                    srcptr  += (8 * srcchannelstep);
                    len--;
                }
                len = length & 7;
                while (len)
                {
                    destptr[0] = (float)srcptr[0] * volume;
                    destptr += destchannelstep;
                    srcptr  += srcchannelstep;
                    len--;
                }
                break;
            }
            case FMOD_SOUND_FORMAT_PCM24:
            {
                FMOD_INT24 *srcptr = (FMOD_INT24 *)inbuffer;
                unsigned int count;

                volume /= (float)(1<<23);

                for (count = 0; count < length; count++)
                {
                    signed int val;

                    val  = ((unsigned int)srcptr->val[0 * srcchannelstep] <<  8);
                    val |= ((unsigned int)srcptr->val[1 * srcchannelstep] << 16);
                    val |= ((unsigned int)srcptr->val[2 * srcchannelstep] << 24);
                    val >>= 8;

                    destptr[0] = (float)val * volume;
                    srcptr  += srcchannelstep;
                    destptr += destchannelstep;
                }
                break;
            }
            case FMOD_SOUND_FORMAT_PCM32: 
            {
                signed int *srcptr = (signed int *)inbuffer;
                unsigned int len;

                volume /= (float)(1U<<31);

                len = length >> 2;
                while (len)
                {
                    destptr[0 * destchannelstep] = (float)(srcptr[0 * srcchannelstep]) * volume;
                    destptr[1 * destchannelstep] = (float)(srcptr[1 * srcchannelstep]) * volume;
                    destptr[2 * destchannelstep] = (float)(srcptr[2 * srcchannelstep]) * volume;
                    destptr[3 * destchannelstep] = (float)(srcptr[3 * srcchannelstep]) * volume;
                    destptr += (4 * destchannelstep);
                    srcptr  += (4 * srcchannelstep);
                    len--;
                }
                len = length & 3;
                while (len)
                {
                    destptr[0] = (float)(srcptr[0]) * volume;
                    destptr += destchannelstep;
                    srcptr  += srcchannelstep;
                    len--;
                }
                break;
            }
            case FMOD_SOUND_FORMAT_PCMFLOAT:
            {
                float *srcptr = (float *)inbuffer;
                unsigned int len;
                
                len = length >> 2;
                while (len)
                {
                    destptr[0 * destchannelstep] = srcptr[0 * srcchannelstep] * volume;
                    destptr[1 * destchannelstep] = srcptr[1 * srcchannelstep] * volume;
                    destptr[2 * destchannelstep] = srcptr[2 * srcchannelstep] * volume;
                    destptr[3 * destchannelstep] = srcptr[3 * srcchannelstep] * volume;
                    destptr += (4 * destchannelstep);
                    srcptr  += (4 * srcchannelstep);
                    len--;
                }

                len = length & 3;
                while (len)
                {
                    destptr[0] = srcptr[0] * volume;
                    destptr += destchannelstep;
                    srcptr  += srcchannelstep;
                    len--;
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else
    {
        float *srcptr = (float *)inbuffer;

        /*
            Source must be float!
        */
        if (informat != FMOD_SOUND_FORMAT_PCMFLOAT)
        {
            return FMOD_ERR_DSP_FORMAT;
        }

        switch (outformat)
        {
            case FMOD_SOUND_FORMAT_PCM8:
            {
                signed char *destptr = (signed char *)outbuffer;
                unsigned int len;

                volume *= (float)(1<<7);

                len = length >> 2;
                while (len)
                {
                    signed int val[4];

                    val[0] = (signed int)(srcptr[0 * srcchannelstep] * volume);
                    val[1] = (signed int)(srcptr[1 * srcchannelstep] * volume);
                    val[2] = (signed int)(srcptr[2 * srcchannelstep] * volume);
                    val[3] = (signed int)(srcptr[3 * srcchannelstep] * volume);

                    destptr[0 * destchannelstep] = val[0] < -128 ? -128 : val[0] > 127 ? 127 : (signed char)val[0];
                    destptr[1 * destchannelstep] = val[1] < -128 ? -128 : val[1] > 127 ? 127 : (signed char)val[1];
                    destptr[2 * destchannelstep] = val[2] < -128 ? -128 : val[2] > 127 ? 127 : (signed char)val[2];
                    destptr[3 * destchannelstep] = val[3] < -128 ? -128 : val[3] > 127 ? 127 : (signed char)val[3];

                    srcptr  += (4 * srcchannelstep);
                    destptr += (4 * destchannelstep);
                    len--;
                }

                len = length  & 3;
                while (len)
                {
                    signed int val;
                    
                    val = (signed int)(srcptr[0] * volume);

                    destptr[0] = val < -128 ? -128 : val > 127 ? 127 : (signed char)val;
                    
                    srcptr  += srcchannelstep;
                    destptr += destchannelstep;
                    len--;
                }
                break;
                break;
            }
            case FMOD_SOUND_FORMAT_PCM16:
            {
                volume *= (float)(1<<15);

                #ifdef FMOD_SUPPORT_SIMD
                if (supportssimd)
                {
                    FMOD_DSP_Convert_FloatToPCM16((short *)outbuffer, (float *)inbuffer, length, destchannelstep, srcchannelstep, volume);
                }
                else
                #endif
                {
                    signed short *destptr = (signed short *)outbuffer;
                    unsigned int len;

                    len = length >> 2;
                    while (len)
                    {
                        signed int val[4];

                        val[0] = (signed int)(srcptr[0 * srcchannelstep] * volume);
                        val[1] = (signed int)(srcptr[1 * srcchannelstep] * volume);
                        val[2] = (signed int)(srcptr[2 * srcchannelstep] * volume);
                        val[3] = (signed int)(srcptr[3 * srcchannelstep] * volume);

                        destptr[0 * destchannelstep] = val[0] < -32768 ? -32768 : val[0] > 32767 ? 32767 : (signed short)val[0];
                        destptr[1 * destchannelstep] = val[1] < -32768 ? -32768 : val[1] > 32767 ? 32767 : (signed short)val[1];
                        destptr[2 * destchannelstep] = val[2] < -32768 ? -32768 : val[2] > 32767 ? 32767 : (signed short)val[2];
                        destptr[3 * destchannelstep] = val[3] < -32768 ? -32768 : val[3] > 32767 ? 32767 : (signed short)val[3];

                        srcptr  += (4 * srcchannelstep);
                        destptr += (4 * destchannelstep);
                        len--;
                    }

                    len = length & 3;
                    while (len)
                    {
                        signed int val;
                    
                        val = (signed int)(srcptr[0] * volume);

                        destptr[0] = val < -32768 ? -32768 : val > 32767 ? 32767 : (signed short)val;
                    
                        srcptr  += srcchannelstep;
                        destptr += destchannelstep;
                        len--;
                    }
                }
                break;
            }
            case FMOD_SOUND_FORMAT_PCM24:
            {
                FMOD_INT24 *destptr = (FMOD_INT24 *)outbuffer;
                unsigned int count;

                volume *= (float)(1<<23);

                for (count = 0; count < length; count++)
                {
                    signed int val;

                    val = (signed int)(srcptr[0] * volume);
                    val = val < -8388608 ? -8388608 : val > 8388607 ? 8388607 : val;
                    
                    destptr->val[0] = (val >>  0) & 0xFF;
                    destptr->val[1] = (val >>  8) & 0xFF;
                    destptr->val[2] = (val >> 16) & 0xFF;
                    destptr += destchannelstep;
                    srcptr  += srcchannelstep;
                }
                break;
            }
            case FMOD_SOUND_FORMAT_PCM32: 
            {
                signed int *destptr = (signed int *)outbuffer;
                unsigned int count;

                volume *= (float)(1U<<31);

                for (count = 0; count < length; count++)
                {
                    float val = srcptr[0] * volume;
                    destptr[0] = val < -2147483648.0f ? -(signed int)2147483647 : val > 2147483647.0f ? 2147483647 : (signed int)val;
                    
                    destptr += destchannelstep;
                    srcptr  += srcchannelstep;
                }
                break;
            }
            case FMOD_SOUND_FORMAT_PCMFLOAT:
            {
                float *destptr = (float *)outbuffer;
                unsigned int count;

                for (count = 0; count < length; count++)
                {
                    float val = srcptr[0] * volume;
                    destptr[0] = val < -1.0f ? -1.0f : val > 1.0f ? 1.0f : val;

                    destptr += destchannelstep;
                    srcptr  += srcchannelstep;
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return FMOD_OK;
}


}

