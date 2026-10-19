#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_MOD

#include "fmod.h"

#include "fmod_channel_software.h"
#include "fmod_codec_mod.h"
#include "fmod_debug.h"
#include "fmod_dspi.h"
#include "fmod_file.h"
#include "fmod_localcriticalsection.h"
#include "fmod_memory.h"
#include "fmod_systemi.h"
#include "fmod_string.h"

#include <stdio.h>
#include <stdlib.h>

namespace FMOD
{


FMOD_CODEC_DESCRIPTION_EX modcodec;


#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetCodecDescription is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_CODEC_DESCRIPTION_EX * F_API FMODGetCodecDescriptionEx()
    {
        return CodecMOD::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


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
// VERIFY: f2869502 manual
// OFFSET: 0x0090CD00
FMOD_CODEC_DESCRIPTION_EX *CodecMOD::getDescriptionEx()
{
    FMOD_memset(&modcodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    modcodec.name            = "FMOD MOD Codec";
    modcodec.version         = 0x00010100;
    modcodec.timeunits       = (FMOD_TIMEUNIT)(FMOD_TIMEUNIT_PCM | FMOD_TIMEUNIT_MODORDER | FMOD_TIMEUNIT_MODROW | FMOD_TIMEUNIT_MODPATTERN);
    modcodec.defaultasstream = 1;
    modcodec.open            = &CodecMOD::openCallback;
    modcodec.close           = &CodecMOD::closeCallback;
    modcodec.read            = &CodecMOD::readCallback;
    modcodec.getlength       = &MusicSong::getLengthCallback;
    modcodec.setposition     = &CodecMOD::setPositionCallback;
    modcodec.getposition     = &MusicSong::getPositionCallback;
    

    modcodec.mType = FMOD_SOUND_TYPE_MOD;
    modcodec.mSize = sizeof(CodecMOD);

    return &modcodec;
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
// VERIFY: 30bbb505 auto
// OFFSET: 0x0090B960
FMOD_RESULT CodecMOD::calculateLength()
{
    waveformat[0].lengthpcm = 0;

    play();

    while (!(mFinished))
    {
        update(false);

        waveformat[0].lengthpcm += mMixerSamplesPerTick;
    }

    stop();

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
int CodecMOD::getAmigaPeriod(int note, int middlec)
{
	return (8363L * gPeriodTable[note-1] / middlec);
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
// VERIFY: c37ada96 manual
// OFFSET: 0x009076E0
FMOD_RESULT MusicChannelMOD::portamento()
{
	MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();

	/*
        Slide pitch down if it needs to.
    */
	if (vcptr->mFrequency < mPortaTarget) 
	{
		vcptr->mFrequency += (mPortaSpeed << 2);
		if (vcptr->mFrequency > mPortaTarget) 
        {
			vcptr->mFrequency = mPortaTarget;
        }
	}

	/*
        Slide pitch up if it needs to.
    */
	if (vcptr->mFrequency > mPortaTarget) 
	{
		vcptr->mFrequency -= (mPortaSpeed << 2);
		if (vcptr->mFrequency < mPortaTarget) 
        {
			vcptr->mFrequency=mPortaTarget;
        }
	}

	 /*
	if (glissando[track]) 
	{
	}
	*/
	vcptr->mNoteControl |= FMUSIC_FREQ;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
	to carry out a vibrato at a certain depth and speed            

	[PARAMETERS]
	track - the track number to do the vibrato to
 
	[RETURN_VALUE]

	[REMARKS]
	AND'ing temp with 31 removes the sign bit giving the abs value

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: a192f5ac auto
// OFFSET: 0x0090A630
FMOD_RESULT MusicChannelMOD::vibrato() 
{
	int delta;
	unsigned char temp;
	MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();

	temp = (mVibPos & 31);

	switch (mWaveControl & 3)
	{
		case 0: delta = gSineTable[temp];			/* sine */
				break;
		case 1: temp <<= 3;								/* ramp down */
				if (mVibPos < 0) 
                {
                    temp=255-temp;
                }
				delta=temp;
				break;
		case 2: delta = 255;							/* square */
				break;
		case 3: delta = FMOD_RAND()&255;						/* random */
				break;
		default:delta = 0;
	};

	delta *= mVibDepth;
	delta >>=7;
	delta <<=2;   /* we use 4*periods so make vibrato 4 times bigger */

	if (mVibPos >= 0) 
    {
		vcptr->mFrequencyDelta = delta;
    }
	else
    {
		vcptr->mFrequencyDelta = -delta;
    }
	
	mVibPos += mVibSpeed;
	if (mVibPos > 31) 
    {
		mVibPos -= 64;
    }

	vcptr->mNoteControl |= FMUSIC_FREQ;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
	To carry out a tremolo at a certain depth and speed

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: cc28d7ed auto
// OFFSET: 0x0090A6F0
FMOD_RESULT MusicChannelMOD::tremolo() 
{
	int delta;
	unsigned char temp;
	MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();

	temp = (mTremoloPosition & 31);

	switch((mWaveControl >> 4) & 3)
	{
		case 0: delta = gSineTable[temp];				/* sine */
				break;
		case 1: temp <<= 3;									/* ramp down */
				if (mTremoloPosition < 0) 
                {
                    temp=255-temp;
                }
				delta=temp;
				break;
		case 2: delta = 255;								/* square */
				break;
		case 3: delta = gSineTable[temp];				/* random (just use sine for now) */
				break;
		default:delta = 0;
	};

	delta *= mTremoloDepth;
	delta >>= 6;

	if (mTremoloPosition >= 0) 
	{
		if (vcptr->mVolume + delta > 64) 
        {
			delta = 64 - vcptr->mVolume;
        }
		vcptr->mVolumeDelta = delta;
	}
	else 
	{
		if ((short)(vcptr->mVolume - delta) < 0) 
        {
			delta = vcptr->mVolume;
        }
		vcptr->mVolumeDelta = delta;
	}

	mTremoloPosition += mTremoloSpeed;
	if (mTremoloPosition > 31) 
    {
		mTremoloPosition -=64;
    }

	vcptr->mNoteControl |= FMUSIC_VOLUME;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 22d1c56d manual
// OFFSET: 0x0090A7C0
FMOD_RESULT CodecMOD::updateNote(bool audible)
{
	MusicNote   *current;
	bool        breakflag = false;
	bool        jumpflag = false;
	int 		count;

	/*
        Point our note pointer to the correct pattern buffer, and to the
	    correct offset in this buffer indicated by row and number of channels
    */
	current = mPattern[mOrderList[mOrder]].mData + (mRow * mNumChannels);
	if (!current) 
    {
		return FMOD_OK;
    }

    if (mVisited)
    {
        if (mVisited[(mOrder * FMUSIC_MAXROWS) + mRow])
        {
            mFinished = true;
            return FMOD_OK;
        }
        mVisited[(mOrder * FMUSIC_MAXROWS) + mRow] = true;
    }

	/*
        Loop through each channel in the row until we have finished
    */
	for (count=0; count < mNumChannels; count++,current++)
	{
		MusicChannel            *cptr  = 0;
		MusicVirtualChannel     *vcptr = 0;
		MusicSample             *sptr  = 0;
		unsigned char			paramx, paramy;
		int 					oldvolume, oldfreq;

		paramx = current->mEffectParam >> 4;			/* get effect param x */
		paramy = current->mEffectParam & 0xF;			/* get effect param y */

		cptr = mMusicChannel[count];
		if (cptr->mVirtualChannelHead.isEmpty()) 
		{
			vcptr             = &gDummyVirtualChannel;  /* no channels allocated yet */
			vcptr->mSample    = &gDummySample;
		}
        else
        {
            vcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext();
        }
		
		/*
            Store any instrument number
        */
		if (current->mNumber) 
		{
			cptr->mInstrument = current->mNumber - 1; /* remember the Instrument # */

//		    if (mInstCallback[current->mNumber] && mInstCallback[current->mNumber]->callback)
//            {
//				checkCallback(FMUSIC_CALLBACK_INSTRUMENT, current->mNumber);
//            }
		}

		/*
            Set up sample pointer
        */
		if (cptr->mInstrument < mNumSamples) 
        {
			sptr = &mSample[cptr->mInstrument];
        }
		else 
        {
			sptr = &gDummySample;
        }

		oldvolume = vcptr->mVolume;
		oldfreq   = vcptr->mFrequency;

		/*
            If there is no more tremolo, set volume to volume + last tremolo delta
        */
		if (cptr->mRecentEffect == FMUSIC_MOD_TREMOLO && current->mEffect != FMUSIC_MOD_TREMOLO)
        {
			vcptr->mVolume += vcptr->mVolumeDelta;		
        }
		cptr->mRecentEffect  = current->mEffect;

		vcptr->mVolumeDelta = 0;
		vcptr->mNoteControl	= 0;

		/*
            PROCESS NOTE
        */
		if (current->mNote) 
		{
			vcptr->mNoteControl |= FMUSIC_STOP;

			if (vcptr == &gDummyVirtualChannel) 
			{
                FMOD_RESULT result;

				result = spawnNewVirtualChannel(cptr, sptr, &vcptr);
				if (result != FMOD_OK)
				{
					vcptr           = &gDummyVirtualChannel; /* no channels allocated yet */
					vcptr->mSample  = &gDummySample;
				}
			}
			
			/* 
                Get period according to relative note, c2spd and finetune 
            */
			cptr->mNote		= current->mNote;					/* now remember the note */
			cptr->mPeriod   = getAmigaPeriod(cptr->mNote, sptr->mMiddleC);
			vcptr->mPan		= mDefaultPan[count];
			
			/*
                Retrigger tremolo and vibrato waveforms
            */
			if ((cptr->mWaveControl & 0xF) < 4) 
            {
				cptr->mVibPos = 0;
            }
			if ((cptr->mWaveControl >> 4)  < 4) 
            {
				cptr->mTremoloPosition = 0;
            }

			/*
                Frequency only changes if there are no portamento effects
            */
			if (current->mEffect != FMUSIC_MOD_PORTATO && current->mEffect != FMUSIC_MOD_PORTATOVOLSLIDE) 
            {
				vcptr->mFrequency = cptr->mPeriod;
            }

			vcptr->mNoteControl = FMUSIC_TRIGGER;
		}

		/*
            PROCESS INSTRUMENT NUMBER
        */
		if (current->mNumber) 
        {
			vcptr->mVolume = sptr->mDefaultVolume;
        }

		vcptr->mFrequencyDelta = 0;
		vcptr->mNoteControl	|= FMUSIC_FREQ | FMUSIC_VOLUME | FMUSIC_PAN;

		/*
            TICK 0 EFFECTS
        */
		switch (current->mEffect) 
		{
			/*
                Not processed on tick 0
            */
			case FMUSIC_MOD_ARPEGGIO : 
			case FMUSIC_MOD_PORTAUP :
			case FMUSIC_MOD_PORTADOWN :
			case FMUSIC_MOD_VOLUMESLIDE : 
			{
				break;				
			}

			case FMUSIC_MOD_SETVOLUME : 
			{
				vcptr->mVolume = current->mEffectParam;
				break;
			}
			case FMUSIC_MOD_PORTATO :
			{
				if (current->mEffectParam) 
                {
					cptr->mPortaSpeed = current->mEffectParam;
                }
				cptr->mPortaTarget = cptr->mPeriod;
				vcptr->mNoteControl &= ~FMUSIC_TRIGGER;
				vcptr->mNoteControl &= ~FMUSIC_FREQ;
				break;
			}
			case FMUSIC_MOD_PORTATOVOLSLIDE : 
			{
				cptr->mPortaTarget = cptr->mPeriod;
				vcptr->mNoteControl &= ~FMUSIC_TRIGGER;
				vcptr->mNoteControl &= ~FMUSIC_FREQ;
				break;
			}
			case FMUSIC_MOD_VIBRATO :
			{
				if (paramx) 
                {
					cptr->mVibSpeed = paramx;
                }
				if (paramy) 
                {
					cptr->mVibDepth = paramy;
                }
			}
			case FMUSIC_MOD_VIBRATOVOLSLIDE : 
			{
				break;								/* not processed on tick 0 */
			}
			case FMUSIC_MOD_TREMOLO : 
			{
				if (paramx) 
                {
					cptr->mTremoloSpeed = paramx;
                }
				if (paramy)
                {
					cptr->mTremoloDepth = paramy;
                }
				vcptr->mNoteControl &= ~FMUSIC_VOLUME;
				break;
			}
			case FMUSIC_MOD_SETPANPOSITION : 
			{
				vcptr->mPan = current->mEffectParam << 1;
				vcptr->mNoteControl |= FMUSIC_PAN;
				break;
			}
			case FMUSIC_MOD_SETSAMPLEOFFSET : 
			{
				unsigned int offset;

				if (current->mEffectParam) 
                {
					cptr->mSampleOffset = current->mEffectParam;
                }

				offset = (int)(cptr->mSampleOffset) << 8;

				if (offset >= sptr->mLoopStart + sptr->mLoopLength) 
				{
                    vcptr->mSampleOffset = sptr->mLoopStart + sptr->mLoopLength - 1;
                }
				else 
                {
                    vcptr->mSampleOffset = offset;
                }
				break;
			}
			case FMUSIC_MOD_PATTERNJUMP : /* --- 00 B00 : --- 00 D63 , should put us at ord=0, row=63 */
			{
				mNextOrder = current->mEffectParam;
				mNextRow = 0;
				
				if (mNextOrder >= mNumOrders) 
                {
					mNextOrder = 0;
                }
				
				jumpflag = true;
				break;
			}
			case FMUSIC_MOD_PATTERNBREAK : 
			{
				mNextRow = (paramx*10) + paramy;
				
				if (mNextRow > 63) 
                {
					mNextRow = 0;
                }
				if (!breakflag && !jumpflag) 
                {
					mNextOrder = mOrder+1;
                }
				if (mNextOrder >= mNumOrders) 
                {
					mNextOrder=0;
                }
				break;
			}
			case FMUSIC_MOD_SETSPEED : 
			{
				if (current->mEffectParam < 0x20) 
                {
                    if (current->mEffectParam)
                    {
    					mSpeed = current->mEffectParam;
                    }
                }
				else 
                {
					setBPM(current->mEffectParam);
                }
				break;
			}
			case FMUSIC_MOD_SPECIAL : 
			{
				switch (paramx) 
				{
					/*
                        Not processed on tick 0 / unsupported
                    */
					case FMUSIC_MOD_RETRIG :
					case FMUSIC_MOD_NOTECUT :
					case FMUSIC_MOD_SETFILTER :
					case FMUSIC_MOD_FUNKREPEAT :
					case FMUSIC_MOD_SETGLISSANDO : 
					{
						break;
					}
					case FMUSIC_MOD_FINEPORTAUP : 
					{
						vcptr->mFrequency -= (paramy << 2);
						break;
					}
					case FMUSIC_MOD_FINEPORTADOWN :
					{
						vcptr->mFrequency += (paramy << 2);
						break;
					}
					case FMUSIC_MOD_SETVIBRATOWAV : 
					{
						cptr->mWaveControl &= 0xF0;
						cptr->mWaveControl |= paramy;
						break;
					}
					case FMUSIC_MOD_SETFINETUNE :
					{
						fineTune2Hz(paramy, &sptr->mMiddleC);
						break;
					}
					case FMUSIC_MOD_PATTERNLOOP :
					{
						if (paramy == 0) 
                        {
							cptr->mPatternLoopRow = mRow;
                        }
						else 
						{
							if (!cptr->mPatternLoopNumber) 
                            {
								cptr->mPatternLoopNumber = paramy;
                            }
							else 
                            {
								cptr->mPatternLoopNumber--;
                            }
							if (cptr->mPatternLoopNumber) 
                            {
                                int count2;

								mNextRow = cptr->mPatternLoopRow;

                                if (mVisited)
                                {
                                    for (count2 = cptr->mPatternLoopRow; count2 <= mRow; count2++)
                                    {
                                        mVisited[(mOrder * FMUSIC_MAXROWS) + count2] = false;
                                    }
                                }
                            }
						}
						break;
					}
					case FMUSIC_MOD_SETTREMOLOWAV : 
					{
						cptr->mWaveControl &= 0xF;
						cptr->mWaveControl |= (paramy << 4);
						break;
					}
					case FMUSIC_MOD_SETPANPOSITION16 :
					{
						vcptr->mPan = paramy << 4;
						vcptr->mNoteControl |= FMUSIC_PAN;
						break;
					}
					case FMUSIC_MOD_FINEVOLUMESLIDEUP :
					{
						vcptr->mVolume += paramy;
						if (vcptr->mVolume > 64) 
                        {
							vcptr->mVolume = 64;
                        }
						break;
					}
					case FMUSIC_MOD_FINEVOLUMESLIDEDOWN :
					{
						vcptr->mVolume -= paramy;
						if (vcptr->mVolume < 0) 
                        {
							vcptr->mVolume = 0;
                        }
						break;
					}
					case FMUSIC_MOD_NOTEDELAY : 
					{
						vcptr->mVolume = oldvolume;
						vcptr->mFrequency   = oldfreq;
						vcptr->mNoteControl &= ~FMUSIC_FREQ;
						vcptr->mNoteControl &= !FMUSIC_PAN;
						vcptr->mNoteControl &= ~FMUSIC_VOLUME;
						vcptr->mNoteControl &= ~FMUSIC_TRIGGER;
						break;
					}
					case FMUSIC_MOD_PATTERNDELAY : 
					{
						mPatternDelay = paramy;
						mPatternDelay *= mSpeed;
						break;
					}
				};
				break;
			}
		};
        
        if (audible)
        {
            vcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext();

			if (!(vcptr->mFrequency + vcptr->mFrequencyDelta)) 
            {
				vcptr->mNoteControl &= ~FMUSIC_FREQ;	/* divide by 0 check */
            }

			if (vcptr->mNoteControl & FMUSIC_TRIGGER)
			{
                playSound(sptr, vcptr, false);
			}

			if (vcptr->mNoteControl & FMUSIC_VOLUME)	
            {
                vcptr->mChannel.setVolume(((float)(vcptr->mVolume + vcptr->mVolumeDelta) / 64.0f) * 0.5f);
            }
			if (vcptr->mNoteControl & FMUSIC_PAN)		
            {
                float finalpan = ((float)vcptr->mPan - 128.0f) * mPanSeparation;
            
                vcptr->mChannel.setPan(finalpan / 128.0f);
            }
			if (vcptr->mNoteControl & FMUSIC_FREQ)		
            {
                int finalfreq = vcptr->mFrequency + vcptr->mFrequencyDelta;

                if (finalfreq < 1)
                {
                    finalfreq = 1;
                }

                vcptr->mChannel.setFrequency((float)period2HZ(finalfreq));
            }
			if (vcptr->mNoteControl & FMUSIC_STOP)
			{
                vcptr->mChannel.stopEx(false, false, true, true, false, false, false);
                vcptr->mSampleOffset = 0;	/* if this channel gets stolen it will be safe */
			}
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

	[SEE_ALSO]
]
*/
// VERIFY: 2e95479f manual
// OFFSET: 0x0090AF70
FMOD_RESULT CodecMOD::updateEffects() 
{
	MusicNote   *current;
	int          count;

	current = mPattern[mOrderList[mOrder]].mData + (mRow * mNumChannels);

	if (!current) 
    {
        return FMOD_OK;
    }

	for (count=0; count < mNumChannels; count++,current++)
	{
		MusicChannelMOD         *cptr  = 0;
		MusicVirtualChannel     *vcptr = 0;
		MusicSample             *sptr  = 0;
		unsigned char			effect, paramx, paramy;

		cptr = (MusicChannelMOD *)mMusicChannel[count];
		
		if (cptr->mInstrument < mNumSamples) 
        {
			sptr = &mSample[cptr->mInstrument];
        }
		else
        {
			sptr = &gDummySample;
        }

		if (cptr->mVirtualChannelHead.isEmpty()) 
        {
			vcptr = &gDummyVirtualChannel;   /* no channels allocated yet */
        }
        else
        {
            vcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext();
        }

		effect = current->mEffect;              /* grab the effect number */
		paramx = current->mEffectParam >> 4;    /* grab the effect parameter x */
		paramy = current->mEffectParam & 0xF;   /* grab the effect parameter y */

		vcptr->mVolumeDelta	= 0;				/* this is for tremolo etc */
		vcptr->mFrequencyDelta = 0;				/* this is for vibrato / arpeggio etc */
		vcptr->mNoteControl	= 0;

		switch(effect) 
		{
			case FMUSIC_MOD_ARPEGGIO : 
			{
				if (current->mEffectParam > 0)
				{
					switch (mTick % 3) 
					{
						case 1: 
							/*
                                BUGFIX : FMOD 3.3 arpeggio was wrong
                            */
							vcptr->mFrequencyDelta = getAmigaPeriod(cptr->mNote + paramx, sptr->mMiddleC) - getAmigaPeriod(cptr->mNote, sptr->mMiddleC);
							break;
						case 2: 
							/*
                                BUGFIX : FMOD 3.3 arpeggio was wrong
                            */
							vcptr->mFrequencyDelta = getAmigaPeriod(cptr->mNote + paramy, sptr->mMiddleC) - getAmigaPeriod(cptr->mNote, sptr->mMiddleC);
							break;
					};
					vcptr->mNoteControl |= FMUSIC_FREQ;
				}
				break;
			}
			case FMUSIC_MOD_PORTAUP : 
			{
				vcptr->mFrequency -= (current->mEffectParam << 2); /* subtract freq */
				if (vcptr->mFrequency < 56) 
                {
					vcptr->mFrequency=56;  /* stop at B#8 */
                }
				vcptr->mNoteControl |= FMUSIC_FREQ;
				break;
			}
			case FMUSIC_MOD_PORTADOWN : 
			{
				vcptr->mFrequency += (current->mEffectParam << 2);
				vcptr->mNoteControl |= FMUSIC_FREQ;
				break;
			}
			case FMUSIC_MOD_PORTATO : 
			{
				cptr->portamento();
				break;
			}
			case FMUSIC_MOD_VIBRATO :
			{
				cptr->vibrato();
				break;
			}
			case FMUSIC_MOD_PORTATOVOLSLIDE : 
			{
				cptr->portamento();
				
				/* 
                    Slide up takes precedence over down
                */
				if (paramx) 
				{
					vcptr->mVolume += paramx;
					if (vcptr->mVolume > 64) 
                    {
						vcptr->mVolume = 64;
                    }
				}
				else if (paramy) 
				{
					vcptr->mVolume -= paramy;
					if (vcptr->mVolume < 0)  
                    {
						vcptr->mVolume = 0;
                    }
				}
				vcptr->mNoteControl |= FMUSIC_VOLUME;
				break;
			}
			case FMUSIC_MOD_VIBRATOVOLSLIDE : 
			{
				cptr->vibrato();

				/*
                    Slide up takes precedence over down
                */
				if (paramx) 
				{
					vcptr->mVolume += paramx;
					if (vcptr->mVolume > 64) 
                    {
						vcptr->mVolume = 64;
                    }
				}
				else if (paramy) 
				{
					vcptr->mVolume -= paramy;
					if (vcptr->mVolume < 0)  
                    {
						vcptr->mVolume = 0;
                    }
				}
				vcptr->mNoteControl |= FMUSIC_VOLUME;
				break;
			}
			case FMUSIC_MOD_TREMOLO :
			{
				cptr->tremolo();
				break;
			}
			case FMUSIC_MOD_VOLUMESLIDE : 
			{
				/*
                    Slide up takes precedence over down
                */
				if (paramx) 
				{
					vcptr->mVolume += paramx;
					if (vcptr->mVolume > 64) 
                    {
						vcptr->mVolume = 64;
                    }
				}
				else if (paramy) 
				{
					vcptr->mVolume -= paramy;
					if (vcptr->mVolume < 0)  
                    {
						vcptr->mVolume = 0;
                    }
				}

				vcptr->mNoteControl |= FMUSIC_VOLUME;
				break;
			}

			/*
                Extended PT effects
            */
			case FMUSIC_MOD_SPECIAL: 
			{
				switch (paramx) 
				{
					case FMUSIC_MOD_NOTECUT: 
					{
						if (mTick==paramy) 
						{
							vcptr->mVolume = 0;
							vcptr->mNoteControl |= FMUSIC_VOLUME;
						}
						break;
					}
					case FMUSIC_MOD_RETRIG : 
					{
						if (!paramy) 
                        {
							break; /* divide by 0 bugfix */
                        }
						
						if (!(mTick % paramy)) 
                        {
							vcptr->mNoteControl |= FMUSIC_TRIGGER;
							vcptr->mNoteControl |= FMUSIC_VOLUME;
							vcptr->mNoteControl |= FMUSIC_PAN;
                        }
						
						break;
					}
					case FMUSIC_MOD_NOTEDELAY : 
					{
						if (mTick == paramy) 
						{
							if (vcptr == &gDummyVirtualChannel) 
							{
                                FMOD_RESULT result;
								result = spawnNewVirtualChannel(cptr, sptr, &vcptr);
								if (result != FMOD_OK)
								{
									vcptr             = &gDummyVirtualChannel; /* no channels allocated yet */
									vcptr->mSample    = &gDummySample;
								}
							}

							if (current->mNumber) 
							{
								vcptr->mVolume = sptr->mDefaultVolume;
								vcptr->mNoteControl |= FMUSIC_VOLUME;
							}
							vcptr->mPan	= mDefaultPan[count];
							vcptr->mFrequency = cptr->mPeriod;
							vcptr->mNoteControl |= FMUSIC_FREQ;
							vcptr->mNoteControl |= FMUSIC_PAN;
							vcptr->mNoteControl |= FMUSIC_TRIGGER;
						}
						else
						{
							vcptr->mNoteControl &= ~FMUSIC_VOLUME;
							vcptr->mNoteControl &= ~FMUSIC_FREQ;
							vcptr->mNoteControl &= ~FMUSIC_TRIGGER;
						}
						break;
					}
				};
				break;
			}
		};

        {
            vcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext();

		    if (!(vcptr->mFrequency + vcptr->mFrequencyDelta)) 
            {
			    vcptr->mNoteControl &= ~FMUSIC_FREQ;	/* divide by 0 check */
            }

		    if (vcptr->mNoteControl & FMUSIC_TRIGGER)		
		    {
                playSound(sptr, vcptr, false);
		    }

		    if (vcptr->mNoteControl & FMUSIC_VOLUME)	
            {
                vcptr->mChannel.setVolume(((float)(vcptr->mVolume + vcptr->mVolumeDelta) / 64.0f) * 0.5f);
            }
		    if (vcptr->mNoteControl & FMUSIC_PAN)		
            {
                float finalpan = ((float)vcptr->mPan - 128.0f) * mPanSeparation;

                vcptr->mChannel.setPan(finalpan / 128.0f);
            }
		    if (vcptr->mNoteControl & FMUSIC_FREQ)		
            {
                int finalfreq = vcptr->mFrequency + vcptr->mFrequencyDelta;

                if (finalfreq < 1)
                {
                    finalfreq = 1;
                }

                vcptr->mChannel.setFrequency((float)period2HZ(finalfreq));
            }
		    if (vcptr->mNoteControl & FMUSIC_STOP)
		    {
                vcptr->mChannel.stopEx(false, false, true, true, false, false, false);
			    vcptr->mSampleOffset = 0;	/* if this channel gets stolen it will be safe */
		    }
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

	[SEE_ALSO]
]
*/
// VERIFY: f4046953 manual
// OFFSET: 0x0090B540
FMOD_RESULT CodecMOD::update(bool audible)
{
	if (mTick == 0)									/* new note */
	{
        if (mFinished && !mLooping)
        {
            stop();
        }
        else
        {
		    /*
                Process any commands to set the next order/row from previous row
            */
		    if (mNextOrder >= 0)
            {
			    mOrder = mNextOrder;
//			    checkCallback(FMUSIC_CALLBACK_ORDER, (unsigned char)mOrder);
		        if (mNextOrder >= 0)
                {
			        mOrder = mNextOrder;
                }
        	    mNextOrder = -1;
            }
		    if (mNextRow >= 0)   
            {
			    mRow = mNextRow;
//			    checkCallback(FMUSIC_CALLBACK_ROW, (unsigned char)mRow);
		        if (mNextRow >= 0)   
                {
			        mRow = mNextRow;
                }
        	    mNextRow = -1;
            }
    
		    updateNote(audible);                           /* Update and play the note */

		    /*
                If there were no row commands
            */
		    if (mNextRow == -1)
		    {
			    mNextRow = mRow+1;
			    if (mNextRow >= 64)					/* if end of pattern */
			    { 
				    mNextOrder = mOrder+1;		/* so increment the order */

				    if (mNextOrder >= mNumOrders) 
                    {
				        mNextOrder = mRestart;
                    }

				    mNextRow = 0;					/* start at top of pattern */
			    }
		    }
        }
	}
	else if (audible)
    {
		updateEffects();				/* Else update the inbetween row effects */
    }

    if (mSpeed)
    {
	    mTick++;
	    if (mTick >= mSpeed + mPatternDelay)
	    {
		    mPatternDelay = 0;
		    mTick = 0;
	    }
    }
    else
    {
        mFinished = true;
        mTick = -1;
    }

    mPCMOffset += mMixerSamplesPerTick;

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
// VERIFY: aaef1e84 manual
// OFFSET: 0x0090BDA0
FMOD_RESULT CodecMOD::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    FMOD_RESULT     result = FMOD_OK;
	int 			count;
    char            temp[4];
    unsigned int    lengthbytes;

    if (!(mFile->mSeekable))
    {
        return FMOD_ERR_FORMAT;
    }

    init(FMOD_SOUND_TYPE_MOD);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecMOD::openInternal", "attempting to open as MOD..\n"));

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

	/*
        Get size of file in bytes
    */
	result = mFile->getSize(&lengthbytes);
    if (result != FMOD_OK)
    {
        return result;
    }

    mFile->setBigEndian(true);

	/*
        Verify Format
    */
	result = mFile->seek(1080, SEEK_SET);
    if (result != FMOD_OK)
    {
        mFile->setBigEndian(false);
        return result;
    }
	
    /* 
        MOD signature (M.K. etc),...  
    */
    result = mFile->read(temp, 1, 4, 0);
    if (result != FMOD_OK)
    {
        mFile->setBigEndian(false);
        return result;
    }

	/*
        Protracker mods && ft2 multichannel .mod's
    */
	if (FMOD_strncmp((char *)temp, "M.K.", 4) && 
        FMOD_strncmp((char *)temp, "M!K!", 4) && 
        FMOD_strncmp((char *)temp, "6CHN", 4) && 
        FMOD_strncmp((char *)temp, "8CHN", 4) &&
		FMOD_strncmp((char *)temp+2, "CH", 2) && 
        FMOD_strncmp((char *)temp+1, "CHN", 3))  
	{
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecMOD::openInternal", "'M.K.' etc ID check failed [%c%c%c%c]\n", temp[0], temp[1], temp[2], temp[3]));		

        mFile->setBigEndian(false);
		return FMOD_ERR_FORMAT;
	}

	/*
        Protracker mods
    */
	if (!FMOD_strncmp(temp, "M.K.", 4))
    {
		mNumChannels = 4;
    }
	else if (!FMOD_strncmp(temp, "M!K!", 4))
    {
		mNumChannels = 4;
    }
	else if (!FMOD_strncmp(temp, "FLT4", 4))
    {
		mNumChannels = 4;
    }

	/*
        FT1 multichannel mods
    */
	else if (!FMOD_strncmp(temp, "6CHN", 4))
    {
		mNumChannels = 6;
    }
	else if (!FMOD_strncmp(temp, "8CHN", 4))
    {
		mNumChannels = 8;
    }

	/*
        FT2 multichannel .mod's
    */
	else if (!FMOD_strncmp(temp+2, "CH", 2)) 
	{
		temp[3] = 0;	/* null terminate this string */
		mNumChannels = atoi(temp); 
	}
    else if (!FMOD_strncmp(temp+1, "CHN", 3)) 
    {
        mNumChannels = (temp[0]-'0');
    }
    else
    {
        mNumChannels = 0;
    }

    if (mNumChannels < 1 || mNumChannels > 32)
    {
        mFile->setBigEndian(false);
        return FMOD_ERR_FORMAT;
    }

    result = metaData(FMOD_TAGTYPE_FMOD, "Number of channels", &mNumChannels, sizeof(mNumChannels), FMOD_TAGDATATYPE_INT, false);
    if (result != FMOD_OK)
    {
        return result;
    }

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        mFile->setBigEndian(false);
        return result;
    }


	/*
        Set a few default values for this format
    */
    for (count = 0; count < MUSIC_MAXCHANNELS; count++)
    {
        mMusicChannel[count] = 0;
    }

	mPattern        = 0;
	mPanSeparation  = 0.8f;
	mMasterSpeed    = 1.0f;
    mLooping        = true;

	mNumSamples     = MOD_MAXSAMPLES;
	mDefaultSpeed   = 6;
	mDefaultBPM     = 125;
	mNumPatterns    = 0;              	/* currently 0, to be set later. */
	mRestart        = 0;

	/*
        Start at the beginning of the file and read in module name.
    */
	result = mFile->read(mSongName, 1, 20, 0);
    if (result != FMOD_OK)
    {
        mFile->setBigEndian(false);
        return result;
    }

	/*
        Load instrument headers
    */
	for (count = 0; count < mNumSamples; count++) 
	{
		unsigned int 	count2;
        char            sample_name[28];
        FMOD_MODE       sample_mode;
        unsigned int    sample_length;
        unsigned int    sample_loopstart;
        unsigned int    sample_looplength;
        unsigned char   sample_volume;

        FMOD_memset(&mSample[count], 0, sizeof(MusicSample));

		/*
            Read sample name, and fix up bad signed characters by making them spaces
        */
		result = mFile->read(sample_name, 1, 22, 0);
        if (result != FMOD_OK)
        {
            mFile->setBigEndian(false);
            return result;
        }

		for (count2 = 0; count2 < 28; count2++) 
        {
			if (sample_name[count2] < 32) 
            {
				sample_name[count2] = 0;
            }
        }

        {
            char s[256];
         
            sprintf(s, "Sample name %d", count);
         
            result = metaData(FMOD_TAGTYPE_FMOD, s, sample_name, 28, FMOD_TAGDATATYPE_STRING, false);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        sample_mode = FMOD_SOFTWARE | FMOD_2D;

		/*
            Read rest of information.
        */
        result = mFile->getWord(&sample_length);
        if (result != FMOD_OK)
        {
            mFile->setBigEndian(false);
            return result;
        }
        sample_length *= 2;

        result = mFile->getByte(&mSample[count].mMiddleC);
        if (result != FMOD_OK)
        {
            mFile->setBigEndian(false);
            return result;
        }        
        fineTune2Hz(mSample[count].mMiddleC, &mSample[count].mMiddleC);

        result = mFile->getByte(&sample_volume);
        if (result != FMOD_OK)
        {
            mFile->setBigEndian(false);
            return result;
        }
        mSample[count].mDefaultVolume = sample_volume;

        result = mFile->getWord(&sample_loopstart);
        if (result != FMOD_OK)
        {
            mFile->setBigEndian(false);
            return result;
        }
        if (sample_loopstart * 2 < sample_length)
        {
            sample_loopstart *= 2;
        }

        result = mFile->getWord(&sample_looplength);
        if (result != FMOD_OK)
        {
            mFile->setBigEndian(false);
            return result;
        }
        sample_looplength *= 2;

		if (sample_loopstart + sample_looplength > sample_length) 
        {
			sample_looplength = sample_length - sample_loopstart;
        }
		
		if (sample_looplength > 2) 
        {
			sample_mode |= FMOD_LOOP_NORMAL;
        }
		else				   
		{
			sample_mode |= FMOD_LOOP_OFF;
			sample_loopstart = 0;
			sample_looplength = sample_length;
		}

		/*
            ALLOCATE MEMORY FOR THE SAMPLE BUFFER
        */
		if (sample_length)
		{
            FMOD_CREATESOUNDEXINFO exinfo;

            FMOD_memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));            
            exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
            exinfo.length           = sample_length;
            exinfo.numchannels      = 1;
            exinfo.defaultfrequency = mSample[count].mMiddleC;
            exinfo.format           = FMOD_SOUND_FORMAT_PCM8;

            result = mSystem->createSound(0, sample_mode | FMOD_OPENUSER, &exinfo, &mSample[count].mSound);
            if (result != FMOD_OK)
            {
                mFile->setBigEndian(false);
                return result;
            }

            if (sample_mode & FMOD_LOOP_NORMAL)
            {
                result = mSample[count].mSound->setLoopPoints(sample_loopstart, FMOD_TIMEUNIT_PCM, sample_loopstart + sample_looplength - 1, FMOD_TIMEUNIT_PCM);
                if (result != FMOD_OK)
                {
                    mFile->setBigEndian(false);
                    return result;
                }
            }
            mSample[count].mLoopStart = sample_loopstart;
            mSample[count].mLoopLength = sample_looplength;
		}
	}

	/*
        Load order data
    */
    
    result = mFile->getByte(&mNumOrders);
    if (result != FMOD_OK)
    {
        mFile->setBigEndian(false);
        return result;
    }

    /* 
        Restart pos.  Usually 127 so it isnt reliable 
    */
    result = mFile->getByte();
    if (result != FMOD_OK)
    {
        mFile->setBigEndian(false);
        return result;
    }

    /* 
        Read in 128 orders worth of pattern numbers.
    */
    FMOD_memset(mOrderList, 0, MUSIC_MAXORDERS);
	result = mFile->read(mOrderList, 1, 128, 0);   
    if (result != FMOD_OK)
    {
        mFile->setBigEndian(false);
        return result;
    }

	for (count = 0; count < 128; count++)
	{
		if (mOrderList[count] > mNumPatterns) 
        {
			mNumPatterns = mOrderList[count];
        }
	}
	mNumPatterns++;

    /* 
        MOD signature (M.K. etc)
    */
    result = mFile->getDword();
	if (result != FMOD_OK)
    {
        mFile->setBigEndian(false);
        return result;
    }
	
	/*
        ALLOCATE MUSIC CHANNELS
    */
    for (count = 0; count < mNumChannels; count++)
    {
        mMusicChannel[count] = FMOD_Object_Calloc(MusicChannelMOD);
        if (!mMusicChannel[count])
        {
            mFile->setBigEndian(false);
            return FMOD_ERR_MEMORY;
        }
    }

	/*
        Set pan values for the mod format
    */
	for (count = 0; count < mNumChannels; count++)
	{
		mDefaultPan[count] = (((count + 1) & 2) == 0) ? 0 : 255;
	}

	/*
        Alloc pattern array
    */
	mNumPatternsMem = mNumPatterns;
	mPattern = (MusicPattern *)FMOD_Memory_Calloc(mNumPatternsMem * sizeof(MusicPattern));
	if (!mPattern) 
	{
        mFile->setBigEndian(false);
		return FMOD_ERR_MEMORY;
	}

	/*
        Load pattern data
    */
	for (count = 0; count < mNumPatterns; count++) 
	{
		MusicPattern   *pptr;
		MusicNote      *nptr;
		int 			count2;
		unsigned short	period;

		pptr = &mPattern[count];
		pptr->mRows = 64;

		/*
            Allocate memory for pattern buffer
        */
		pptr->mData = (MusicNote *)FMOD_Memory_Calloc(mNumChannels * pptr->mRows * sizeof(MusicNote));
		if (!pptr->mData)
		{
            mFile->setBigEndian(false);
			return FMOD_ERR_MEMORY;
		}

		nptr = pptr->mData;

		/*
            Read the pattern data in
        */
		for (count2 = 0; count2 < (mNumChannels * pptr->mRows); count2++) 
		{
			unsigned int count3;

			/*
                Load up 4 bytes of note information from file
            */
			result = mFile->read(temp, 1, 4, 0);
            if (result != FMOD_OK)
            {
                mFile->setBigEndian(false);
                return result;
            }

			/*
                Store sample number
            */
			nptr->mNumber = (((unsigned char)temp[0] & 0xF0) + ((unsigned char)temp[2] >> 4));

			/*
                Store note
            */
			period = (((unsigned char)temp[0] & 0xF) << 8) + (unsigned char)temp[1];

			/*
                Convert the amiga period to a note number
            */
			nptr->mNote = 0;
			
			for (count3=0; count3 < 108; count3++) 
			{
				if (period >= gPeriodTable[count3+24]) 
				{
					nptr->mNote = (unsigned char)count3+1;
					break;
				}
			}

			/*
                Store volume byte (mods dont have one so NOVOL = empty)
            */
			nptr->mVolume = 0;
			
			/*
                Store effects and arguments
            */
			nptr->mEffect      = (unsigned char)temp[2] & 0xF;	/* Effect number */
			nptr->mEffectParam = (unsigned char)temp[3];			/* parameter */

			nptr++;
		}
	}


#if 1
	/* 
        BUGFIX : FMOD 3.3
	    Seek to the end of the file minus the sum of all the samples.
	    This was done because some mods specify more patterns than they actually contain!!!!
	*/
	{
		unsigned int totallen = 0, curroffset, filelen;
		
		result = mFile->tell(&curroffset);
        if (result != FMOD_OK)
        {
            mFile->setBigEndian(false);
            return result;
        }

		result = mFile->getSize(&filelen);
        if (result != FMOD_OK)
        {
            mFile->setBigEndian(false);
            return result;
        }

		for (count=0; count < mNumSamples; count++)
		{
			MusicSample *sptr;

			sptr = &mSample[count];
			
            if (sptr->mSound)
            {
			    totallen += sptr->mSound->mLength;
            }
		}

		/*
            Some sort of safety check i guess
        */
		if (filelen - totallen > 1080 && (filelen < (curroffset + totallen)))    /* BUFIX - FMOD 3.70.  Only do it if the filelen is smaller than the expected size */
        {
			result = mFile->seek(filelen - totallen, SEEK_SET);
            if (result != FMOD_OK)
            {
                mFile->setBigEndian(false);
                return result;
            }

        }
	}
#endif
    
	/*
        Load sample data
    */
	for (count = 0; count < mNumSamples; count++) 
	{
		MusicSample *sptr;

		sptr = &mSample[count];

		if (sptr->mSound)
		{
            void *ptr1, *ptr2;
            unsigned int len1, len2, lenbytes;
            
            sptr->mSound->getBytesFromSamples(sptr->mSound->mLength, &lenbytes);

            result = sptr->mSound->lock(0, lenbytes, &ptr1, &ptr2, &len1, &len2);
            if (result != FMOD_OK)
            {
                mFile->setBigEndian(false);
                return result;
            }

            if (ptr1 && len1)
            {
			    result = mFile->read(ptr1, 1, len1, 0);
                if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
                {
                    mFile->setBigEndian(false);                
                    return result;
                }
            }
			
            result = sptr->mSound->unlock(ptr1, ptr2, len1, len2);
            if (result != FMOD_OK)
            {
                mFile->setBigEndian(false);
                return result;
            }
		}
	}

    mFile->setBigEndian(false);


    mWaveFormatMemory = (FMOD_CODEC_WAVEFORMAT *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_WAVEFORMAT));
    if (!mWaveFormatMemory)
    {
        return FMOD_ERR_MEMORY;
    }

    waveformat    = mWaveFormatMemory;
    waveformat[0].lengthbytes = lengthbytes;

    /*
        Set up general codec parameters.
    */
    if (userexinfo && userexinfo->format != FMOD_SOUND_FORMAT_NONE)
    {
        waveformat[0].format = userexinfo->format;
    }
    #ifndef PLATFORM_PS3
    else if (usermode & FMOD_SOFTWARE)
    {
        waveformat[0].format = FMOD_SOUND_FORMAT_PCMFLOAT;
    }
    #endif
    else
    {
        waveformat[0].format = FMOD_SOUND_FORMAT_PCM16;
    }

	waveformat[0].channels = 2;
    FMOD_strncpy(waveformat[0].name, mSongName, FMOD_STRING_MAXNAMELEN);

    result = mSystem->getSoftwareFormat(&waveformat[0].frequency, 0, 0, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

	mSrcDataOffset  = 0;

    SoundI::getBytesFromSamples(1, (unsigned int *)&waveformat[0].blockalign, waveformat[0].channels, waveformat[0].format);

    /*
        Create a head unit that software channels connect to.
    */
    {
        FMOD_DSP_DESCRIPTION_EX descriptionex;
        
        FMOD_memset(&descriptionex, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));
        FMOD_strcpy(descriptionex.name, "FMOD MOD Target Unit");
        descriptionex.version       = 0x00010100;
        descriptionex.channels      = waveformat[0].channels;
        descriptionex.mFormat       = waveformat[0].format;
        descriptionex.mCategory     = FMOD_DSP_CATEGORY_SOUNDCARD;

        result = mSystem->createDSP(&descriptionex, &mDSPHead);
        if (result != FMOD_OK)
        {
            return result;
        }

        mDSPHead->mDefaultFrequency = (float)waveformat[0].frequency;
    }

    /*
        Create a pool of virtual channels.
    */
    {
        mNumVirtualChannels = mNumChannels;
        mVirtualChannel = (MusicVirtualChannel *)FMOD_Memory_Calloc(sizeof(MusicVirtualChannel) * mNumVirtualChannels);
        if (!mVirtualChannel)
        {
            return FMOD_ERR_MEMORY;
        }
        for (count = 0; count < mNumVirtualChannels; count++)
        {
            new (&mVirtualChannel[count]) MusicVirtualChannel;
        }
    }

    /*
        Create a pool of real channels.  2x the number of virtual channels for double buffer channel usage.
    */
    {
        int numrealchannels = mNumVirtualChannels * 2;

        mChannelPool = FMOD_Object_Calloc(ChannelPool);
        if (!mChannelPool)
        {
            return FMOD_ERR_MEMORY;
        }

        result = mChannelPool->init(mSystem, 0, numrealchannels);
        if (result != FMOD_OK)
        {
            return result;
        }

        mChannelSoftware = (ChannelSoftware *)FMOD_Memory_Calloc(sizeof(ChannelSoftware) * numrealchannels);
        if (!mChannelSoftware)
        {
            return FMOD_ERR_MEMORY;
        }

        for (count = 0; count < numrealchannels; count++)
        {
            new (&mChannelSoftware[count]) ChannelSoftware;
            mChannelPool->setChannel(count, &mChannelSoftware[count], mDSPHead);
            mChannelSoftware[count].allowReverb(false);
        }
    }

    if ((usermode & FMOD_ACCURATETIME) || (usermode & FMOD_CREATESAMPLE))
    {
        mVisited = (bool *)FMOD_Memory_Calloc(sizeof(bool) * mNumOrders * FMUSIC_MAXROWS);
        if (!mVisited)
        {
            return FMOD_ERR_MEMORY;
        }

        calculateLength();
    }
    else
    {
        mVisited = 0;
        waveformat[0].lengthpcm = (unsigned int)-1;         /* Thanks to those stupid comment markers! */
    }

    /*
        Fill out base class members, also pointing to or allocating storage for them.
    */
    numsubsounds  = 0;

    play();

    return result;
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
// VERIFY: f076385f auto
// OFFSET: 0x0090B680
FMOD_RESULT CodecMOD::closeInternal()
{
    int count;

    stop();

    for (count = 0; count < mNumSamples; count++) 
	{
        if (mSample[count].mSound)
        {
            mSample[count].mSound->release();
            mSample[count].mSound = 0;
        }
    }

    if (mVirtualChannel)
    {
        FMOD_Memory_Free(mVirtualChannel);
        mVirtualChannel = 0;
    }

    if (mChannelPool)
    {
        mChannelPool->release();
        mChannelPool = 0;
    }

    if (mChannelSoftware)
    {
        FMOD_Memory_Free(mChannelSoftware);
        mChannelSoftware = 0;
    }
        
    if (mPattern)
    {
	    for (count = 0; count < mNumPatterns; count++) 
	    {
            if (mPattern[count].mData)
            {
		        FMOD_Memory_Free(mPattern[count].mData);
                mPattern[count].mData = 0;
            }
        }

        FMOD_Memory_Free(mPattern);
        mPattern = 0;
    }

	for (count = 0; count < mNumChannels; count++) 
	{
        if (mMusicChannel[count])
        {
		    FMOD_Memory_Free(mMusicChannel[count]);
            mMusicChannel[count] = 0;
        }
    }

    if (mVisited)
    {
        FMOD_Memory_Free(mVisited);
        mVisited = 0;
    }

    if (mDSPHead)
    {
        mDSPHead->release();
        mDSPHead = 0;
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
// VERIFY: 168ce95c manual
// OFFSET: 0x0090B9B0
FMOD_RESULT CodecMOD::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    FMOD_RESULT              result = FMOD_OK;
    unsigned int             numsamples;
    int                      numchannels;
    FMOD_OS_CRITICALSECTION *crit = mSystem->mDSPCrit;

    numchannels = waveformat[0].channels;

    SoundI::getSamplesFromBytes(sizebytes, &numsamples, numchannels, waveformat[0].format);

	if (mPlaying && mMasterSpeed)
	{
		unsigned int    mixedsofar = 0;
		unsigned int    mixedleft = mMixerSamplesLeft;
		unsigned int    samplestomix;
        char           *destptr = (char *)buffer;

		while (mixedsofar < numsamples)
		{
            unsigned int read, bytes;
            void        *outbuffer = destptr;

			if (!mixedleft)
			{
                result = update(true);
                if (result != FMOD_OK)
                {
                    return result;
                }

				samplestomix = mMixerSamplesPerTick;
				mixedleft = samplestomix;
			}
			else
            {
				samplestomix = mixedleft;
            }

			if (mixedsofar + samplestomix > numsamples)
            {
				samplestomix = numsamples - mixedsofar;
            }

            read = samplestomix;

            FMOD_OS_CriticalSection_Enter(crit);
            if (buffer)
			{
                result = mDSPHead->execute(outbuffer, &outbuffer, &read, numchannels, &numchannels, FMOD_SPEAKERMODE_STEREO_LINEAR);
                if (result != FMOD_OK)
                {
                    FMOD_OS_CriticalSection_Leave(crit);
                    return result;
                }

                mDSPHead->resetVisited();
            }
            FMOD_OS_CriticalSection_Leave(crit);

            SoundI::getBytesFromSamples(read, &bytes, numchannels, waveformat[0].format);

            if (destptr != outbuffer && buffer)
            {
                FMOD_memcpy(destptr, outbuffer, bytes);
            }

			mixedleft  -= read;
			destptr    += bytes;
			mixedsofar += read;
		}

		mMixerSamplesLeft = mixedleft;
	}

    if (bytesread)
    {
        *bytesread = sizebytes;
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
// VERIFY: 4261427c auto
// OFFSET: 0x0090B860
FMOD_RESULT CodecMOD::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    if (postype == FMOD_TIMEUNIT_MODORDER)
    {
        play();
        mNextOrder = mOrder = position;
        
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_PCM)
    {
        bool restarted = false;

        if (position == mPCMOffset)
        {
            return FMOD_OK;
        }
    
        /*
            Want to seek backwards, start from the start.
        */
        if (position < mPCMOffset)
        {
            play();
            restarted = true;
        }

        while (mPCMOffset < position)
        {
            update(true);
        }

        if (restarted)
        {
            bool oldplaying = mPlaying;
            bool oldfinished = mFinished;
        
            stop();

	        mPlaying = oldplaying;
            mFinished = oldfinished;
        }

        return FMOD_OK;
    }

    return FMOD_ERR_FORMAT;
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
// VERIFY: 4f7dda69 manual
// OFFSET: 0x0090CCC0
FMOD_RESULT F_CALLBACK CodecMOD::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecMOD *cmod = (CodecMOD *)codec;

    return cmod->openInternal(usermode, userexinfo);
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
// VERIFY: 7f5953ba manual
// OFFSET: 0x0090B900
FMOD_RESULT F_CALLBACK CodecMOD::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecMOD *cmod = (CodecMOD *)codec;

    return cmod->closeInternal();
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
// VERIFY: 2a03dde9 auto
// OFFSET: 0x0090BD70
FMOD_RESULT F_CALLBACK CodecMOD::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecMOD *cmod = (CodecMOD *)codec;

    return cmod->readInternal(buffer, sizebytes, bytesread);
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
// VERIFY: ad2fa33d auto
// OFFSET: 0x0090B930
FMOD_RESULT F_CALLBACK CodecMOD::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype   )
{
    CodecMOD *cmod = (CodecMOD *)codec;

    return cmod->setPositionInternal(subsound, position, postype);
}
}

#endif

