#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_FREEVERB

#include "fmod.h"
#include "fmod_dspi.h"
#include "fmod_dsp_reverb.h"
#include "fmod_systemi.h"

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dspreverb;

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
        return DSPReverb::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


FMOD_DSP_PARAMETERDESC dspreverb_param[6] =
{
    {    0.0f,    1.0f,    0.5f,  "Roomsize", "", "Roomsize.  0.0 to 1.0.  Default = 0.5" },
    {    0.0f,    1.0f,    0.5f,  "Damp",     "", "Damp.  0.0 to 1.0.  Default = 0.5" },
    {    0.0f,    1.0f,    0.33f, "Wet",      "", "Wet mix.  0.0 to 1.0.  Default = 0.33" },
    {    0.0f,    1.0f,    0.66f, "Dry",      "", "Dry mix.  0.0 to 1.0.  Default = 0.66" },
    {    0.0f,    1.0f,    1.0f,  "Width",    "", "Width.  0.0 to 1.0.  Default = 1.0" },
    {    0.0f,    1.0f,    0.0f,  "Mode",     "", "Mode.  0, 1.  Default = 0" }
};


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
// VERIFY: 9bb548f5 auto
// OFFSET: 0x008EB900
FMOD_DSP_DESCRIPTION_EX *DSPReverb::getDescriptionEx()
{
    FMOD_memset(&dspreverb, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    FMOD_strcpy(dspreverb.name, "FMOD Reverb");
    dspreverb.version       = 0x00010100;
    dspreverb.create        = DSPReverb::createCallback;
    dspreverb.release       = DSPReverb::releaseCallback;
    dspreverb.reset         = DSPReverb::resetCallback;
    dspreverb.read          = DSPReverb::readCallback;

    dspreverb.numparameters = 6;
    dspreverb.paramdesc     = dspreverb_param;
    dspreverb.setparameter  = DSPReverb::setParameterCallback;
    dspreverb.getparameter  = DSPReverb::getParameterCallback;

    dspreverb.mType         = FMOD_DSP_TYPE_REVERB;
    dspreverb.mCategory     = FMOD_DSP_CATEGORY_FILTER;
    dspreverb.mSize         = sizeof(DSPReverb);

    return &dspreverb;
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
// VERIFY: 24a21389 auto
// OFFSET: 0x008EB3E0
FMOD_RESULT DSPReverb::createInternal()
{
    int count;

    init();

    new (&mReverb) revmodel;

    for (count = 0; count < mDescription.numparameters; count++)
    {
        FMOD_RESULT result;
        
        result = setParameter(count, mDescription.paramdesc[count].defaultval);
        if (result != FMOD_OK)
        {
            return result;
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
FMOD_RESULT DSPReverb::releaseInternal()
{
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
FMOD_RESULT DSPReverb::resetInternal()
{
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
FMOD_RESULT DSPReverb::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{   
    float *inleft = inbuffer;
    float *inright = inbuffer + 1;

    float *outleft = outbuffer;
    float *outright = outbuffer + 1;

    if (inchannels > 2)
    {
        // Prevent output buffer crosstalk by ensuring every sample is set
        FMOD_memcpy(outbuffer, inbuffer, inchannels*length*sizeof(float));
    }
    
    mReverb.processreplace(inleft, inright, outleft, outright, length, inchannels);

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
// VERIFY: 352133d3 auto
// OFFSET: 0x008EB440
FMOD_RESULT DSPReverb::setParameterInternal(int index, float value)
{
    mSystem->lockDSP();

    switch (index)
    {
        case FMOD_DSP_REVERB_ROOMSIZE:
        {
            if (value > 1.0f)
            {
                value = 1.0f;
            }
            else if (value < 0.0f)
            {
                value = 0.0f;
            }
            mReverb.setroomsize(value);
            break;
        }
        case FMOD_DSP_REVERB_DAMP:
        {
            if (value > 1.0f)
            {
                value = 1.0f;
            }
            else if (value < 0.0f)
            {
                value = 0.0f;
            }
            mReverb.setdamp(value);
            break;
        }
        case FMOD_DSP_REVERB_WETMIX:
        {
            if (value > 1.0f)
            {
                value = 1.0f;
            }
            else if (value < 0.0f)
            {
                value = 0.0f;
            }
            mReverb.setwet(value);
            break;
        }
        case FMOD_DSP_REVERB_DRYMIX:
        {
            if (value > 1.0f)
            {
                value = 1.0f;
            }
            else if (value < 0.0f)
            {
                value = 0.0f;
            }
            mReverb.setdry(value);
            break;
        }
        case FMOD_DSP_REVERB_WIDTH:
        {
            if (value > 1.0f)
            {
                value = 1.0f;
            }
            else if (value < 0.0f)
            {
                value = 0.0f;
            }
            mReverb.setwidth(value);
            break;
        }
        case FMOD_DSP_REVERB_MODE:
        {
            if (value >= freezemode)
            {
                value = 1.0f;
            }
            else
            {
                value = 0.0f;
            }
            mReverb.setmode(value);
            break;
        }
    }

    mSystem->unlockDSP();

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
// VERIFY: e5c5ae80 manual
// OFFSET: 0x008EB680
FMOD_RESULT DSPReverb::getParameterInternal(int index, float *value, char *valuestr)
{
    switch (index)
    {
        case FMOD_DSP_REVERB_ROOMSIZE:
        {
            *value = mReverb.getroomsize();
            sprintf(valuestr, "%0.2f", *value);
            break;
        }
        case FMOD_DSP_REVERB_DAMP:
        {
            *value = mReverb.getdamp();
            sprintf(valuestr, "%0.2f", *value);
            break;
        }
        case FMOD_DSP_REVERB_WETMIX:
        {
            *value = mReverb.getwet();
            sprintf(valuestr, "%0.2f", *value);
            break;
        }
        case FMOD_DSP_REVERB_DRYMIX:
        {
            *value = mReverb.getdry();
            sprintf(valuestr, "%0.2f", *value);
            break;
        }
        case FMOD_DSP_REVERB_WIDTH:
        {
            *value = mReverb.getwidth();
            sprintf(valuestr, "%0.2f", *value);
            break;
        }
        case FMOD_DSP_REVERB_MODE:
        {
            *value = mReverb.getmode();
            if (*value >= freezemode)
            {
                *value = 1.0f;
                sprintf(valuestr, "FREEZE");
            }
            else
            {
                *value = 0.0f;
                sprintf(valuestr, "NORMAL");
            }
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



/*
    ==============================================================================================================

    CALLBACK INTERFACE

    ==============================================================================================================
*/


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
// VERIFY: 55804a62 manual
// OFFSET: 0x008EB7F0
FMOD_RESULT F_CALLBACK DSPReverb::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPReverb *reverb = (DSPReverb *)dsp;

    return reverb->createInternal();
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
FMOD_RESULT F_CALLBACK DSPReverb::releaseCallback(FMOD_DSP_STATE *dsp)
{
    DSPReverb *reverb = (DSPReverb *)dsp;

    return reverb->releaseInternal();
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
FMOD_RESULT F_CALLBACK DSPReverb::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPReverb *reverb = (DSPReverb *)dsp;

    return reverb->resetInternal();
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
// VERIFY: 7dde89f7 manual
// OFFSET: 0x008EB820
FMOD_RESULT F_CALLBACK DSPReverb::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPReverb *reverb = (DSPReverb *)dsp;

    return reverb->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
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
// VERIFY: 240e89bf manual
// OFFSET: 0x008EB890
FMOD_RESULT F_CALLBACK DSPReverb::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPReverb *reverb = (DSPReverb *)dsp;

    return reverb->setParameterInternal(index, value);
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
// VERIFY: b9219c87 auto
// OFFSET: 0x008EB8D0
FMOD_RESULT F_CALLBACK DSPReverb::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPReverb *reverb = (DSPReverb *)dsp;

    return reverb->getParameterInternal(index, value, valuestr);
}



}

#endif
