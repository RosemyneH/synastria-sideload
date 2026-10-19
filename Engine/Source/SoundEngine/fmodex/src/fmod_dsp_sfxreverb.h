#ifndef _FMOD_DSP_SFXREVERB_H
#define _FMOD_DSP_SFXREVERB_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SFXREVERB

#include "fmod.h"
#include "fmod_dsp_filter.h"
#include "3dl2.h"
#include "aSfxDsp.hpp"

namespace FMOD
{
    class DSPSfxReverb : public DSPFilter
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

    private:
        I3DL2_LISTENERPROPERTIES   mProps;             // Reverb properties
        I3DL2_LISTENERPROPERTIES   mUpdateProps;       // Update properties
        bool                       mUpdate;
        float                      mDryLevelmB;        // Dry level (mB)
        float                      mDryLevelLin;       // Dry level (linear)
        int                        mRoomLF;
        float                      mLFReference;
        int                        mRoomLFUpdate;
        float                      mLFReferenceUpdate;
        ASfxDsp                   *mpSfxDsp;           // SFX DSP module
        int                        mOutputRate;        // Sample rate

        FMOD_RESULT     createInternal();
        FMOD_RESULT     releaseInternal();
        FMOD_RESULT     resetInternal();
        FMOD_RESULT     readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        FMOD_RESULT     setParameterInternal(int index, float value);
        FMOD_RESULT     getParameterInternal(int index, float *value, char *valuestr);

    private:
        bool SetRoom(I3DL2_LISTENERPROPERTIES *pProps);                     // [-10000, 0]      default: -10000 mB
        bool SetRoomHF(I3DL2_LISTENERPROPERTIES *pProps);                   // [-10000, 0]      default: 0 mB
        bool SetRoomLF(int roomLF);						                    // [-10000, 0]      default: 0 mB
        bool Calculate1stOrderLowpassCoeff(float gain, float cutoff, float sampleRate, float *pK);
        bool CalculateShelfCoeffs(float gain, float cutoff, float sampleRate, float *a0, float *a1, float *a2, float *b1, float *b2);
        bool SetRoomRolloffFactor(I3DL2_LISTENERPROPERTIES *pProps);        // [0.0, 10.0]      default: 0.0
        bool SetDecayTime(I3DL2_LISTENERPROPERTIES *pProps);                // [0.1, 20.0]      default: 1.0 s
        bool SetDelayLineLengths(I3DL2_LISTENERPROPERTIES *pProps);
        bool SetDecayHFRatio(I3DL2_LISTENERPROPERTIES *pProps);             // [0.1, 2.0]       default: 0.5
        bool SetReflectionsLevel(I3DL2_LISTENERPROPERTIES *pProps);         // [-10000, 1000]   default: -10000 mB
        bool SetReflectionsDelay(I3DL2_LISTENERPROPERTIES *pProps);         // [0.0, 0.3]       default: 0.02 s
        bool SetReverbLevel(I3DL2_LISTENERPROPERTIES *pProps);              // [-10000, 2000]   default: -10000 mB
        bool SetReverbDelay(I3DL2_LISTENERPROPERTIES *pProps);              // [0.0, 0.1]       default: 0.04 s
        bool SetDiffusion(I3DL2_LISTENERPROPERTIES *pProps);                // [0.0, 100.0]     default: 100.0 %
        bool SetHFReference(I3DL2_LISTENERPROPERTIES *pProps);              // [20.0, 20000.0]  default: 5000.0 Hz
        bool SetLFReference(float lfReference);                    // [20.0, 20000.0]  default: 5000.0 Hz
        bool SetDensity(I3DL2_LISTENERPROPERTIES *pProps);                  // [0.0, 100.0]     default: 100.0 %

    public:
        static FMOD_DSP_DESCRIPTION_EX *getDescriptionEx();        

        static FMOD_RESULT F_CALLBACK createCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK releaseCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK resetCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        static FMOD_RESULT F_CALLBACK setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value);
        static FMOD_RESULT F_CALLBACK getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr);
    };
}

#endif

#endif  

