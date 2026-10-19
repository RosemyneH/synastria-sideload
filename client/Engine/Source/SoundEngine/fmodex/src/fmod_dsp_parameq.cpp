#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_PARAMEQ

#include "fmod.h"
#include "fmod_dspi.h"
#include "fmod_dsp_parameq.h"

#ifdef PLATFORM_PS3_SPU
    #include "fmod_systemi_spu.h"
#else
    #include "fmod_systemi.h"
#endif

#include <stdio.h>

#ifdef PLATFORM_PS3
extern unsigned int _binary_spu_fmod_dsp_parameq_pic_start[];
#endif

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dspparameq;

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
        return DSPParamEq::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */

#ifndef PLATFORM_PS3_SPU

FMOD_DSP_PARAMETERDESC dspparameq_param[3] =
{
    { 20.0f,  22000.0f, 8000.0f, "Center freq",     "hz",      "Frequency center.  20.0 to 22000.0.  Default = 8000.0." },
    {  0.2f,     5.0f,     1.0f, "Octave range",    "octaves", "Octave range around the center frequency to filter.  0.2 to 5.0.  Default = 1.0." },
    {  0.05f,    3.0f,     1.0f, "Frequency gain",  "",        "Frequency Gain.  0.05 to 3.0.  Default = 1.0." },
};

#endif // PLATFORM_PS3_SPU

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
// VERIFY: ca858cf3 auto
// OFFSET: 0x008EEC70
FMOD_DSP_DESCRIPTION_EX *DSPParamEq::getDescriptionEx()
{
#ifndef PLATFORM_PS3_SPU
    FMOD_memset(&dspparameq, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    FMOD_strcpy(dspparameq.name, "FMOD ParamEQ");
    dspparameq.version       = 0x00010100;
    dspparameq.create        = DSPParamEq::createCallback;
    dspparameq.reset         = DSPParamEq::resetCallback;

    #ifdef PLATFORM_PS3
    dspparameq.read          = (FMOD_DSP_READCALLBACK)_binary_spu_fmod_dsp_parameq_pic_start; // MRAM ADDRESS OF SPU CODE
    #else
    dspparameq.read          = DSPParamEq::readCallback;
    #endif

    dspparameq.numparameters = sizeof(dspparameq_param) / sizeof(dspparameq_param[0]);
    dspparameq.paramdesc     = dspparameq_param;
    dspparameq.setparameter  = DSPParamEq::setParameterCallback;
    dspparameq.getparameter  = DSPParamEq::getParameterCallback;

    dspparameq.mType         = FMOD_DSP_TYPE_PARAMEQ;
    dspparameq.mCategory     = FMOD_DSP_CATEGORY_FILTER;
    dspparameq.mSize         = sizeof(DSPParamEq);
#else
    dspparameq.read          = DSPParamEq::readCallback;                  /* We only care about read function on SPU */
#endif

    return &dspparameq;
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
// VERIFY: 175a0b95 manual
// OFFSET: 0x008EEB20
FMOD_RESULT DSPParamEq::createInternal()
{
    int count;

    init();

    mBandwidth = 0.2f;
    mGain = 1.0f;

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
    mCenter    = mCenterUpdate;
    mBandwidth = mBandwidthUpdate;
    mGain      = mGainUpdate;

    updateCoefficients(mCenter, mBandwidth, mGain);

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
FMOD_RESULT DSPParamEq::resetInternal()
{
    int count;

    for (count=0; count < DSP_MAXLEVELS_MAX; count++)
    {
        mFilterIn[count][0]  = mFilterIn[count][1]  = 0;
        mFilterOut[count][0] = mFilterOut[count][1] = 0;
    }

    return FMOD_OK;
}

#endif // !PLATFORM_PS3_SPU

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
// VERIFY: f1cb7e6e manual
// OFFSET: 0x008EDF40
FMOD_RESULT DSPParamEq::updateCoefficients(float center, float bandwidth, float gain)
{
    int outputrate;

#ifdef PLATFORM_PS3_SPU
    outputrate = 48000;
#else
    mSystem->getSoftwareFormat(&outputrate, 0, 0, 0, 0, 0);
#endif

    float w0 = 2.0f * FMOD_PI * center / outputrate;
                      
    #if 0
    
    // bandpass
    float alpha = FMOD_SIN(w0) * FMOD_SINH( FMOD_LOG(2.0f) / 2.0f * bandwidth * w0 / FMOD_SIN(w0) );    // (case: BW)

    mCoefficient_b0 =   FMOD_SIN(w0) / 2.0f;   //  =   Q*alpha
    mCoefficient_b1 =   0;
    mCoefficient_b2 =  -FMOD_SIN(w0) / 2.0f;   //  =  -Q*alpha
    mCoefficient_a0 =   1.0f + alpha;
    mCoefficient_a1 =  -2.0f * FMOD_COS(w0);
    mCoefficient_a2 =   1.0f - alpha;
    #endif

    #if 0

    // notch
    float alpha = FMOD_SIN(w0) * FMOD_SINH( FMOD_LOG(2.0f) / 2.0f * bandwidth * w0 / FMOD_SIN(w0) );    // (case: BW)

    mCoefficient_b0 =  1.0f;
    mCoefficient_b1 = -2.0f * FMOD_COS(w0);
    mCoefficient_b2 =  1.0f;
    mCoefficient_a0 =  1.0f + alpha;
    mCoefficient_a1 = -2.0f * FMOD_COS(w0);
    mCoefficient_a2 =  1.0f - alpha;
    #endif

    #if 1

    // Peaking EQ
    float Q     = 1.0f / bandwidth;
    float A     = gain;
    float alpha = FMOD_SIN(w0) / (2.0f * Q);

    mCoefficient_b0 =   1.0f + alpha *A;
    mCoefficient_b1 =  -2.0f * FMOD_COS(w0);
    mCoefficient_b2 =   1.0f - alpha*A;
    mCoefficient_a0 =   1.0f + alpha/A;
    mCoefficient_a1 =  -2.0f * FMOD_COS(w0);
    mCoefficient_a2 =   1.0f - alpha/A;

    #endif
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
// VERIFY: 59e1faa7 manual
// OFFSET: 0x008EDFC0
FMOD_RESULT DSPParamEq::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    unsigned int count;
    int count2;
    static float dc = (float)1E-20;

    if (mCenter != mCenterUpdate || mBandwidth != mBandwidthUpdate || mGain != mGainUpdate)
    {
        mCenter    = mCenterUpdate;
        mBandwidth = mBandwidthUpdate;
        mGain      = mGainUpdate;

        updateCoefficients(mCenter, mBandwidth, mGain);
    }

    float   a0 = 1.0f / mCoefficient_a0, a1 = mCoefficient_a1, a2 = mCoefficient_a2;       /* numerator coefficients */
    float   b0 =        mCoefficient_b0, b1 = mCoefficient_b1, b2 = mCoefficient_b2;       /* denominator coefficients */      
    
    if (inchannels == 1)
    {
        float in1 = mFilterIn[0][0];
        float in2 = mFilterIn[0][1];
        float out1 = mFilterOut[0][0];
        float out2 = mFilterOut[0][1];
        
        for (count = 0; count < length; count++)
        {
            float in0, out0;
                       
            in0  = inbuffer[count] + dc;
            out0 = (b0 * in0 + b1 * in1 + b2 * in2 - a1 * out1 - a2 * out2) * a0;

	        in2 = in1;
            in1 = in0;
            out2 = out1;
            out1 = out0;

            outbuffer[count] = out0;

            dc = -dc;
        }

        mFilterIn[0][0] = in1;
        mFilterIn[0][1] = in2;
        mFilterOut[0][0] = out1;
        mFilterOut[0][1] = out2;       
    }
    else if (inchannels == 2)
    {
        float in1L  = mFilterIn[0][0];
        float in1R  = mFilterIn[1][0];
        float in2L  = mFilterIn[0][1];
        float in2R  = mFilterIn[1][1];
        float out1L = mFilterOut[0][0];
        float out1R = mFilterOut[1][0];
        float out2L = mFilterOut[0][1];
        float out2R = mFilterOut[1][1];

        while (length)
        {
            float in0L, out0L;
            float in0R, out0R;
            
            in0L = inbuffer[0] + dc;
            in0R = inbuffer[1] + dc;
            
            out0L = (b0 * in0L + b1 * in1L + b2 * in2L - a1 * out1L - a2 * out2L) * a0;
            out0R = (b0 * in0R + b1 * in1R + b2 * in2R - a1 * out1R - a2 * out2R) * a0;

	        in2L = in1L;
            in1L = in0L;
	        in2R = in1R;
            in1R = in0R;
            out2L = out1L;
            out1L = out0L;
            out2R = out1R;
            out1R = out0R;

            outbuffer[0] = out0L;
            outbuffer[1] = out0R;

            dc = -dc;
            outbuffer+=2;
            inbuffer+=2;
            length--;
        }

        mFilterIn[0][0] = in1L;
        mFilterIn[1][0] = in1R;
        mFilterIn[0][1] = in2L;
        mFilterIn[1][1] = in2R;
        mFilterOut[0][0] = out1L;
        mFilterOut[1][0] = out1R;
        mFilterOut[0][1] = out2L;
        mFilterOut[1][1] = out2R;        
    }
    else if (inchannels == 6)
    {
        float in1A  = mFilterIn[0][0];
        float in1B  = mFilterIn[1][0];
        float in1C  = mFilterIn[2][0];
        float in1D  = mFilterIn[3][0];
        float in1E  = mFilterIn[4][0];
        float in1F  = mFilterIn[5][0];
        float in2A  = mFilterIn[0][1];
        float in2B  = mFilterIn[1][1];
        float in2C  = mFilterIn[2][1];
        float in2D  = mFilterIn[3][1];
        float in2E  = mFilterIn[4][1];
        float in2F  = mFilterIn[5][1];
        float out1A = mFilterOut[0][0];
        float out1B = mFilterOut[1][0];
        float out1C = mFilterOut[2][0];
        float out1D = mFilterOut[3][0];
        float out1E = mFilterOut[4][0];
        float out1F = mFilterOut[5][0];
        float out2A = mFilterOut[0][1];
        float out2B = mFilterOut[1][1];
        float out2C = mFilterOut[2][1];
        float out2D = mFilterOut[3][1];
        float out2E = mFilterOut[4][1];
        float out2F = mFilterOut[5][1];

        while (length)
        {
            float in0A, out0A;
            float in0B, out0B;
            float in0C, out0C;
            float in0D, out0D;
            float in0E, out0E;
            float in0F, out0F;
            
            in0A = inbuffer[0] + dc;
            in0B = inbuffer[1] + dc;
            in0C = inbuffer[2] + dc;
            in0D = inbuffer[3] + dc;
            in0E = inbuffer[4] + dc;
            in0F = inbuffer[5] + dc;
            
            out0A = (b0 * in0A + b1 * in1A + b2 * in2A - a1 * out1A - a2 * out2A) * a0;
            out0B = (b0 * in0B + b1 * in1B + b2 * in2B - a1 * out1B - a2 * out2B) * a0;
            out0C = (b0 * in0C + b1 * in1C + b2 * in2C - a1 * out1C - a2 * out2C) * a0;
            out0D = (b0 * in0D + b1 * in1D + b2 * in2D - a1 * out1D - a2 * out2D) * a0;
            out0E = (b0 * in0E + b1 * in1E + b2 * in2E - a1 * out1E - a2 * out2E) * a0;
            out0F = (b0 * in0F + b1 * in1F + b2 * in2F - a1 * out1F - a2 * out2F) * a0;

	        in2A = in1A;
	        in2B = in1B;
	        in2C = in1C;
	        in2D = in1D;
	        in2E = in1E;
	        in2F = in1F;
            in1A = in0A;
            in1B = in0B;
            in1C = in0C;
            in1D = in0D;
            in1E = in0E;
            in1F = in0F;

            out2A = out1A;
            out2B = out1B;
            out2C = out1C;
            out2D = out1D;
            out2E = out1E;
            out2F = out1F;
            out1A = out0A;
            out1B = out0B;
            out1C = out0C;
            out1D = out0D;
            out1E = out0E;
            out1F = out0F;

            outbuffer[0] = out0A;
            outbuffer[1] = out0B;
            outbuffer[2] = out0C;
            outbuffer[3] = out0D;
            outbuffer[4] = out0E;
            outbuffer[5] = out0F;

            dc = -dc;
            outbuffer+=6;
            inbuffer+=6;
            length--;
        }
        mFilterIn[0][0] = in1A;
        mFilterIn[1][0] = in1B;
        mFilterIn[2][0] = in1C;
        mFilterIn[3][0] = in1D;
        mFilterIn[4][0] = in1E;
        mFilterIn[5][0] = in1F;
        mFilterIn[0][1] = in2A;
        mFilterIn[1][1] = in2B;
        mFilterIn[2][1] = in2C;
        mFilterIn[3][1] = in2D;
        mFilterIn[4][1] = in2E;
        mFilterIn[5][1] = in2F;
        mFilterOut[0][0] = out1A;
        mFilterOut[1][0] = out1B;
        mFilterOut[2][0] = out1C;
        mFilterOut[3][0] = out1D;
        mFilterOut[4][0] = out1E;
        mFilterOut[5][0] = out1F;
        mFilterOut[0][1] = out2A;
        mFilterOut[1][1] = out2B;
        mFilterOut[2][1] = out2C;
        mFilterOut[3][1] = out2D;
        mFilterOut[4][1] = out2E;
        mFilterOut[5][1] = out2F;        
    }
    else
    {
        for (count2 = 0; count2 < inchannels; count2++)
        {
			unsigned int len;

            float *in = inbuffer + count2;
            float *out = outbuffer + count2;
			
			len = length;
			float in1  = mFilterIn[count2][0];
			float in2  = mFilterIn[count2][1];
			float out1 = mFilterOut[count2][0];
			float out2 = mFilterOut[count2][1];
			while (len)
			{
				float in0, out0;
            
				in0 = in[0] + dc;
				out0 = (b0 * in0 + b1 * in1 + b2 * in2 - a1 * out1 - a2 * out2) * a0;
				in2 = in1;
				in1 = in0;
				out2 = out1;
				out1 = out0;
				out[0] = out0;

				in += inchannels;
				out += inchannels;
				len--;
				dc = -dc;
			}
            
			mFilterIn[count2][0] = in1;
			mFilterIn[count2][1] = in2;
			mFilterOut[count2][0] = out1;
			mFilterOut[count2][1] = out2;
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
// VERIFY: f35a3686 auto
// OFFSET: 0x008EE930
FMOD_RESULT DSPParamEq::setParameterInternal(int index, float value)
{
    FMOD_RESULT result;
    int outputrate;

    result = mSystem->getSoftwareFormat(&outputrate, 0, 0, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    switch (index)
    {
        case FMOD_DSP_PARAMEQ_CENTER:
        {
            if (value >= ((float)outputrate / 2.0f) - 100.0f)
            {
                value = ((float)outputrate / 2.0f) - 100.0f;
            }
            mCenterUpdate = value;
            break;
        }
        case FMOD_DSP_PARAMEQ_BANDWIDTH:
        {
            mBandwidthUpdate = value;
            break;
        }
        case FMOD_DSP_PARAMEQ_GAIN:
        {
            mGainUpdate = value;
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
// VERIFY: d3826967 auto
// OFFSET: 0x008EE9B0
FMOD_RESULT DSPParamEq::getParameterInternal(int index, float *value, char *valuestr)
{
    switch (index)
    {
        case FMOD_DSP_PARAMEQ_CENTER:
        {
            *value = mCenterUpdate;
            sprintf(valuestr, "%.02f", mCenterUpdate);
            break;
        }
        case FMOD_DSP_PARAMEQ_BANDWIDTH:
        {
            *value = mBandwidthUpdate;
            sprintf(valuestr, "%.02f", mBandwidthUpdate);
            break;
        }
        case FMOD_DSP_PARAMEQ_GAIN:
        {
            *value = mGainUpdate;
            sprintf(valuestr, "%.02f", mGainUpdate);
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
 
	[SEE_ALSO]
]
*/


#endif // !PLATFORM_PS3_SPU

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
// VERIFY: 57df82ee manual
// OFFSET: 0x008EEC40
FMOD_RESULT F_CALLBACK DSPParamEq::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPParamEq *parameq = (DSPParamEq *)dsp;

    return parameq->createInternal();
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
// VERIFY: e3bc7e9b manual
// OFFSET: 0x008EEA40
FMOD_RESULT F_CALLBACK DSPParamEq::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPParamEq *parameq = (DSPParamEq *)dsp;

    return parameq->resetInternal();
}

#endif // !PLATFORM_PS3_SPU

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
// VERIFY: df48cec1 auto
// OFFSET: 0x008EEA80
FMOD_RESULT F_CALLBACK DSPParamEq::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPParamEq *parameq = (DSPParamEq *)dsp;

    return parameq->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
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
// VERIFY: a34a64ec manual
// OFFSET: 0x008EEAB0
FMOD_RESULT F_CALLBACK DSPParamEq::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPParamEq *parameq = (DSPParamEq *)dsp;

    return parameq->setParameterInternal(index, value);
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
// VERIFY: 121f5f9d auto
// OFFSET: 0x008EEAF0
FMOD_RESULT F_CALLBACK DSPParamEq::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPParamEq *parameq = (DSPParamEq *)dsp;

    return parameq->getParameterInternal(index, value, valuestr);
}



#endif // !PLATFORM_PS3_SPU

}

#endif
