#ifndef __ASFXDSP_H
#define __ASFXDSP_H

#include "../../../src/fmod_settings.h"
#include "../../../src/fmod_types.h"

#include <math.h>

struct DspValues
{
	short	reinitialize;			// 0 means in progress. 1 means settings have changed, start over
};

struct coeff2ndorder
{
    float a0, a1, a2, b1, b2;
};

#define kNumLateReverbDelays				8			
// Only feed the first 8 or so late reverb inputs at first?
#define	kNumMatrixStages					3			// 2 ^ kNumMatrixStages should equal kNumLateReverbDelays
#define kNumEarly_EarlyLateDelayTaps		1			
#define kNumLate_EarlyLateDelayTaps			8			// Should equal kNumLateReverbDelays
#define kNumEarlyLateDelayTaps				(kNumEarly_EarlyLateDelayTaps + kNumLate_EarlyLateDelayTaps)
#define kNumEarlyDelayTaps					7
#define	kNumAllpassDelays					2

#define kSFXREVERB_MAXCHANNELS 8

#define kEarlyLateDelayLen					65536		// Delete
#define	kEarlyDelayLenSamples				16384		// Delete
#define	kAllpassLengthSamples				2048		// Delete

// Main late delays
#define LOWEST_DELAY_LINE_LENGTH_SEC		(0.061f)
#define	DELAY_LINE_RATIO					(1.32f)		
#define LOWEST_DELAY_LINE_LENGTH_B_SEC		(0.0015f)
#define DELAY_LINE_RATIO_B					(1.47f)

// Allpass delays
#define	kAllpassDelayLenSec0				(0.0059f)
#define kAllpassDelayLenSec1				(0.0077f)
	
// EarlyLate delays
#define	kEarlyLateNextLengthSec				(0.0187f)
#define	kEarlyLateDelayRatio				(1.29f)	
	
// Early delays
#define	kEarlyFirstDelaySec					(0.005f)
#define kEarlyDelayRatio					(1.59f)

// Error codes
#define	REVERB_ALLPASS_ALLOCATION_ERR		4500
#define REVERB_EARLYLATE_ALLOCATION_ERR		4501
#define REVERB_EARLY_ALLOCATION_ERR			4502
#define	REVERB_LATE_ALLOCATION_ERR			4503
#define REVERB_PARAMETER_ERROR              4504
#define REVERB_INBUFF_ALLOCATION_ERR        4505

namespace FMOD
{
    class SystemI;
}

class ASfxDsp
{
public:
  	int     init(float rate);
  	void    close();

	bool UpdateBufferSize(int newMaxBlockSize);	// possibly reallocate mInBuff buffers to new MaxBlockSize
	void ClearBuffers();	// zero the audio buffers
	void ClearInBuff();
	void ClearReverbInternalBuffers();
	int	AllocateEarlyLateDelay(float *delaySec, float sampleRate);
	int AllocateAllpassDelays(int numDelays, float *delaySec, float sampleRate);
	int AllocateEarlyDelay(float delaySec, float sampleRate);
	int AllocateLateDelays(int numDelays, float *delaySec, float sampleRate);
	int	 NextPowerOf2(int delaySamples);
	
  	void DoDSPProcessing (float *inAudio, float *outAudio, int channels, unsigned int sampleframes, float rate, float drymix);
	void BlockProcessInput(unsigned int sampframes, int channels, float *inAudio, float rate);

public:	
	void SetAllpassDelays(float rate);
	void SetLate_EarlyLateDelayTaps(float refPlusRevDelaySec, float nextLengthSec, float delayRatio, float rate);	
	void SetEarlyDelay(float nextLengthSec, float delayRatio, float rate);
	void SetLateDelays(float nextLengthSec, float delayRatio, float nextLengthBSec, float delayRatioB, float rate);
	void DeallocateLateDelays();
	void DeallocateAllpassDelays();
	void DeallocateEarlyLateDelay();
	void DeallocateEarlyDelay();
	void ZeroWritePointers();

    float	*mInBuff;
    
	int		mNumAllocatedInBuffSamples;	// the number of samples per mInBuff currently allocated 
										// (varies according to MaxBlockSize)

    #if defined(PLATFORM_PS3) //|| defined(PLATFORM_WINDOWS_PS3MODE)
    float   FMOD_PPCALIGN16(mTempbuff[(9*256) + 4]);  // 8k
    #endif

	float	mRoomHF;
	bool	mRoomHFChanged;
	float	mLatchRoomHF;				// Latch it for safety
	float	mPrevRoomHF;				// For our interpolation
	float	mOldRoomHF;					// For next time
	float	mzRoomHF0, mzRoomHF1;

    struct  coeff2ndorder mRoomLFcoeffs;
    float   mRoomLF;
	float	mzRoomLF0, mzRoomLF1;

	float	FMOD_PPCALIGN16(mDecayHF[kNumLateReverbDelays]);
	bool	FMOD_PPCALIGN16(mDecayHFChanged[kNumLateReverbDelays]);
	float	FMOD_PPCALIGN16(mLatchDecayHF[kNumLateReverbDelays]);		    // Latch it for safety.
	float	FMOD_PPCALIGN16(mLatchOneMinusDecayHF[kNumLateReverbDelays]);
	float	FMOD_PPCALIGN16(mPrevDecayHF[kNumLateReverbDelays]);        	// Latch previous one
	float	FMOD_PPCALIGN16(mOldDecayHF[kNumLateReverbDelays]);  		    // Save current one for next time...
	float	FMOD_PPCALIGN16(mzDecayHF[kNumLateReverbDelays]);
	float	FMOD_PPCALIGN16(mMatrix[kNumMatrixStages * kNumLateReverbDelays]);
	float	FMOD_PPCALIGN16(mFeedback[kNumLateReverbDelays]);
	float	FMOD_PPCALIGN16(mLatchFeedback[kNumLateReverbDelays]);	// Latch it for safety
	float	FMOD_PPCALIGN16(mOldFeedback[kNumLateReverbDelays]);	// Save for next buffer
	float	FMOD_PPCALIGN16(mPrevFeedback[kNumLateReverbDelays]);	// Use previous value for crossfade

    float	mHadamard;
	bool	mHadamardChanged;
	float	mLatchHadamard;				// Latch it for safety
	float	mPrevHadamard;				// For our interpolation
	float	mOldHadamard;			    // For next time

	float	mERgain, mLRgain;
	bool	mLRgainChanged;
	float	mLatchLRgain;				// Latch it for safety
	float	mPrevLRgain;				// For our interpolation
	float	mOldLRgain;					// For next time
	bool	mERgainChanged;
	float	mLatchERgain;				// Latch it for safety
	float	mPrevERgain;				// For our interpolation
	float	mOldERgain;					// For next time
    
    float	mDiffusionScale;
	bool	mDiffusionScaleChanged;
	float	mLatchDiffusionScale;		// Latch it for safety
	float	mPrevDiffusionScale;		// For our interpolation
	float	mOldDiffusionScale; 			// For next time

	float	mAllpassGain;
	int		mNumLateReverbDelays;
	
	// Late delays
	float **mLateDelays;                 // Array of delay lines
	float	FMOD_PPCALIGN16(mLateDelayLenSec[kNumLateReverbDelays]);            // Seconds
	int		FMOD_PPCALIGN16(mLateDelayLenSamples[kNumLateReverbDelays]);    	// Samples used
	int		FMOD_PPCALIGN16(mOldLateDelayLenSamples[kNumLateReverbDelays]);
    int		FMOD_PPCALIGN16(mLateDelayLenBSamples[kNumLateReverbDelays]);   	// Samples used
	int		FMOD_PPCALIGN16(mLateDelaySamplesAllocated[kNumLateReverbDelays]);  // Samples allocated
	bool	FMOD_PPCALIGN16(mLateChanged[kNumLateReverbDelays]);            	// Either the delay lengths or mFeedback changed
	int		FMOD_PPCALIGN16(mLateMask[kNumLateReverbDelays]);
	int		FMOD_PPCALIGN16(mLateWritePointer[kNumLateReverbDelays]);
	int		FMOD_PPCALIGN16(mLateReadPointer[kNumLateReverbDelays]);
	int		FMOD_PPCALIGN16(mOldLateReadPointer[kNumLateReverbDelays]);
    int		FMOD_PPCALIGN16(mLateReadPointerB[kNumLateReverbDelays]);           // Short (~10 ms) reads to avoid cancellations

	// EarlyLate delay
	float  *FMOD_PPCALIGN16(mEarlyLateDelay);
    float  *FMOD_PPCALIGN16(mEarlyLateDelayMemory);
	float	FMOD_PPCALIGN16(mEarlyLateSec[kNumEarlyLateDelayTaps]);
	int		FMOD_PPCALIGN16(mEarlyLateSamples[kNumEarlyLateDelayTaps]);
	int		FMOD_PPCALIGN16(mOldEarlyLateSamples[kNumEarlyLateDelayTaps]);
	bool	FMOD_PPCALIGN16(mEarlyLateChanged[kNumEarlyLateDelayTaps]);
    int		FMOD_PPCALIGN16(mEarlyLateSamplesAllocated);
	int		FMOD_PPCALIGN16(mEarlyLateMask);
	int		FMOD_PPCALIGN16(mEarlyLateWritePointer);
	int		FMOD_PPCALIGN16(mEarlyLateReadPointer[kNumEarlyLateDelayTaps]);
	int		FMOD_PPCALIGN16(mOldEarlyLateReadPointer[kNumEarlyLateDelayTaps]);
            
	// Early delay
	#if defined(PLATFORM_PS3) //|| defined(PLATFORM_WINDOWS_PS3MODE)
	float	FMOD_PPCALIGN16(mEarlyDelay[4096]);
    #else
    float	*mEarlyDelay;
    #endif
	float	FMOD_PPCALIGN16(mEarlyDelayLenSec[kNumEarlyDelayTaps]);
	int		FMOD_PPCALIGN16(mEarlyDelayLenSamples[kNumEarlyDelayTaps]);
	int		FMOD_PPCALIGN16(mEarlyDelaySamplesAllocated);
	int		FMOD_PPCALIGN16(mEarlyMask);
	int		FMOD_PPCALIGN16(mEarlyWritePointer);
	int		FMOD_PPCALIGN16(mEarlyReadPointer[kNumEarlyDelayTaps]);
	
	// Allpass delays
    float	**mAllpassDelays;
	float	FMOD_PPCALIGN16(mAllpassDelayLenSec[kNumAllpassDelays]);
	int		FMOD_PPCALIGN16(mAllpassDelayLenSamples[kNumAllpassDelays]);
	int		FMOD_PPCALIGN16(mAllpassSamplesAllocated[kNumAllpassDelays]);
	int		FMOD_PPCALIGN16(mAllpassMask[kNumAllpassDelays]);
	int		FMOD_PPCALIGN16(mAllpassWritePointer[kNumAllpassDelays]);
	int		FMOD_PPCALIGN16(mAllpassReadPointer[kNumAllpassDelays]);

	int		mNumMatrixStages;
};


#endif	// __ASFXDSP_H

