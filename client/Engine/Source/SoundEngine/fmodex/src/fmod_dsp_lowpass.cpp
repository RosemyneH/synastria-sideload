#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_LOWPASS

#include "fmod.h"
#include "fmod_dspi.h"
#include "fmod_dsp_lowpass.h"

#ifdef PLATFORM_PS3_SPU
    #include "fmod_systemi_spu.h"
#else
    #include "fmod_systemi.h"
#endif

#include <stdio.h>
#include <math.h>

#ifdef PLATFORM_PS3
extern unsigned int _binary_spu_fmod_dsp_lowpass_pic_start[];
#endif

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dsplowpass;

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
        return DSPLowPass::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */

#ifndef PLATFORM_PS3_SPU

FMOD_DSP_PARAMETERDESC dsplowpass_param[2] =
{
    { 10.0f, 22000.0f,  5000.0, "Cutoff freq", "hz", "Lowpass cutoff frequency in hz.   1.0 to output 22000.0.  Default = 5000.0." },
    { 1.0f,    10.0f,     1.0, "Resonance",   "",   "Lowpass resonance Q value. 1.0 to 10.0.  Default = 1.0." }
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
// VERIFY: 6a2d1b66 auto
// OFFSET: 0x008F3790
FMOD_DSP_DESCRIPTION_EX *DSPLowPass::getDescriptionEx()
{
#ifndef PLATFORM_PS3_SPU
    FMOD_memset(&dsplowpass, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));
    
    FMOD_strcpy(dsplowpass.name, "FMOD Lowpass");
    dsplowpass.version       = 0x00010100;
    dsplowpass.create        = DSPLowPass::createCallback;

    #ifdef PLATFORM_PS3
    dsplowpass.read          = (FMOD_DSP_READCALLBACK)_binary_spu_fmod_dsp_lowpass_pic_start;    /* SPU PIC entry address */
    #else
    dsplowpass.read          = DSPLowPass::readCallback;
    #endif

    dsplowpass.numparameters = sizeof(dsplowpass_param) / sizeof(dsplowpass_param[0]);
    dsplowpass.paramdesc     = dsplowpass_param;
    dsplowpass.setparameter  = DSPLowPass::setParameterCallback;
    dsplowpass.getparameter  = DSPLowPass::getParameterCallback;

    dsplowpass.mType         = FMOD_DSP_TYPE_LOWPASS;
    dsplowpass.mCategory     = FMOD_DSP_CATEGORY_FILTER;
    dsplowpass.mSize         = sizeof(DSPLowPass);
#else
    dsplowpass.read          = DSPLowPass::readCallback;                  /* We only care about read function on SPU */
#endif

    return &dsplowpass;
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
// VERIFY: 23fd43e5 auto
// OFFSET: 0x008F2E00
FMOD_RESULT DSPLowPass::createInternal()
{
    int count;

    init();

    /*
        Setup filter s-domain coefficients
    */
	/* Section 1 */       
	mProtoCoef[0].a0 = 1.0f;
	mProtoCoef[0].a1 = 0;
	mProtoCoef[0].a2 = 0;
	mProtoCoef[0].b0 = 1.0f;
	mProtoCoef[0].b1 = 0.765367f;
	mProtoCoef[0].b2 = 1.0f;
	mGainFactor[0] = 1.0f;

	/* Section 2 */       
	mProtoCoef[1].a0 = 1.0f;
	mProtoCoef[1].a1 = 0;
	mProtoCoef[1].a2 = 0;
	mProtoCoef[1].b0 = 1.0f;
	mProtoCoef[1].b1 = 1.847759f;
	mProtoCoef[1].b2 = 1.0f;
	mGainFactor[1] = 1.0f;

    for (count = 0; count < mDescription.numparameters; count++)
    {
        FMOD_RESULT result;

        result = setParameter(count, mDescription.paramdesc[count].defaultval);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    /*
        Do a forced update here to make sure everything is calculated correctly first.
    */
    mResonance = mResonanceUpdate;
    mCutoffHz  = mCutoffHzUpdate;

    updateState(mResonance, mCutoffHz);

    return FMOD_OK;
}

#endif


/*
 * --------------------------------------------------------------------
 * 
 * iir_filter - Perform IIR filtering sample by sample on floats
 * 
 * Implements cascaded direct form II second order sections.
 * Requires FILTER structure for history and coefficients.
 * The length in the filter structure specifies the number of sections.
 * The size of the history array is 2 * LOWPASS_FILTER_SECTIONS.
 * The size of the coefficient array is 4 * LOWPASS_FILTER_SECTIONS + 1 because
 * the first coefficient is the overall scale factor for the filter.
 * Returns one output sample for each input sample.  Allocates history
 * array if not previously allocated.
 * 
 * float iir_filter(float input,FILTER *iir)
 * 
 *     float input        new float input sample
 *     FILTER *iir        pointer to FILTER structure
 * 
 * Returns float value giving the current output.
 * 
 * Allocation errors cause an error message and a call to exit.
 * --------------------------------------------------------------------
 */
FMOD_INLINE float DSPLowPass::filter(float input, int channel)
{
    int i;
    float *history1,*history2,*coef_ptr;
    float output,new_hist;
    static float dc = (float)1E-20;

    input += dc;
    dc = -dc;

	/* allocate history array if different size than last call */

    coef_ptr = mCoefficients;                   /* coefficient pointer */

    history1 = mHistory[channel];              /* first history */
    history2 = history1 + 1;                  /* next history */

    /* 1st number of coefficients array is overall input scale factor,
     * or filter gain */
    output = input * (*coef_ptr++);

    for (i = 0 ; i < LOWPASS_FILTER_SECTIONS; i++)
    {
        output   = output - history1[0] * coef_ptr[0];
        new_hist = output - history2[0] * coef_ptr[1];    /* poles */

        output = new_hist + history1[0] * coef_ptr[2];
        output = output   + history2[0] * coef_ptr[3];    /* zeros */

        coef_ptr += 4;
        *history2++ = *history1;
        *history1++ = new_hist;
        history1++;
        history2++;
    }

    return(output);
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
// VERIFY: eac87804 manual
// OFFSET: 0x008F2EE0
FMOD_RESULT DSPLowPass::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    unsigned int count;
    static float dc = (float)1E-20;

    if (mResonance != mResonanceUpdate || mCutoffHz != mCutoffHzUpdate)
    {
        mResonance = mResonanceUpdate;
        mCutoffHz  = mCutoffHzUpdate;

        updateState(mResonance, mCutoffHz);
    }

	if (inchannels == 1)
	{
		float *history;

		history = mHistory[0];

		while (length)
		{
			float a,b,c;

            a = ((inbuffer[0] + dc) * mCoefficients[0]) - (history[0] * mCoefficients[1]) - (history[1] * mCoefficients[2]);
            b =         (history[0] * mCoefficients[3]) + (history[1] * mCoefficients[4]) - (history[2] * mCoefficients[5]) - (history[3] * mCoefficients[6]);
            c =         (history[2] * mCoefficients[7]) + (history[3] * mCoefficients[8]);

			history[1] = history[0];
			history[3] = history[2];
     
			dc = -dc;
			length--;
			inbuffer++;
			outbuffer++; 

			history[0]    = a;
			history[2]    = a + b;
			outbuffer[-1] = a + b + c;
		}
	} 
	else if (inchannels == 2)
	{
		float *historyL;
		float *historyR;

		historyL = mHistory[0];
		historyR = mHistory[1];

		while (length)
		{
			float aL,bL,cL;
			float aR,bR,cR;

			aL = ((inbuffer[0] + dc) * mCoefficients[0]) - (historyL[0] * mCoefficients[1]) - (historyL[1] * mCoefficients[2]);
			aR = ((inbuffer[1] + dc) * mCoefficients[0]) - (historyR[0] * mCoefficients[1]) - (historyR[1] * mCoefficients[2]);
			bL =        (historyL[0] * mCoefficients[3]) + (historyL[1] * mCoefficients[4]) - (historyL[2] * mCoefficients[5]) - (historyL[3] * mCoefficients[6]);
			bR =        (historyR[0] * mCoefficients[3]) + (historyR[1] * mCoefficients[4]) - (historyR[2] * mCoefficients[5]) - (historyR[3] * mCoefficients[6]);
			cL =        (historyL[2] * mCoefficients[7]) + (historyL[3] * mCoefficients[8]);
			cR =        (historyR[2] * mCoefficients[7]) + (historyR[3] * mCoefficients[8]);

			dc = -dc;
			length--;
			inbuffer+=2;
			outbuffer+=2; 

			historyL[1] = historyL[0];
			historyL[3] = historyL[2];
			historyL[0]  = aL;
			historyL[2]  = aL + bL;
			historyR[1] = historyR[0];
			historyR[3] = historyR[2];
			historyR[0]  = aR;
			historyR[2]  = aR + bR;            
            
			outbuffer[-2] = aL + bL + cL;    
			outbuffer[-1] = aR + bR + cR;        
		}
	}
	else if (inchannels == 6)
	{
		float *history0 = mHistory[0];
		float *history1 = mHistory[1];
		float *history2 = mHistory[2];
		float *history3 = mHistory[3];
		float *history4 = mHistory[4];
		float *history5 = mHistory[5];

		while (length)
		{
			float a0,b0,c0;
			float a1,b1,c1;
			float a2,b2,c2;
			float a3,b3,c3;
			float a4,b4,c4;
			float a5,b5,c5;

			a0 = ((inbuffer[0] + dc) * mCoefficients[0]) - (history0[0] * mCoefficients[1]) - (history0[1] * mCoefficients[2]);
			a1 = ((inbuffer[1] + dc) * mCoefficients[0]) - (history1[0] * mCoefficients[1]) - (history1[1] * mCoefficients[2]);
			a2 = ((inbuffer[2] + dc) * mCoefficients[0]) - (history2[0] * mCoefficients[1]) - (history2[1] * mCoefficients[2]);
			a3 = ((inbuffer[3] + dc) * mCoefficients[0]) - (history3[0] * mCoefficients[1]) - (history3[1] * mCoefficients[2]);
			a4 = ((inbuffer[4] + dc) * mCoefficients[0]) - (history4[0] * mCoefficients[1]) - (history4[1] * mCoefficients[2]);
			a5 = ((inbuffer[5] + dc) * mCoefficients[0]) - (history5[0] * mCoefficients[1]) - (history5[1] * mCoefficients[2]);
			b0 =        (history0[0] * mCoefficients[3]) + (history0[1] * mCoefficients[4]) - (history0[2] * mCoefficients[5]) - (history0[3] * mCoefficients[6]);
			b1 =        (history1[0] * mCoefficients[3]) + (history1[1] * mCoefficients[4]) - (history1[2] * mCoefficients[5]) - (history1[3] * mCoefficients[6]);
			b2 =        (history2[0] * mCoefficients[3]) + (history2[1] * mCoefficients[4]) - (history2[2] * mCoefficients[5]) - (history2[3] * mCoefficients[6]);
			b3 =        (history3[0] * mCoefficients[3]) + (history3[1] * mCoefficients[4]) - (history3[2] * mCoefficients[5]) - (history3[3] * mCoefficients[6]);
			b4 =        (history4[0] * mCoefficients[3]) + (history4[1] * mCoefficients[4]) - (history4[2] * mCoefficients[5]) - (history4[3] * mCoefficients[6]);
			b5 =        (history5[0] * mCoefficients[3]) + (history5[1] * mCoefficients[4]) - (history5[2] * mCoefficients[5]) - (history5[3] * mCoefficients[6]);
			c0 =        (history0[2] * mCoefficients[7]) + (history0[3] * mCoefficients[8]);
			c1 =        (history1[2] * mCoefficients[7]) + (history1[3] * mCoefficients[8]);
			c2 =        (history2[2] * mCoefficients[7]) + (history2[3] * mCoefficients[8]);
			c3 =        (history3[2] * mCoefficients[7]) + (history3[3] * mCoefficients[8]);
			c4 =        (history4[2] * mCoefficients[7]) + (history4[3] * mCoefficients[8]);
			c5 =        (history5[2] * mCoefficients[7]) + (history5[3] * mCoefficients[8]);

			dc = -dc;
			length--;
			inbuffer+=6;
			outbuffer+=6; 

			history0[1] = history0[0];
			history0[3] = history0[2];
			history0[0]  = a0;
			history0[2]  = a0 + b0;
			history1[1] = history1[0];
			history1[3] = history1[2];
			history1[0]  = a1;
			history1[2]  = a1 + b1;
			history2[1] = history2[0];
			history2[3] = history2[2];
			history2[0]  = a2;
			history2[2]  = a2 + b2;
			history3[1] = history3[0];
			history3[3] = history3[2];
			history3[0]  = a3;
			history3[2]  = a3 + b3;
			history4[1] = history4[0];
			history4[3] = history4[2];
			history4[0]  = a4;
			history4[2]  = a4 + b4;
			history5[1] = history5[0];
			history5[3] = history5[2];
			history5[0]  = a5;
			history5[2]  = a5 + b5;
            
			outbuffer[-6] = a0 + b0 + c0;    
			outbuffer[-5] = a1 + b1 + c1;    
			outbuffer[-4] = a2 + b2 + c2;    
			outbuffer[-3] = a3 + b3 + c3;    
			outbuffer[-2] = a4 + b4 + c4;    
			outbuffer[-1] = a5 + b5 + c5;    
		}
	}
	else
	{
		while (length)
		{
			for (count = 0; count < (unsigned int)inchannels; count++)
			{
				outbuffer[count] = filter(inbuffer[count], count);
			}

			length--;
			inbuffer += inchannels;
			outbuffer += inchannels;
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


/*
 * ----------------------------------------------------------
 *      bilinear.c
 *
 *      Perform bilinear transformation on s-domain coefficients
 *      of 2nd order biquad section.
 *      First design an analog filter and use s-domain coefficients
 *      as input to szxform() to convert them to z-domain.
 *
 * Here's the butterworth polinomials for 2nd, 4th and 6th order sections.
 *      When we construct a 24 db/oct filter, we take to 2nd order
 *      sections and compute the coefficients separately for each section.
 *
 *      n       Polinomials
 * --------------------------------------------------------------------
 *      2       s^2 + 1.4142s +1
 *      4       (s^2 + 0.765367s + 1) (s^2 + 1.847759s + 1)
 *      6       (s^2 + 0.5176387s + 1) (s^2 + 1.414214 + 1) (s^2 + 1.931852s + 1)
 *
 *      Where n is a filter order.
 *      For n=4, or two second order sections, we have following equasions for each
 *      2nd order stage:
 *
 *      (1 / (s^2 + (1/Q) * 0.765367s + 1)) * (1 / (s^2 + (1/Q) * 1.847759s + 1))
 *
 *      Where Q is filter quality factor in the range of
 *      1 to 1000. The overall filter Q is a product of all
 *      2nd order stages. For example, the 6th order filter
 *      (3 stages, or biquads) with individual Q of 2 will
 *      have filter Q = 2 * 2 * 2 = 8.
 *
 *      The nominator part is just 1.
 *      The denominator coefficients for stage 1 of filter are:
 *      b2 = 1; b1 = 0.765367; b0 = 1;
 *      numerator is
 *      a2 = 0; a1 = 0; a0 = 1;
 *
 *      The denominator coefficients for stage 1 of filter are:
 *      b2 = 1; b1 = 1.847759; b0 = 1;
 *      numerator is
 *      a2 = 0; a1 = 0; a0 = 1;
 *
 *      These coefficients are used directly by the szxform()
 *      and bilinear() functions. For all stages the numerator
 *      is the same and the only thing that is different between
 *      different stages is 1st order coefficient. The rest of
 *      coefficients are the same for any stage and equal to 1.
 *
 *      Any filter could be constructed using this approach.
 *
 *      References:
 *             Van Valkenburg, "Analog Filter Design"
 *             Oxford University Press 1982
 *             ISBN 0-19-510734-9
 *
 *             C Language Algorithms for Digital Signal Processing
 *             Paul Embree, Bruce Kimble
 *             Prentice Hall, 1991
 *             ISBN 0-13-133406-9
 *
 *             Digital Filter Designer's Handbook
 *             With C++ Algorithms
 *             Britton Rorabaugh
 *             McGraw Hill, 1997
 *             ISBN 0-07-053806-9
 * ----------------------------------------------------------
 */

/*
 * ----------------------------------------------------------
 *      Pre-warp the coefficients of a numerator or denominator.
 *      Note that a0 is assumed to be 1, so there is no wrapping
 *      of it.
 * ----------------------------------------------------------
 */
FMOD_RESULT DSPLowPass::prewarp(float *a0, float *a1, float *a2, float fc, float fs)
{
    float wp, pi;

    pi = 4.0f * FMOD_ATAN(1.0f);
    wp = 2.0f * fs * FMOD_TAN(pi * fc / fs);

    *a2 = (*a2) / (wp * wp);
    *a1 = (*a1) / wp;

    return FMOD_OK;
}


/*
 * ----------------------------------------------------------
 * bilinear()
 *
 * Transform the numerator and denominator coefficients
 * of s-domain biquad section into corresponding
 * z-domain coefficients.
 *
 *      Store the 4 IIR coefficients in array pointed by coef
 *      in following order:
 *             beta1, beta2    (denominator)
 *             alpha1, alpha2  (numerator)
 *
 * Arguments:
 *             a0-a2   - s-domain numerator coefficients
 *             b0-b2   - s-domain denominator coefficients
 *             k               - filter gain factor. initially set to 1
 *                                and modified by each biquad section in such
 *                                a way, as to make it the coefficient by
 *                                which to multiply the overall filter gain
 *                                in order to achieve a desired overall filter gain,
 *                                specified in initial value of k.
 *             fs             - sampling rate (Hz)
 *             coef    - array of z-domain coefficients to be filled in.
 *
 * Return:
 *             On return, set coef z-domain coefficients
 * ----------------------------------------------------------
 */
// OFFSET: 0x008F2AA0
FMOD_RESULT DSPLowPass::bilinear(
    float a0, float a1, float a2,    /* numerator coefficients */
    float b0, float b1, float b2,    /* denominator coefficients */
    float *k,                                  /* overall gain factor */
    float fs,                                  /* sampling rate */
    float *coef                                /* pointer to 4 iir coefficients */
)
{
    float ad, bd;

	/* alpha (Numerator in s-domain) */
    ad = 4.0f * a2 * fs * fs + 2.0f * a1 * fs + a0;
	/* beta (Denominator in s-domain) */
    bd = 4.0f * b2 * fs * fs + 2.0f * b1* fs + b0;

	/* update gain constant for this section */
    *k *= ad/bd;

	/* Denominator */
    *coef++ = (float)((2.0f * b0 - 8.0f * b2 * fs * fs)           / bd); /* beta1 */
    *coef++ = (float)((4.0f * b2 * fs * fs - 2.0f * b1 * fs + b0) / bd); /* beta2 */

	/* Nominator */
    *coef++ = (float)((2.0f * a0 - 8.0f * a2 * fs * fs) / ad);			 /* alpha1 */
    *coef   = (float)((4.0f * a2 * fs * fs - 2.0f * a1 * fs + a0) / ad); /* alpha2 */

    return FMOD_OK;
}


/*
 * ----------------------------------------------------------
 * Transform from s to z domain using bilinear transform
 * with prewarp.
 *
 * Arguments:
 *      For argument description look at bilinear()
 *
 *      coef - pointer to array of floating point coefficients,
 *                     corresponding to output of bilinear transofrm
 *                     (z domain).
 *
 * Note: frequencies are in Hz.
 * ----------------------------------------------------------
 */
// OFFSET: 0x008F2B70
FMOD_RESULT DSPLowPass::szxform(
    float *a0, float *a1, float *a2, /* numerator coefficients */
    float *b0, float *b1, float *b2, /* denominator coefficients */
    float fc,         /* Filter cutoff frequency */
    float fs,         /* sampling rate */
    float *k,         /* overall gain factor */
    float *coef)         /* pointer to 4 iir coefficients */
{
        /* Calculate a1 and a2 and overwrite the original values */
        prewarp(a0, a1, a2, fc, fs);
        prewarp(b0, b1, b2, fc, fs);
        bilinear(*a0, *a1, *a2, *b0, *b1, *b2, k, fs, coef);
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
FMOD_RESULT DSPLowPass::setParameterInternal(int index, float value)
{
    if (index == FMOD_DSP_LOWPASS_CUTOFF)
    {
        mCutoffHzUpdate  = value;
    }
    else if (index == FMOD_DSP_LOWPASS_RESONANCE)
    {
        mResonanceUpdate = value;
    }

    return FMOD_OK;;
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
// VERIFY: fed05641 manual
// OFFSET: 0x008F2C00
FMOD_RESULT DSPLowPass::getParameterInternal(int index, float *value, char *valuestr)
{
    switch (index)
    {
        case FMOD_DSP_LOWPASS_CUTOFF:
        {
            *value = mCutoffHzUpdate;
            sprintf(valuestr, "%.02f", mCutoffHzUpdate);
            break;
        }
        case FMOD_DSP_LOWPASS_RESONANCE:
        {
            *value = mResonanceUpdate;
            sprintf(valuestr, "%.02f", mResonanceUpdate);
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
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: ebe12ca3 manual
// OFFSET: 0x008F2C70
FMOD_RESULT DSPLowPass::updateState(float resonance, float cutoff)
{
    FMOD_RESULT result;
    int         outputrate;

#ifdef PLATFORM_PS3_SPU
    outputrate = 48000;
#else
    result = mSystem->getSoftwareFormat(&outputrate, 0, 0, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }
#endif

    if (resonance >= 1.0f)
    {
        int    nInd;
        float  a0, a1, a2, b0, b1, b2;
        float  fs;                          /* Sampling frequency, cutoff frequency */
        float  k[2];                        /* overall gain factor */
        float  ktotal;
        float *coef;

	    k[0]   = 1.0f;                      /* Set overall filter gain */
        k[1]   = 1.0f;
        ktotal = 1.0f;
	    coef   = mCoefficients + 1;         /* Skip k, or gain */
	    fs     = (float)outputrate;         /* Sampling frequency (Hz) */

        /*
         * Compute z-domain coefficients for each biquad section
         * for new Cutoff Frequency and Resonance
         */
        for (nInd = 0; nInd < LOWPASS_FILTER_SECTIONS; nInd++)
        {
            a0 = mProtoCoef[nInd].a0;
            a1 = mProtoCoef[nInd].a1;
            a2 = mProtoCoef[nInd].a2;

            b0 = mProtoCoef[nInd].b0;
            b1 = mProtoCoef[nInd].b1 / resonance;      /* Divide by resonance or Q */
            b2 = mProtoCoef[nInd].b2;
            szxform(&a0, &a1, &a2, &b0, &b1, &b2, (float)cutoff, fs, &k[nInd], coef);

            // Scale the history by the ratio of current gain vs old gain 
            if (mGainFactor[nInd] != 0.0f)
            {
                for(int lout=0; lout < DSP_MAXLEVELS_MAX; lout++)
                {
                    mHistory[lout][nInd] *= k[nInd]/mGainFactor[nInd];
                }
            }
            mGainFactor[nInd] = k[nInd];
            ktotal *= k[nInd];

            coef += 4;                       /* Point to next filter section */
        }

        /* Update overall filter gain in coef array */
        mCoefficients[0] = ktotal;
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
// VERIFY: 693ab094 manual
// OFFSET: 0x008F3730
FMOD_RESULT F_CALLBACK DSPLowPass::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPLowPass *lowpass = (DSPLowPass *)dsp;

    return lowpass->createInternal();
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
// VERIFY: 77600a46 auto
// OFFSET: 0x008F3760
FMOD_RESULT F_CALLBACK DSPLowPass::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPLowPass *lowpass = (DSPLowPass *)dsp;

    return lowpass->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
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
// VERIFY: 6c15eb5c auto
// OFFSET: 0x008F2D90
FMOD_RESULT F_CALLBACK DSPLowPass::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPLowPass *lowpass = (DSPLowPass *)dsp;

    return lowpass->setParameterInternal(index, value);
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
// VERIFY: 9edbeb1d auto
// OFFSET: 0x008F2DD0
FMOD_RESULT F_CALLBACK DSPLowPass::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPLowPass *lowpass = (DSPLowPass *)dsp;

    return lowpass->getParameterInternal(index, value, valuestr);
}


#endif

}

#endif
