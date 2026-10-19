#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_LOWPASS_SIMPLE

#include "fmod.h"
#include "fmod_dspi.h"
#include "fmod_dsp_lowpass_simple.h"

#ifdef PLATFORM_PS3_SPU
    #include "fmod_systemi_spu.h"
    #include "fmod_common_spu.h"
    #include "fmod_spu_printf.h"
#else
    #include "fmod_systemi.h"
#endif

#include <stdio.h>
#include <math.h>

#define CUTOFF_MIN 10.0f
#define CUTOFF_MAX 22000.0f

#ifdef PLATFORM_PS3
extern unsigned int _binary_spu_fmod_dsp_lowpass_simple_pic_start[];
#endif

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dsplowpass_simple;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetDSPDescription is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_DSP_DESCRIPTION_EX * F_API FMODGetDSPDescriptionEx()
    {
        return DSPLowPassSimple::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */

#ifndef PLATFORM_PS3_SPU_PIC

FMOD_DSP_PARAMETERDESC dsplowpass_simple_param[1] =
{
    {  CUTOFF_MIN, CUTOFF_MAX,  5000.0f, "Cutoff freq", "hz", "Lowpass cutoff frequency in hz.  1.0 to 22000.0.  Default = 5000.0" },
};

#endif // PLATFORM_PS3_SPU_PIC

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
// VERIFY: 0eb7cdc5 auto
// OFFSET: 0x008F21C0
FMOD_DSP_DESCRIPTION_EX *DSPLowPassSimple::getDescriptionEx()
{
#ifndef PLATFORM_PS3_SPU
    FMOD_memset(&dsplowpass_simple, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    FMOD_strcpy(dsplowpass_simple.name, "FMOD Lowpass Simple");
    dsplowpass_simple.version       = 0x00010100;
    dsplowpass_simple.create        = DSPLowPassSimple::createCallback;
    dsplowpass_simple.reset         = DSPLowPassSimple::resetCallback;

    #ifdef PLATFORM_PS3
    dsplowpass_simple.read          = (FMOD_DSP_READCALLBACK)_binary_spu_fmod_dsp_lowpass_simple_pic_start;    /* SPU PIC entry address */
    #else
    dsplowpass_simple.read          = DSPLowPassSimple::readCallback;
    #endif

    dsplowpass_simple.numparameters = sizeof(dsplowpass_simple_param) / sizeof(dsplowpass_simple_param[0]);
    dsplowpass_simple.paramdesc     = dsplowpass_simple_param;
    dsplowpass_simple.setparameter  = DSPLowPassSimple::setParameterCallback;
    dsplowpass_simple.getparameter  = DSPLowPassSimple::getParameterCallback;

    dsplowpass_simple.mType         = FMOD_DSP_TYPE_LOWPASS_SIMPLE;
    dsplowpass_simple.mCategory     = FMOD_DSP_CATEGORY_FILTER;
    dsplowpass_simple.mSize         = sizeof(DSPLowPassSimple);
#else
    dsplowpass_simple.read          = DSPLowPassSimple::readCallback;                  /* We only care about read function on SPU */
#endif
    return &dsplowpass_simple;
}


#ifndef PLATFORM_PS3_SPU

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
// VERIFY: f65f5650 manual
// OFFSET: 0x008F2100
FMOD_RESULT DSPLowPassSimple::createInternal()
{
    int count;

    init();

    for (count = 0; count < mDescription.numparameters; count++)
    {
        FMOD_RESULT result;

        result = setParameter(count, mDescription.paramdesc[count].defaultval);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    resetInternal();

    /*
        Do a forced update here to make sure everything is calculated correctly first.
    */
    mCutoffHz = mCutoffHzUpdate;
    updateCoefficients(mCutoffHz);
    
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
FMOD_RESULT DSPLowPassSimple::resetInternal()
{
    int count;

    for (count = 0; count < DSP_MAXLEVELS_MAX; count++)
    {
    	mFilter_Y[count][0] = mFilter_Y[count][1] = 0;
    }

    return FMOD_OK;
}

#endif //!PLATFORM_PS3_SPU


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 976c72e5 auto
// OFFSET: 0x008F19F0
FMOD_RESULT DSPLowPassSimple::updateCoefficients(float cutoffhz)
{
    /*
        Simple RC based time constant method H(z) = tc / (1 - (1-tc)z^-1)
        Single pole {0 -> 0.99..} on real axis
    */
    int   outputrate;
    float RC;
    float dt;
    float threshold;

#ifdef PLATFORM_PS3_SPU
    outputrate = 48000;
#else
    mSystem->getSoftwareFormat(&outputrate, 0, 0, 0, 0, 0);
#endif

    dt = 1.0f/outputrate;
    threshold = outputrate/3.14159265358979323846f;

    if (cutoffhz >= CUTOFF_MAX)
    {
        mFilter_tc = 1.0f;
        mFilter_oneminus_tc = 0.0f;
    }
    else if (cutoffhz <= threshold)
    {
        RC = 1.0f / (2.0f * FMOD_PI * mCutoffHz);
        mFilter_tc = dt / (RC + dt);
        mFilter_oneminus_tc = 1.0f - mFilter_tc;
    }
    else
    {
        mFilter_tc = 0.666666667f + (cutoffhz-threshold)/(3.0f*(CUTOFF_MAX - threshold));
        mFilter_oneminus_tc = 1.0f - mFilter_tc;
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: fa49e284 manual
// OFFSET: 0x008F1AA0
FMOD_RESULT DSPLowPassSimple::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    static float dc = (float)1E-20;

    if (mCutoffHz != mCutoffHzUpdate)
    {
        mCutoffHz = mCutoffHzUpdate;
        updateCoefficients(mCutoffHz);
    }

    if (mFilter_tc == 1.0f)
    {
        FMOD_memcpy(outbuffer, inbuffer, sizeof(float)*length*inchannels);
        return FMOD_OK;
    }
	else if (mFilter_tc == 0.0f)
    {
        int count;

        FMOD_memset(outbuffer, 0, sizeof(float)*length*inchannels);

        for (count = 0; count < inchannels; count++)
        {
    	    mFilter_Y[count][0] = mFilter_Y[count][1] = 0;
        }
        return FMOD_OK;
    }
	
	if (inchannels == 1)
	{
		float fy1 = mFilter_Y[0][0];
		float fy2 = mFilter_Y[0][1];
		float fy;

		while (length)
		{
			fy =  mFilter_tc * (inbuffer[0]+dc) + mFilter_oneminus_tc * fy1;        
			fy1 = fy;	        
			fy =  mFilter_tc * fy + mFilter_oneminus_tc * fy2;        
			fy2 = fy;	        
			outbuffer[0] = fy;

			dc = -dc;
			length--;
			outbuffer++;
			inbuffer++;
		}

		mFilter_Y[0][0] = fy1;
		mFilter_Y[0][1] = fy2;
	}
	else if (inchannels == 2)
	{
		float fyL1 = mFilter_Y[0][0];
		float fyL2 = mFilter_Y[0][1];
		float fyR1 = mFilter_Y[1][0];
		float fyR2 = mFilter_Y[1][1];

		float fyL, fyR;
		while (length)
		{
			fyL =  mFilter_tc * (inbuffer[0]+dc) + mFilter_oneminus_tc * fyL1;        
			fyR =  mFilter_tc * (inbuffer[1]+dc) + mFilter_oneminus_tc * fyR1;
            
			fyL1 = fyL;	        
			fyR1 = fyR;	      

			fyL =  mFilter_tc * fyL + mFilter_oneminus_tc * fyL2;        
			fyR =  mFilter_tc * fyR + mFilter_oneminus_tc * fyR2;

			fyL2 = fyL;	        
			fyR2 = fyR;	
            
			outbuffer[0] = fyL;
			outbuffer[1] = fyR;

			dc = -dc;
			length--;
			outbuffer+=2;
			inbuffer+=2;
		}

		mFilter_Y[0][0] = fyL1;
		mFilter_Y[0][1] = fyL2;
		mFilter_Y[1][0] = fyR1;
		mFilter_Y[1][1] = fyR2;
	}
	else if (inchannels == 6)
	{
		float fyA1 = mFilter_Y[0][0];
		float fyA2 = mFilter_Y[0][1];
		float fyB1 = mFilter_Y[1][0];
		float fyB2 = mFilter_Y[1][1];
		float fyC1 = mFilter_Y[2][0];
		float fyC2 = mFilter_Y[2][1];
		float fyD1 = mFilter_Y[3][0];
		float fyD2 = mFilter_Y[3][1];
		float fyE1 = mFilter_Y[4][0];
		float fyE2 = mFilter_Y[4][1];
		float fyF1 = mFilter_Y[5][0];
		float fyF2 = mFilter_Y[5][1];

		float fyA,fyB,fyC,fyD,fyE,fyF;
		while (length)
		{
            
			fyA =  mFilter_tc * (inbuffer[0]+dc) + mFilter_oneminus_tc * fyA1;        
			fyB =  mFilter_tc * (inbuffer[1]+dc) + mFilter_oneminus_tc * fyB1;        
			fyC =  mFilter_tc * (inbuffer[2]+dc) + mFilter_oneminus_tc * fyC1;        
			fyD =  mFilter_tc * (inbuffer[3]+dc) + mFilter_oneminus_tc * fyD1;        
			fyE =  mFilter_tc * (inbuffer[4]+dc) + mFilter_oneminus_tc * fyE1;        
			fyF =  mFilter_tc * (inbuffer[5]+dc) + mFilter_oneminus_tc * fyF1;
            
			fyA1 = fyA;
			fyB1 = fyB;	        
			fyC1 = fyC;	        
			fyD1 = fyD;	        
			fyE1 = fyE;	        
			fyF1 = fyF;	        
           
			fyA =  mFilter_tc * fyA + mFilter_oneminus_tc * fyA2;        
			fyB =  mFilter_tc * fyB + mFilter_oneminus_tc * fyB2;        
			fyC =  mFilter_tc * fyC + mFilter_oneminus_tc * fyC2;        
			fyD =  mFilter_tc * fyD + mFilter_oneminus_tc * fyD2;        
			fyE =  mFilter_tc * fyE + mFilter_oneminus_tc * fyE2;        
			fyF =  mFilter_tc * fyF + mFilter_oneminus_tc * fyF2;
            
			fyA2 = fyA;	        
			fyB2 = fyB;	        
			fyC2 = fyC;	        
			fyD2 = fyD;	        
			fyE2 = fyE;	        
			fyF2 = fyF;	        

			outbuffer[0] = fyA;
			outbuffer[1] = fyB;
			outbuffer[2] = fyC;
			outbuffer[3] = fyD;
			outbuffer[4] = fyE;
			outbuffer[5] = fyF;

			dc = -dc;
			length--;
			outbuffer+=6;
			inbuffer+=6;
		}

		mFilter_Y[0][0] = fyA1;
		mFilter_Y[0][1] = fyA2;
		mFilter_Y[1][0] = fyB1;
		mFilter_Y[1][1] = fyB2;
		mFilter_Y[2][0] = fyC1;
		mFilter_Y[2][1] = fyC2;
		mFilter_Y[3][0] = fyD1;
		mFilter_Y[3][1] = fyD2;
		mFilter_Y[4][0] = fyE1;
		mFilter_Y[4][1] = fyE2;
		mFilter_Y[5][0] = fyF1;
		mFilter_Y[5][1] = fyF2;
	}
	else
	{
		int count2;

		for (count2 = 0; count2 < inchannels; count2++)
		{
			float *in = inbuffer + count2;
			float *out = outbuffer + count2;
			int len;
			
            
			float fy1 = mFilter_Y[count2][0];
			float fy2 = mFilter_Y[count2][1];
			float fy;
			len = length;
			while (len)
			{    
				fy =  mFilter_tc * (in[0]+dc) + mFilter_oneminus_tc * fy1;        
				fy1 = fy;	        
				fy =  mFilter_tc * fy + mFilter_oneminus_tc * fy2;        
				fy2 = fy;
				out[0] = fy;
                
				dc = -dc;
				len--;
				in += inchannels;
				out += inchannels;
			}

			mFilter_Y[count2][0] = fy1;
			mFilter_Y[count2][1] = fy2;
		}
	}
    return FMOD_OK;
}

#ifndef PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPLowPassSimple::setParameterInternal(int index, float value)
{
    switch (index)
    {
        case FMOD_DSP_LOWPASS_SIMPLE_CUTOFF:
        {
            mCutoffHzUpdate = value;
            break;
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPLowPassSimple::getParameterInternal(int index, float *value, char *valuestr)
{
    switch (index)
    {
        case FMOD_DSP_LOWPASS_CUTOFF:
        {
            *value = mCutoffHzUpdate;
            sprintf(valuestr, "%.02f", mCutoffHzUpdate);
            break;
        }
    }

    return FMOD_OK;
}

#endif //!PLATFORM_PS3_SPU


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



/*
    ==============================================================================================================

    CALLBACK INTERFACE

    ==============================================================================================================
*/

#ifndef PLATFORM_PS3_SPU

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
// VERIFY: 4642f97f manual
// OFFSET: 0x008F2190
FMOD_RESULT F_CALLBACK DSPLowPassSimple::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPLowPassSimple *lowpass_simple = (DSPLowPassSimple *)dsp;

    return lowpass_simple->createInternal();
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
// VERIFY: f9e089e9 manual
// OFFSET: 0x008F2010
FMOD_RESULT F_CALLBACK DSPLowPassSimple::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPLowPassSimple *lowpass_simple = (DSPLowPassSimple *)dsp;

    return lowpass_simple->resetInternal();
}

#endif

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
// VERIFY: 50c61e6d auto
// OFFSET: 0x008F2050
FMOD_RESULT F_CALLBACK DSPLowPassSimple::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPLowPassSimple *lowpass_simple = (DSPLowPassSimple *)dsp;

    return lowpass_simple->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
}

#ifndef PLATFORM_PS3_SPU

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
// VERIFY: dc521824 manual
// OFFSET: 0x008F2080
FMOD_RESULT F_CALLBACK DSPLowPassSimple::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPLowPassSimple *lowpass_simple = (DSPLowPassSimple *)dsp;

    return lowpass_simple->setParameterInternal(index, value);
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
// VERIFY: 7e5b50c6 manual
// OFFSET: 0x008F20B0
FMOD_RESULT F_CALLBACK DSPLowPassSimple::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPLowPassSimple *lowpass_simple = (DSPLowPassSimple *)dsp;

    return lowpass_simple->getParameterInternal(index, value, valuestr);
}



#endif

}

#endif

