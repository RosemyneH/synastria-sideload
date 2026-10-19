#include <stdio.h>

#include "aSfxDsp.hpp"
#include "3dl2.h"

#include <stdlib.h>
#include <string.h>	// for memmove and FMOD_memcpy

#include "../../../src/fmod_memory.h"
#include "../../../src/fmod_types.h"

#define ADD_ANTI_DENORMALS_TO_INPUT

#define SfxDataMove(source, dest, bytes)	memmove(dest, source, (unsigned)bytes)

//-----------------------------------------------------------------------------

// VERIFY: 0c87e1df manual
// OFFSET: 0x009287F0
int ASfxDsp::init(float rate)
{
	int delayLine;
	int error = 0;

	mLateDelays = NULL;
	mEarlyLateDelay = NULL;
	mEarlyDelay = NULL;
	mAllpassDelays = NULL;

	ZeroWritePointers();
	
	mEarlyLateSec[0] = 0.040f;
	mEarlyLateSamples[0] = (int)(mEarlyLateSec[0] * rate);
	SetLate_EarlyLateDelayTaps(0.060f, kEarlyLateNextLengthSec, kEarlyLateDelayRatio, rate);


    for (delayLine=0; delayLine<kNumEarlyLateDelayTaps; delayLine++)
	{
		mOldEarlyLateSamples[delayLine] = mEarlyLateSamples[delayLine];
	}
	for (delayLine=0; delayLine<kNumLateReverbDelays; delayLine++)
	{
		mLateMask[delayLine] = 0;
    }

	SetEarlyDelay(kEarlyFirstDelaySec, kEarlyDelayRatio, rate);
	SetAllpassDelays(rate);	
	SetLateDelays(LOWEST_DELAY_LINE_LENGTH_SEC, DELAY_LINE_RATIO, LOWEST_DELAY_LINE_LENGTH_B_SEC, DELAY_LINE_RATIO_B, rate);

	for (delayLine=0; delayLine<kNumLateReverbDelays; delayLine++)
	{
		mOldLateDelayLenSamples[delayLine] = mLateDelayLenSamples[delayLine];
		mLateChanged[delayLine] = false;
		mDecayHFChanged[delayLine] = false;
    }

	if (error = AllocateLateDelays(kNumLateReverbDelays, mLateDelayLenSec, rate))	// assignment, not comparison!
	{
		return error;
	}
	
	if (error = AllocateEarlyLateDelay(mEarlyLateSec, rate))
	{
		return error;
	}
	
	if (error = AllocateAllpassDelays(kNumAllpassDelays, mAllpassDelayLenSec, rate))
	{
		return error;
	}
	
	if (error = AllocateEarlyDelay(mEarlyDelayLenSec[kNumEarlyDelayTaps-1], rate))
	{
		return error;
	}

	mNumAllocatedInBuffSamples = 0;
	
	mInBuff = 0;
			
	for (delayLine=0; delayLine<kNumLateReverbDelays; delayLine++)
	{
		mOldFeedback[delayLine] = mPrevFeedback[delayLine] = mLatchFeedback[delayLine] = mFeedback[delayLine] = 0.32f;	
		mLatchDecayHF[delayLine] = mPrevDecayHF[delayLine] = mOldDecayHF[delayLine] = mDecayHF[delayLine] = 0.40f;			
		mLatchOneMinusDecayHF[delayLine] = 1.0f - mDecayHF[delayLine];
	}

	mOldHadamard = mPrevHadamard = mLatchHadamard = mHadamard = 1.0f;  
	mOldDiffusionScale = mPrevDiffusionScale = mLatchDiffusionScale = mDiffusionScale = 1.0f;  
	mOldRoomHF = mPrevRoomHF = mLatchRoomHF = mRoomHF = 0.98f;	
	mRoomHFChanged = false;	
	
    //FMOD_memset(&mOldRoomLF, 0, sizeof(struct coeff2ndorder));
    //FMOD_memset(&mPrevRoomLF, 0, sizeof(struct coeff2ndorder));
    //FMOD_memset(&mLatchRoomLF, 0, sizeof(struct coeff2ndorder));
    FMOD_memset(&mRoomLFcoeffs, 0, sizeof(struct coeff2ndorder));
	// mOldRoomLF.a0 = mPrevRoomLF.a0 = mLatchRoomLF.a0 = 
    mRoomLFcoeffs.a0 = 1.0f;	
    mRoomLF = 0.0f;
	//mRoomLFChanged = false;	

	mDiffusionScale = 0.25f;	// 0.3535;	// cos^4(PI/4)
	mAllpassGain = 0.63f;	// ?
	mPrevLRgain = mOldLRgain = mLatchLRgain = mLRgain = 0.0f;	// ?
	mLRgainChanged = false;
	mPrevERgain = mOldERgain = mLatchERgain = mERgain = 0.0f;		// ?
	mERgainChanged = false;
	ClearReverbInternalBuffers();
	
	return 0;
}

// VERIFY: ff701bf7 auto
// OFFSET: 0x00927560
void ASfxDsp::close()
{
	if (mInBuff)
	{
		FMOD_Memory_Free(mInBuff);
	}
	mInBuff = 0;

	DeallocateLateDelays();
	DeallocateEarlyLateDelay();
    DeallocateAllpassDelays();
    DeallocateEarlyDelay();
}


// VERIFY: 600309cf manual
// OFFSET: 0x00927600
int ASfxDsp::AllocateLateDelays(int numDelays, float *delaySec, float sampleRate)
{
	int delaySamples;
	
	DeallocateLateDelays();		// *** But if it hasn't changed, we may want to skip all this... ***

	if (!mLateDelays)
	{
		if ((mLateDelays = (float**) FMOD_Memory_Calloc(kNumLateReverbDelays * sizeof(float *))) == NULL)
		{
			return REVERB_LATE_ALLOCATION_ERR;
		}
	}

	for (int delayLine=0; delayLine<kNumLateReverbDelays; delayLine++)
	{
		delaySamples = (int)(delaySec[delayLine] * sampleRate) + 1;
	
		delaySamples = NextPowerOf2(delaySamples);

		// Allocate delay
		
		if ((mLateDelays[delayLine] = (float*) FMOD_Memory_Calloc(delaySamples * sizeof(float))) == NULL)
		{
			return REVERB_LATE_ALLOCATION_ERR;
		}
		
		mLateDelaySamplesAllocated[delayLine] = delaySamples;
		mLateMask[delayLine] = delaySamples - 1;
		mLateWritePointer[delayLine] = 0;	
	}
	
	return 0;
}

// VERIFY: be8475b9 manual
// OFFSET: 0x00927740
int ASfxDsp::AllocateEarlyLateDelay(float *delaySec, float sampleRate)
{
	// Allocate it to the max Ref & Rev delays, so we don't have to reallocate it if the delays change.
	float maxDelaySec = I3DL2LISTENER_MAXREFLECTIONSDELAY + I3DL2LISTENER_MAXREVERBDELAY
				+ delaySec[kNumEarlyLateDelayTaps-1] - delaySec[kNumEarly_EarlyLateDelayTaps];

	int delaySamples = (int)(maxDelaySec * sampleRate) + 1;
	
	delaySamples = NextPowerOf2(delaySamples);
	
	DeallocateEarlyLateDelay();		// *** But if it hasn't changed, we may want to skip all this... ***
	
	// Allocate delay
	
	if ((mEarlyLateDelay = (float*) FMOD_Memory_Calloc(delaySamples * sizeof(float))) == NULL)
	{
		return REVERB_EARLYLATE_ALLOCATION_ERR;
	}
	
	mEarlyLateSamplesAllocated = delaySamples;
	mEarlyLateMask = delaySamples - 1;
	
	return 0;
}

// VERIFY: 6ce7917f manual
// OFFSET: 0x00927810
int ASfxDsp::AllocateAllpassDelays(int numDelays, float *delaySec, float sampleRate)
{
	int		delaySamples;
	
	DeallocateAllpassDelays();		// *** But if it hasn't changed, we may want to skip all this... ***

	if (!mAllpassDelays)
	{
		if ((mAllpassDelays = (float**) FMOD_Memory_Calloc(kNumAllpassDelays * sizeof(float *))) == NULL)
		{
			return REVERB_ALLPASS_ALLOCATION_ERR;
		}
	}

	for (int delayLine=0; delayLine<kNumAllpassDelays; delayLine++)
	{
		delaySamples = (int)(delaySec[delayLine] * sampleRate) + 1;
		delaySamples = NextPowerOf2(delaySamples);

		// Allocate delay
		
		if ((mAllpassDelays[delayLine] = (float*) FMOD_Memory_Calloc(delaySamples * sizeof(float))) == NULL)
		{
			return REVERB_ALLPASS_ALLOCATION_ERR;
		}
		
		mAllpassSamplesAllocated[delayLine] = delaySamples;
		mAllpassMask[delayLine] = delaySamples - 1;
		mAllpassWritePointer[delayLine] = 0;		// Do this here?
	}
	
	return 0;
}

// VERIFY: 7642805b manual
// OFFSET: 0x00927950
int ASfxDsp::AllocateEarlyDelay(float delaySec, float sampleRate)
{
	int delaySamples = (int)(delaySec * sampleRate) + 1;
	
	delaySamples = NextPowerOf2(delaySamples);

	DeallocateEarlyDelay();		// *** But if it hasn't changed, we may want to skip all this... ***
	
	// Allocate delay
	
	if ((mEarlyDelay = (float*) FMOD_Memory_Calloc(delaySamples * sizeof(float))) == NULL)
	{
		return REVERB_EARLY_ALLOCATION_ERR;
	}
	
	mEarlyDelaySamplesAllocated = delaySamples;
	mEarlyMask = delaySamples - 1;
	
	return 0;
}

int	ASfxDsp::NextPowerOf2(int delaySamples)
{
	return 1 << ((int)(FMOD_LOG((float)delaySamples) / FMOD_LOG(2.0f)) + 1);
}


// VERIFY: 798766cc auto
// OFFSET: 0x009270B0
void ASfxDsp::DeallocateLateDelays()
{
	if (mLateDelays)
	{
		for (int delayLine=0; delayLine<kNumLateReverbDelays; delayLine++)
		{
			if (mLateDelays[delayLine])
			{
				FMOD_Memory_Free(mLateDelays[delayLine]);
			}

			mLateDelays[delayLine] = NULL;
		}

		FMOD_Memory_Free(mLateDelays);
	}

	mLateDelays = NULL;
}

// VERIFY: 3364aaa0 auto
// OFFSET: 0x00927130
void ASfxDsp::DeallocateAllpassDelays()
{
	if (mAllpassDelays)
	{
		for (int delayLine=0; delayLine<kNumAllpassDelays; delayLine++)
		{
			if (mAllpassDelays[delayLine])
			{
				FMOD_Memory_Free(mAllpassDelays[delayLine]);
			}

			mAllpassDelays[delayLine] = NULL;
		}

		FMOD_Memory_Free(mAllpassDelays);
	}

	mAllpassDelays = NULL;
}

void ASfxDsp::DeallocateEarlyDelay()
{
	if (mEarlyDelay)
	{
		FMOD_Memory_Free(mEarlyDelay);
	}
	
	mEarlyDelay = NULL;
}

void ASfxDsp::DeallocateEarlyLateDelay()
{
	if (mEarlyLateDelay)
	{
		FMOD_Memory_Free(mEarlyLateDelay);
	}
	
	mEarlyLateDelay = NULL;
}

//-----------------------------------------------------------------------------
//#pragma export(UpdateBufferSize)
// VERIFY: 72597f31 auto
// OFFSET: 0x009271B0
bool ASfxDsp::UpdateBufferSize(int newMaxBlockSize)
{
	// nothing to do if the maximum number of samples per processing block hasn't changed
	if (newMaxBlockSize == mNumAllocatedInBuffSamples)
    {
		return false;	// buffer size wasn't changed
    }

	// update with the new value
	mNumAllocatedInBuffSamples = newMaxBlockSize;

	// (re)allocate audio buffers in mInBuff
	if (mInBuff != NULL)
	{
		FMOD_Memory_Free(mInBuff);
	}

	mInBuff = (float*) FMOD_Memory_Alloc(newMaxBlockSize * sizeof(float));

	return true;	// buffer size changed
}

//-----------------------------------------------------------------------------
//#pragma export (ASfxDsp::ClearBuffers)
// VERIFY: dab940a9 auto
// OFFSET: 0x00927A10
void ASfxDsp::ClearBuffers()
{
	ClearInBuff();
	ClearReverbInternalBuffers();
}


//-----------------------------------------------------------------------------
void ASfxDsp::ClearInBuff()
{
	if (mInBuff != NULL)
	{
		// CHANGE THIS TO A BLOCK MOVE
		for (int i=0; i < mNumAllocatedInBuffSamples; i++)
		{
			mInBuff[i] = 0.0f;
		}
	}
}


//-----------------------------------------------------------------------------
// VERIFY: 7c3496f7 manual
// OFFSET: 0x00927220
void ASfxDsp::ClearReverbInternalBuffers()
{
	int delayLine;
	
	if (mLateDelays)
	{
		for (delayLine=0; delayLine<kNumLateReverbDelays; delayLine++)
		{
			if (mLateDelays[delayLine])
			{
				// *** CHANGE THESE TO BLOCK MOVES FOR BETTER EFFICIENCY ***
				for (int samp=0; samp<mLateDelaySamplesAllocated[delayLine]; samp++)	
				{
					mLateDelays[delayLine][samp] = 0.0f;
				}
			}
		}
	}
	
	// Clear earlyLate delay line.
	// Replace this with a block move.
	int samp;
    for (samp=0; samp<mEarlyLateSamplesAllocated; samp++)
	{
		mEarlyLateDelay[samp] = 0.0f;
	}
	
	// Clear early delay line.
	// Replace this with a block move.
	for (samp=0; samp<mEarlyDelaySamplesAllocated; samp++)
	{
		mEarlyDelay[samp] = 0.0f;
	}
	
	// Clear allpass delays
	for (delayLine=0; delayLine<kNumAllpassDelays; delayLine++)
	{
		for (int samp=0; samp<mAllpassSamplesAllocated[delayLine]; samp++)
		{
			mAllpassDelays[delayLine][samp] = 0.0f;
		}
	}
	
	// Zero the internal variables
   FMOD_memset(mzDecayHF, 0, kNumLateReverbDelays * sizeof(float));
   FMOD_memset(mMatrix, 0, kNumMatrixStages * kNumLateReverbDelays * sizeof(float));
   mzRoomHF0 = 0.0f;
   mzRoomHF1 = 0.0f;
   mzRoomLF0 = 0.0f;
   mzRoomLF1 = 0.0f;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~ DoDSPProcessing ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// OFFSET: 0x00927A40
void ASfxDsp::DoDSPProcessing (float * inAudio, float * outAudio, int channels, unsigned int l_sampframes,
							float rate, float drymix)
{
	unsigned int	i;
	int				ch;	
	float           eruOut[kNumEarlyDelayTaps + 1];       
	float			val, oldVal;
	int				delayLine, delayTap;
	unsigned int	sampframes = (unsigned int)l_sampframes;
	unsigned int	sampframesmi;	// sampframes - i
	float			invSampframes = 1.0f / (float)sampframes;

#ifdef ADD_ANTI_DENORMALS_TO_INPUT
#define VERY_SMALL_BUT_NOT_DENORMAL_VALUE	(1.0e-15f)
	// mix very quiet noise (-300 dB) into the input signal to hopefully avoid any denormal values
	for (ch=0; ch < channels; ch++)
	{
		for (i=0; i < sampframes; i += 2)
        {
			inAudio[ch+channels*i] += VERY_SMALL_BUT_NOT_DENORMAL_VALUE;
        }
		for (i=1; i < sampframes; i += 2)
        {
			inAudio[ch+channels*i] -= VERY_SMALL_BUT_NOT_DENORMAL_VALUE;
        }
	}
#undef VERY_SMALL_BUT_NOT_DENORMAL_VALUE
#endif

    // Copy input into our input buffer. This may be faster than doing it inside the big loop.
	BlockProcessInput(sampframes, channels, inAudio, rate);

	// Adjust the EarlyLate read pointers in case Ref or Rev delays have changed
	for (delayTap=0; delayTap<kNumEarlyLateDelayTaps; delayTap++)
	{
		mEarlyLateReadPointer[delayTap] = (mEarlyLateSamples[delayTap] + mEarlyLateWritePointer - 1) & mEarlyLateMask;

        if (mEarlyLateSamples[delayTap] == mOldEarlyLateSamples[delayTap])
		{
			mEarlyLateChanged[delayTap] = false;
		}
		else	// If the delay has changed, we need to do two reads and crossfade them.
		{
			mEarlyLateChanged[delayTap] = true;
			mOldEarlyLateReadPointer[delayTap] = (mOldEarlyLateSamples[delayTap] + mEarlyLateWritePointer - 1) & mEarlyLateMask;
			mOldEarlyLateSamples[delayTap] = mEarlyLateSamples[delayTap];	// For next time...
		}
    }
	// Deal with interpolations and crossfades
	for (delayLine=0; delayLine<kNumLateReverbDelays; delayLine++)
	{
		// Adjust the Late read pointers in case late delay sizes have changed
		mLateReadPointer[delayLine] = (mLateDelayLenSamples[delayLine] + mLateWritePointer[delayLine] - 1) 
										& mLateMask[delayLine];
		mLateReadPointerB[delayLine] = (mLateDelayLenBSamples[delayLine] + mLateWritePointer[delayLine] - 1) 
										& mLateMask[delayLine];
		if (mLateDelayLenSamples[delayLine] == mOldLateDelayLenSamples[delayLine])
		{
			mLateChanged[delayLine] = false;
		}
		else	// If the late delays changed, we need to do two reads and crossfade them.
		{
			mLateChanged[delayLine] = true;
			mOldLateReadPointer[delayLine] = (mOldLateDelayLenSamples[delayLine] + mLateWritePointer[delayLine] - 1) 
										& mLateMask[delayLine];
			mOldLateDelayLenSamples[delayLine] = mLateDelayLenSamples[delayLine];	// For next time...
		}
		
		mLatchFeedback[delayLine] = mFeedback[delayLine];		// Latch it in case it changes in the interim.
		
		if (mLatchFeedback[delayLine] == mOldFeedback[delayLine])
		{
			mPrevFeedback[delayLine] = mLatchFeedback[delayLine];	// Use it for our crossfade (in case mFeedback hasn't changed)
		}
		else
		{
			if (!mLateChanged[delayLine])	// if we haven't made these changes above
			{
				mLateChanged[delayLine] = true;
				mOldLateReadPointer[delayLine] = (mOldLateDelayLenSamples[delayLine] + mLateWritePointer[delayLine] - 1) 
									& mLateMask[delayLine];
				// We do this inside the 'if', because mOldLateDelayLenSamples gets changed above, 
				// which would change mOldLateReadPointer back to mLateReadPointer.
			}
			
			mPrevFeedback[delayLine] = mOldFeedback[delayLine];	// For our crossfade
			mOldFeedback[delayLine] = mLatchFeedback[delayLine];		// For next time...
		}
		
		mLatchDecayHF[delayLine] = mDecayHF[delayLine];		// Latch it for our interpolation.
		mLatchOneMinusDecayHF[delayLine] = 1.0f - mLatchDecayHF[delayLine];
		
		if (mLatchDecayHF[delayLine] == mOldDecayHF[delayLine])
		{
			mDecayHFChanged[delayLine] = false;
		}
		else
		{
			mDecayHFChanged[delayLine] = true;
			mPrevDecayHF[delayLine] = mOldDecayHF[delayLine];	// For our interpolation
			mOldDecayHF[delayLine] = mLatchDecayHF[delayLine];	// For next time...
		}
	}

	mLatchLRgain = mLRgain;
	
	if (mLatchLRgain == mOldLRgain)
	{
		mLRgainChanged = false;
	}
	else
	{
		mLRgainChanged = true;
		mPrevLRgain = mOldLRgain;		// For our interpolation
		mOldLRgain = mLatchLRgain;		// For next time
	}

	mLatchERgain = mERgain;
	
	if (mLatchERgain == mOldERgain)
	{
		mERgainChanged = false;
	}
	else
	{
		mERgainChanged = true;
		mPrevERgain = mOldERgain;		// For our interpolation
		mOldERgain = mLatchERgain;		// For next time
	}

	mLatchRoomHF = mRoomHF;
	
	if (mLatchRoomHF == mOldRoomHF)
	{
		mRoomHFChanged = false;
	}
	else
	{
		mRoomHFChanged = true;
		mPrevRoomHF = mOldRoomHF;		// For our interpolation
		mOldRoomHF = mLatchRoomHF;		// For next time
	}

	mLatchHadamard = mHadamard;

	if (mLatchHadamard == mOldHadamard)
	{
		mHadamardChanged = false;
	}
	else
	{
		mHadamardChanged = true;
		mPrevHadamard = mOldHadamard;		// For our interpolation
		mOldHadamard = mLatchHadamard;		// For next time
	}

	mLatchDiffusionScale = mDiffusionScale;

    if (mLatchDiffusionScale == mOldDiffusionScale)
	{
		mDiffusionScaleChanged = false;
	}
	else
	{
		mDiffusionScaleChanged = true;
		mPrevDiffusionScale = mOldDiffusionScale;		// For our interpolation
		mOldDiffusionScale = mLatchDiffusionScale;		// For next time
	}

    /*
	mLatchRoomLF = mRoomLF;
	
	if (mLatchRoomLF == mOldRoomLF)
	{
		mRoomLFChanged = false;
	}
	else
	{
		mRoomLFChanged = true;
		mPrevRoomLF = mOldRoomLF;		// For our interpolation
		mOldRoomLF = mLatchRoomLF;		// For next time
	}
    */

	// Now start the Big DSP Loop...
	for (i=0; i<sampframes; i++)	// for each sample
	{
		sampframesmi = sampframes - i;
        val = (float)mInBuff[i];
					
		// Room HF low-pass filters
		float roomHF, hadamard, diffusionscale;
		
		if (mRoomHFChanged)
		{
			roomHF = ((mPrevRoomHF * sampframesmi) + (mLatchRoomHF * i)) * invSampframes;
		}
		else
		{
			roomHF = mLatchRoomHF;
		}

		if (mHadamardChanged)
		{
			hadamard = ((mPrevHadamard * sampframesmi) + (mLatchHadamard * i)) * invSampframes;
		}
		else
		{
			hadamard = mLatchHadamard;
		}

		if (mDiffusionScaleChanged)
		{
			diffusionscale = ((mPrevDiffusionScale * sampframesmi) + (mLatchDiffusionScale * i)) * invSampframes;
		}
		else
		{
			diffusionscale = mLatchDiffusionScale;
		}


        /*
        Room LF - 2nd order high pass filter, don't interpolate yet

        if(mRoomLFChanged)
        {
			roomLF = ((mPrevRoomLF * sampframesmi) + (mLatchRoomLF * i)) * invSampframes;
        }
        else
        {
			roomLF = mLatchRoomLF;
        }
        */

		float oneMinusRoomHF = 1.0f - roomHF;
		mzRoomHF0 = val*roomHF + mzRoomHF0*oneMinusRoomHF;
		mzRoomHF1 = mzRoomHF0*roomHF + mzRoomHF1*oneMinusRoomHF;

        float fout;
        if(mRoomLF != 0.0f)
        {
            fout = mzRoomHF1 * mRoomLFcoeffs.a0 + mzRoomLF0;
            mzRoomLF0  = mzRoomHF1 * mRoomLFcoeffs.a1 + mzRoomLF1 + fout * mRoomLFcoeffs.b1;
            mzRoomLF1  = mzRoomHF1 * mRoomLFcoeffs.a2 + fout * mRoomLFcoeffs.b2;
        }
        else
        {
            fout = mzRoomHF1;
        }

		// Insert Delay here
		mEarlyLateWritePointer = --mEarlyLateWritePointer & mEarlyLateMask;
		mEarlyLateDelay[mEarlyLateWritePointer] = fout;
				
		// ================== Early Reflections ================== 
		
		mEarlyLateReadPointer[0] = --mEarlyLateReadPointer[0] & mEarlyLateMask;
		float	input, output, ERgain;
		
		if (mERgainChanged)
		{
			ERgain = ((mPrevERgain * sampframesmi) + (mLatchERgain * i)) * invSampframes;
		}
		else
		{
			ERgain = mLatchERgain;
		}
		
		input = mEarlyLateDelay[mEarlyLateReadPointer[0]] * ERgain;

		if (mEarlyLateChanged[0])	// Crossfade the old and new reads
		{
			mOldEarlyLateReadPointer[0] = --mOldEarlyLateReadPointer[0] & mEarlyLateMask;
			float oldInput = mEarlyLateDelay[mOldEarlyLateReadPointer[0]] * ERgain;
			input = ((oldInput * sampframesmi) + (input * i)) * invSampframes;
		}	
        
		// 2 Allpasses in series

		for (delayLine=0; delayLine<kNumAllpassDelays; delayLine++)
		{
			mAllpassWritePointer[delayLine] = --mAllpassWritePointer[delayLine] & mAllpassMask[delayLine];
			
			mAllpassReadPointer[delayLine] = --mAllpassReadPointer[delayLine] & mAllpassMask[delayLine];
			output = mAllpassDelays[delayLine][mAllpassReadPointer[delayLine]]
										- input * mAllpassGain;		// feedforward loop
			
			mAllpassDelays[delayLine][mAllpassWritePointer[delayLine]] =
										input + output * mAllpassGain;	// feedback loop
			input = output;
		}
		
		// Early Delay FIR
		
		eruOut[0] = output;
		
		mEarlyWritePointer = --mEarlyWritePointer & mEarlyMask;
		mEarlyDelay[mEarlyWritePointer] = output;
		
		for (delayTap=0; delayTap<kNumEarlyDelayTaps; delayTap++)
		{
			mEarlyReadPointer[delayTap] = --mEarlyReadPointer[delayTap] & mEarlyMask;
			eruOut[delayTap+1] = mEarlyDelay[mEarlyReadPointer[delayTap]];
		}

		
		// ================== Late Reverb ====================
		
		// This assumes mNumLateReverbDelays = kNumLate_EarlyLateDelayTaps.
		for (delayLine=0; delayLine<mNumLateReverbDelays; delayLine++)
		{
			delayTap = kNumEarly_EarlyLateDelayTaps + delayLine;
			
			mEarlyLateReadPointer[delayTap] = --mEarlyLateReadPointer[delayTap] & mEarlyLateMask;
			val = mEarlyLateDelay[mEarlyLateReadPointer[delayTap]];	

			if (mEarlyLateChanged[delayTap])	// Crossfade the old and new reads
			{
				mOldEarlyLateReadPointer[delayTap] = --mOldEarlyLateReadPointer[delayTap] & mEarlyLateMask;
				oldVal = mEarlyLateDelay[mOldEarlyLateReadPointer[delayTap]];
				val = ((oldVal * sampframesmi) + (val * i)) * invSampframes;
			}

			if (mLRgainChanged)
			{
				float LRgain = ((mPrevLRgain * sampframesmi) + (mLatchLRgain * i)) * invSampframes;
				val *= LRgain;
			}
			else
			{
				val *= mLatchLRgain;
			}

			mLateReadPointer[delayLine] = --mLateReadPointer[delayLine] & mLateMask[delayLine];
			if (mLateChanged[delayLine])	// Crossfade
			{
				mOldLateReadPointer[delayLine] = --mOldLateReadPointer[delayLine] & mLateMask[delayLine];
				oldVal = mPrevFeedback[delayLine] * mLateDelays[delayLine][mOldLateReadPointer[delayLine]];
				float thisVal = mLatchFeedback[delayLine] * mLateDelays[delayLine][mLateReadPointer[delayLine]];
				val += (((oldVal * sampframesmi) + (thisVal * i)) * invSampframes);
			}
			else
			{
				val += (mLatchFeedback[delayLine] * mLateDelays[delayLine][mLateReadPointer[delayLine]]);
			}

			// Gain to compensate for one matrix gain, based on the diffusion.
			// val *= mDiffusionScale;
			val *= diffusionscale;
			
			// Low-pass filter for HF Decay ratio
			if (mDecayHFChanged[delayLine])
			{
				// Interpolate decayHF
				float decayHF = ((mPrevDecayHF[delayLine] * sampframesmi) + (mLatchDecayHF[delayLine] * i)) * invSampframes;
				float oneMinusDecayHF = 1.0f - decayHF;
				mzDecayHF[delayLine] = val * decayHF + mzDecayHF[delayLine] * oneMinusDecayHF;
			}
			else
			{
				mzDecayHF[delayLine] = val * mLatchDecayHF[delayLine] + mzDecayHF[delayLine] * mLatchOneMinusDecayHF[delayLine];
			}
			
			mLateWritePointer[delayLine] = --mLateWritePointer[delayLine] & mLateMask[delayLine];			
		}


		mMatrix[0 * kNumLateReverbDelays + 0] = mzDecayHF[0] + mzDecayHF[1] * hadamard;
		mMatrix[0 * kNumLateReverbDelays + 1] = mzDecayHF[1] - mzDecayHF[0] * hadamard;
		mMatrix[0 * kNumLateReverbDelays + 2] = mzDecayHF[2] + mzDecayHF[3] * hadamard;
		mMatrix[0 * kNumLateReverbDelays + 3] = mzDecayHF[3] - mzDecayHF[2] * hadamard;
		mMatrix[0 * kNumLateReverbDelays + 4] = mzDecayHF[4] + mzDecayHF[5] * hadamard;
		mMatrix[0 * kNumLateReverbDelays + 5] = mzDecayHF[5] - mzDecayHF[4] * hadamard;
		mMatrix[0 * kNumLateReverbDelays + 6] = mzDecayHF[6] + mzDecayHF[7] * hadamard;
		mMatrix[0 * kNumLateReverbDelays + 7] = mzDecayHF[7] - mzDecayHF[6] * hadamard;
		
		mMatrix[1 * kNumLateReverbDelays + 0] = mMatrix[0 * kNumLateReverbDelays + 0] + mMatrix[0 * kNumLateReverbDelays + 2] * hadamard;
		mMatrix[1 * kNumLateReverbDelays + 1] = mMatrix[0 * kNumLateReverbDelays + 1] + mMatrix[0 * kNumLateReverbDelays + 3] * hadamard;
		mMatrix[1 * kNumLateReverbDelays + 2] = mMatrix[0 * kNumLateReverbDelays + 2] - mMatrix[0 * kNumLateReverbDelays + 0] * hadamard;
		mMatrix[1 * kNumLateReverbDelays + 3] = mMatrix[0 * kNumLateReverbDelays + 3] - mMatrix[0 * kNumLateReverbDelays + 1] * hadamard;
		mMatrix[1 * kNumLateReverbDelays + 4] = mMatrix[0 * kNumLateReverbDelays + 4] + mMatrix[0 * kNumLateReverbDelays + 6] * hadamard;
		mMatrix[1 * kNumLateReverbDelays + 5] = mMatrix[0 * kNumLateReverbDelays + 5] + mMatrix[0 * kNumLateReverbDelays + 7] * hadamard;
		mMatrix[1 * kNumLateReverbDelays + 6] = mMatrix[0 * kNumLateReverbDelays + 6] - mMatrix[0 * kNumLateReverbDelays + 4] * hadamard;
		mMatrix[1 * kNumLateReverbDelays + 7] = mMatrix[0 * kNumLateReverbDelays + 7] - mMatrix[0 * kNumLateReverbDelays + 5] * hadamard;
		
		mMatrix[2 * kNumLateReverbDelays + 0] = mMatrix[1 * kNumLateReverbDelays + 0] + mMatrix[1 * kNumLateReverbDelays + 4] * hadamard;
		mMatrix[2 * kNumLateReverbDelays + 1] = mMatrix[1 * kNumLateReverbDelays + 1] + mMatrix[1 * kNumLateReverbDelays + 5] * hadamard;
		mMatrix[2 * kNumLateReverbDelays + 2] = mMatrix[1 * kNumLateReverbDelays + 2] + mMatrix[1 * kNumLateReverbDelays + 6] * hadamard;
		mMatrix[2 * kNumLateReverbDelays + 3] = mMatrix[1 * kNumLateReverbDelays + 3] + mMatrix[1 * kNumLateReverbDelays + 7] * hadamard;
		mMatrix[2 * kNumLateReverbDelays + 4] = mMatrix[1 * kNumLateReverbDelays + 4] - mMatrix[1 * kNumLateReverbDelays + 0] * hadamard;
		mMatrix[2 * kNumLateReverbDelays + 5] = mMatrix[1 * kNumLateReverbDelays + 5] - mMatrix[1 * kNumLateReverbDelays + 1] * hadamard;
		mMatrix[2 * kNumLateReverbDelays + 6] = mMatrix[1 * kNumLateReverbDelays + 6] - mMatrix[1 * kNumLateReverbDelays + 2] * hadamard;
		mMatrix[2 * kNumLateReverbDelays + 7] = mMatrix[1 * kNumLateReverbDelays + 7] - mMatrix[1 * kNumLateReverbDelays + 3] * hadamard;
		
		// Write to the lateDelays
		mLateDelays[0][mLateWritePointer[0]] = mMatrix[2 * kNumLateReverbDelays + 1];
		mLateDelays[1][mLateWritePointer[1]] = mMatrix[2 * kNumLateReverbDelays + 2];
		mLateDelays[2][mLateWritePointer[2]] = mMatrix[2 * kNumLateReverbDelays + 3];
		mLateDelays[3][mLateWritePointer[3]] = mMatrix[2 * kNumLateReverbDelays + 4];
		mLateDelays[4][mLateWritePointer[4]] = mMatrix[2 * kNumLateReverbDelays + 5];
		mLateDelays[5][mLateWritePointer[5]] = mMatrix[2 * kNumLateReverbDelays + 6];
		mLateDelays[6][mLateWritePointer[6]] = mMatrix[2 * kNumLateReverbDelays + 7];
		mLateDelays[7][mLateWritePointer[7]] = mMatrix[2 * kNumLateReverbDelays + 0];

		for (ch=0; ch<channels; ch++)		// for each channel
		{
			float lruSum = 0.0f;				
			float eruSum = 0.0f;
			int nextDelay = ch;

			for (int delay=0; delay<(kNumLateReverbDelays/channels); delay++)
			{
				mLateReadPointerB[nextDelay] = --mLateReadPointerB[nextDelay] & mLateMask[nextDelay];
				lruSum += mLateDelays[nextDelay][mLateReadPointerB[nextDelay]];
				
				eruSum += eruOut[nextDelay];
				nextDelay += channels;
			}
									
			if (drymix == 0.0f)
			{
				outAudio[i*channels+ch] = eruSum + lruSum;	
			}
			else
			{
				outAudio[i*channels+ch] = (inAudio[i*channels+ch] * drymix) + eruSum + lruSum;	
			}

		}	// end for each channel
	}	// end for each sample
		
}	// end DoDSPProcessing


// Copy input into our input buffer. This may be faster than doing it inside the big loop (?).
// VERIFY: c2997d28 manual
// OFFSET: 0x00927340
void ASfxDsp::BlockProcessInput(unsigned int sampframes, int channels, float *inAudio, float rate)
{
	unsigned int i;

	if (channels == 1)            // mono   
	{
       SfxDataMove(inAudio, mInBuff, sampframes*sizeof(float));
	}
	else
	{
    	int ch;

    	for (i=0; i<sampframes; i++)    // for each sample
		{
			mInBuff[i] = 0.0f;
			for (ch=0; ch<channels; ++ch)        // for each channel
			{
				mInBuff[i] += inAudio[i*channels+ch];	// *** Would it be more efficient to reverse the loops? ***
			}
           // mInBuff[i] /= (float)channels;	// Don't average them!
		}          
	}
}	




// VERIFY: 23f97caa auto
// OFFSET: 0x00927400
void ASfxDsp::SetLate_EarlyLateDelayTaps(float refPlusRevDelaySec, float nextLengthSec, float delayRatio, float rate)	
{
    mEarlyLateSec[kNumEarly_EarlyLateDelayTaps] = refPlusRevDelaySec;       // Tap 1 = Ref+Rev delay
    int earlyLateSamples = (int)(refPlusRevDelaySec * rate);       
    if (earlyLateSamples == 0)
    {
        earlyLateSamples++;             // Avoid long wrap-around delay.
    }
    mEarlyLateSamples[kNumEarly_EarlyLateDelayTaps] = earlyLateSamples;
	
	// Set the remaining EarlyLate delay taps (2 - 8)		
	for (int delayTap=kNumEarly_EarlyLateDelayTaps+1; delayTap<kNumEarlyLateDelayTaps; delayTap++)
	{
		mEarlyLateSec[delayTap] = refPlusRevDelaySec + nextLengthSec;
		mEarlyLateSamples[delayTap] = (int)(mEarlyLateSec[delayTap] * rate);
		nextLengthSec *= delayRatio;
	}
}

// VERIFY: 5b44cbf9 auto
// OFFSET: 0x00927470
void ASfxDsp::SetAllpassDelays(float rate)
{
	mAllpassDelayLenSec[0] = kAllpassDelayLenSec0;
	mAllpassDelayLenSec[1] = kAllpassDelayLenSec1;

	for (int delayLine = 0; delayLine<kNumAllpassDelays; delayLine++)
	{
		mAllpassDelayLenSamples[delayLine] = (int)(mAllpassDelayLenSec[delayLine] * rate);
		mAllpassReadPointer[delayLine] = mAllpassDelayLenSamples[delayLine] - 1;
	}
}

void ASfxDsp::SetEarlyDelay(float nextLengthSec, float delayRatio, float rate)
{	
	// Set the early delay taps (0 - 6)
	for (int delayLine=0; delayLine<kNumEarlyDelayTaps; delayLine++)
	{
		mEarlyDelayLenSec[delayLine] = nextLengthSec;
		mEarlyDelayLenSamples[delayLine] = (int)(nextLengthSec * rate);
		nextLengthSec *= delayRatio;

		mEarlyReadPointer[delayLine] = mEarlyDelayLenSamples[delayLine] - 1;
	}
}

// VERIFY: 37367dfa manual
// OFFSET: 0x009274C0
void ASfxDsp::SetLateDelays(float nextLengthSec, float delayRatio, float nextLengthBSec, float delayRatioB, float rate)
{
	float	nextLengthSamples;

	float	nextLengthBSamples = nextLengthBSec * rate;
	
	for (int delayLine=0; delayLine<kNumLateReverbDelays; delayLine++)
	{
		// Set main delay line lengths
		mLateDelayLenSec[delayLine] = nextLengthSec;
		nextLengthSamples = nextLengthSec * rate;
		mLateDelayLenSamples[delayLine] = (int)(nextLengthSamples + 0.5);	
		nextLengthSec *= delayRatio;
		mLateReadPointer[delayLine] = (mLateDelayLenSamples[delayLine] + mLateWritePointer[delayLine] - 1) 
										& mLateMask[delayLine];

		// Set delay line lengths for B reads
		mLateDelayLenBSamples[delayLine] = (int)(nextLengthBSamples + 0.5);	// Short delays to keep outputs from cancelling.
		nextLengthBSamples *= delayRatioB;
		mLateReadPointerB[delayLine] = (mLateDelayLenBSamples[delayLine] + mLateWritePointer[delayLine] - 1) 
										& mLateMask[delayLine];
	}
}

void ASfxDsp::ZeroWritePointers()
{
	int delayLine;
	
	mEarlyWritePointer = 0;	
	mEarlyLateWritePointer = 0;	
	
	for (delayLine=0; delayLine<kNumAllpassDelays; delayLine++)
	{
		mAllpassWritePointer[delayLine] = 0;
	}
	
	for (delayLine=0; delayLine<kNumLateReverbDelays; delayLine++)
	{
		mLateWritePointer[delayLine] = 0;
	}
}
