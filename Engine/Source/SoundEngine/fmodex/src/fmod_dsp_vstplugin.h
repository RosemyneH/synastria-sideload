#ifndef _FMOD_DSP_VSTPLUGIN_H
#define _FMOD_DSP_VSTPLUGIN_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_VSTPLUGIN

#include "fmod.h"
#include "fmod_dsp_filter.h"
#include "fmod_os_misc.h"

#include "../lib/vst/AEffect.h"
#include "../lib/vst/aeffectx.h"


namespace FMOD
{   
    class DSPVSTPlugin : public DSPFilter
    {
      private:

        static float   *mDeInterleavedInputBuffer[DSP_MAXLEVELS_MAX];
        static float   *mDeInterleavedOutputBuffer[DSP_MAXLEVELS_MAX];
        static int      mDeInterleavedBufferCount;

        FMOD_RESULT     createInternal();
        FMOD_RESULT     releaseInternal();
        FMOD_RESULT     resetInternal();
        FMOD_RESULT     readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        FMOD_RESULT     setParameterInternal(int index, float value);
        FMOD_RESULT     getParameterInternal(int index, float *value, char *valuestr);
        FMOD_RESULT     showConfigDialogInternal(void *hwnd, int show);

      public:
        
        AEffect        *mEffect;

        static long VSTCALLBACK audioMasterCB(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

        static FMOD_DSP_DESCRIPTION_EX *getDescriptionEx();        

        static FMOD_RESULT F_CALLBACK createCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK releaseCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK resetCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        static FMOD_RESULT F_CALLBACK setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value);
        static FMOD_RESULT F_CALLBACK getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr);
        static FMOD_RESULT F_CALLBACK configCallback(FMOD_DSP_STATE *dsp, void *hwnd, int show);
    };
}

#endif

#endif  

