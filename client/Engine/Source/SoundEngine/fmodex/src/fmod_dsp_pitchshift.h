#ifndef _FMOD_DSP_PITCHSHIFT_H
#define _FMOD_DSP_PITCHSHIFT_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_PITCHSHIFT

#include "fmod.h"
#include "fmod_dsp_filter.h"


#if !defined(PLATFORM_PS3) && !defined(PLATFORM_WINDOWS_PS3MODE)
    #define DSP_PITCHSHIFT_USECOSTAB
    #define USEWINDOWTAB
#endif

namespace FMOD
{   
    const int DSP_PITCHSHIFT_COSTABBITS = 13;
    const int DSP_PITCHSHIFT_COSTABSIZE = (1 << DSP_PITCHSHIFT_COSTABBITS);
    const int DSP_PITCHSHIFT_TABLERANGE = (DSP_PITCHSHIFT_COSTABSIZE * 4);
    const int DSP_PITCHSHIFT_TABLEMASK  = (DSP_PITCHSHIFT_TABLERANGE - 1);

    class DSPPitchShift;

    #if defined(PLATFORM_PS3) || defined(PLATFORM_WINDOWS_PS3MODE)
    const int MAX_FRAME_LENGTH = 2048;
    const int MAX_FRAME_LENGTH_HALF_BITS = 6; // 2048 = 2^(5.5 * 2) -> round 5.5 up to 6
    #else
    const int MAX_FRAME_LENGTH = 4096;
    const int MAX_FRAME_LENGTH_HALF_BITS = 6; // 2048 = 2^(6 * 2)
    #endif

    class DSPPitchShiftSMB
    {
      public:

#if defined(PLATFORM_PS3) || defined(PLATFORM_WINDOWS_PS3MODE)    	

        float          *mInFIFO;
	    float          *mOutFIFO;
        float          *mLastPhase;
	    float          *mSumPhase;
	    float          *mOutputAccum;
    	float          *mInFIFOMemory;
	    float          *mOutFIFOMemory;
        float          *mLastPhaseMemory;
	    float          *mSumPhaseMemory;
	    float          *mOutputAccumMemory;

        float          *gFFTtable;
        int            *gFFTbitrev;

        DSPPitchShift  *mDSPPitchShift;
        DSPPitchShift  *mDSPPitchShiftMram;
#else  	
        float           mInFIFO     [MAX_FRAME_LENGTH];
	    float           mOutFIFO    [MAX_FRAME_LENGTH];
        float           mLastPhase  [MAX_FRAME_LENGTH/2+4];
	    float           mSumPhase   [MAX_FRAME_LENGTH/2+4];
	    float           mOutputAccum[MAX_FRAME_LENGTH * 2];   
#endif
        float          *mWindow;

	    int             mRover;
        float           mPitchRatioFromLast;                        
        int             mFFTFrameSize;
        int             mFFTFrameBits;
        float          *mCosTab;
        bool            mResetPhaseFlag;

        void            initFft(int fftSize);
        void            bitrv2(float *data, int count);
        void            bitrv2conj(float *data, int count);
        void            cft1st(float *fftBuffer);
        void            cftmdl(float *fftBuffer, int count);
        void            cftfsub(float *fftBuffer);
        void            cftbsub(float *fftBuffer);
        void            fft(float *fftBuffer, int sign);

        void            smbInit();
        void            smbFft(float *fftBuffer, int sign);
        void            smbPitchShift(float pitchShift, int numSampsToProcess, int osamp, float sampleRate, float *indata, float *outdata, int channel, int numchannels);
        void            setResetPhaseFlag();

        FMOD_INLINE const float    smbAtan2(float x, float y);

#ifdef DSP_PITCHSHIFT_USECOSTAB
        FMOD_INLINE const float    cosine(float x);
        FMOD_INLINE const float    sine(float x);
#else
        FMOD_INLINE const float    cosine(float x) { return FMOD_COS(x); }
        FMOD_INLINE const float    sine(float x)   { return FMOD_SIN(x); }
#endif
    };

    class DSPPitchShift : public DSPFilter
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      friend class DSPPitchShiftSMB;

      private:

#ifdef DSP_PITCHSHIFT_USECOSTAB
        float mCosTab[DSP_PITCHSHIFT_COSTABSIZE];
#endif
#ifdef USEWINDOWTAB
        float             mWindow[MAX_FRAME_LENGTH];
#endif
        float             mPitch;
        int               mFFTSize;
        int               mOverlap;
        int               mMaxChannels;

#if defined(PLATFORM_PS3) || defined(PLATFORM_WINDOWS_PS3MODE)

        FMOD_PPCALIGN128(float           mTempLastPhase  [MAX_FRAME_LENGTH/2+4 + 4]);    // 4K
	    FMOD_PPCALIGN128(float           mTempSumPhase   [MAX_FRAME_LENGTH/2+4 + 4]);    // 4K
        FMOD_PPCALIGN128(float           mTempBuffer0    [MAX_FRAME_LENGTH * 2]);        // 16k
        float          *mTempBuffer1;

        float          *mSynFreq;
        float          *mSynMagn;
        float          *mAnaFreq;
        float          *mAnaMagn;
        float          *mSynFreqMemory;
        float          *mSynMagnMemory;
        float          *mAnaFreqMemory;
        float          *mAnaMagnMemory;

        #ifdef FASTER_DFT
        float           mFFTtable[MAX_FRAME_LENGTH / 2];
        int             mFFTbitrev[2 + (1<<MAX_FRAME_LENGTH_HALF_BITS)];
        #endif

        DSPPitchShiftSMB  mPitchShift[8];
#else
        DSPPitchShiftSMB *mPitchShift;
#endif
        int               mOutputRate;
        int               mChannels;
        int               mFFTFrameBits;

        FMOD_RESULT     createInternal();
        FMOD_RESULT     releaseInternal();
        FMOD_RESULT     resetInternal();
        FMOD_RESULT     readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        FMOD_RESULT     setParameterInternal(int index, float value);
        FMOD_RESULT     getParameterInternal(int index, float *value, char *valuestr);

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
