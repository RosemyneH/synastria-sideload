#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_S3M

#include "fmod.h"

#include "fmod_channel_software.h"
#include "fmod_codec_s3m.h"
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


FMOD_CODEC_DESCRIPTION_EX s3mcodec;


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
        return CodecS3M::getDescriptionEx();
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: ff379bfa manual
// OFFSET: 0x0090A5A0
FMOD_CODEC_DESCRIPTION_EX *CodecS3M::getDescriptionEx()
{
    FMOD_memset(&s3mcodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    s3mcodec.name            = "FMOD S3M Codec";
    s3mcodec.version         = 0x00010100;
    s3mcodec.timeunits       = (FMOD_TIMEUNIT)(FMOD_TIMEUNIT_PCM | FMOD_TIMEUNIT_MODORDER | FMOD_TIMEUNIT_MODROW | FMOD_TIMEUNIT_MODPATTERN);
    s3mcodec.defaultasstream = 1;
    s3mcodec.open            = &CodecS3M::openCallback;
    s3mcodec.close           = &CodecS3M::closeCallback;
    s3mcodec.read            = &CodecS3M::readCallback;
    s3mcodec.getlength       = &MusicSong::getLengthCallback;
    s3mcodec.setposition     = &CodecS3M::setPositionCallback;
    s3mcodec.getposition     = &MusicSong::getPositionCallback;

    s3mcodec.mType = FMOD_SOUND_TYPE_S3M;
    s3mcodec.mSize = sizeof(CodecS3M);

    return &s3mcodec;
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
// VERIFY: d3cdfbb2 auto
// OFFSET: 0x00908EF0
FMOD_RESULT CodecS3M::calculateLength()
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

	[SEE_ALSO]
]
*/
// VERIFY: b49a6c2d manual
// OFFSET: 0x00907680
FMOD_RESULT MusicChannelS3M::volumeSlide() 
{
	MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();

	if (!(mVolumeSlide & 0xF)) 
    {
		vcptr->mVolume += (mVolumeSlide >> 4);
    }
	if (!(mVolumeSlide >>  4)) 
    {
		vcptr->mVolume -= (mVolumeSlide & 0xF);
    }
	
	if (vcptr->mVolume > 64) 
    {
		vcptr->mVolume = 64;
    }
	if (vcptr->mVolume < 0)  
    {
		vcptr->mVolume = 0;
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
FMOD_RESULT MusicChannelS3M::portamento()
{
    MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();

	/*
        Slide pitch down if it needs too.
    */
	if (vcptr->mFrequency < mPortaTarget) 
	{
		vcptr->mFrequency += (mPortaSpeed << 2);
		if (vcptr->mFrequency > mPortaTarget) 
        {
			vcptr->mFrequency  = mPortaTarget;
        }
	}

	/*
        Slide pitch up if it needs too.
    */
	if (vcptr->mFrequency  > mPortaTarget) 
	{
		vcptr->mFrequency  -= (mPortaSpeed << 2);
		if (vcptr->mFrequency  < mPortaTarget) 
        {
			vcptr->mFrequency = mPortaTarget;
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
	track - the track number to do the vibrato too           
 
	[RETURN_VALUE]

	[REMARKS]
	AND'ing temp with 31 removes the sign bit giving the abs value

	[SEE_ALSO]
]
*/
FMOD_RESULT MusicChannelS3M::vibrato() 
{
	int delta;
	unsigned char temp;
	MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();

	temp = (unsigned char)(mVibPos & 31);

	switch (mWaveControl & 3)
	{
		case 0: 
				delta = gSineTable[temp];			/* sine */
				break;
		case 1: 
				temp <<= 3;								/* ramp down */
				if (mVibPos < 0) 
                {
					temp=255-temp;
                }
				delta=temp;
				break;
		case 2: 
				delta = 255;							/* square */
				break;
		case 3: 
				delta = FMOD_RAND()&255;						/* random */
				break;
		default : 
				delta = 0;
				break;
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
// VERIFY: 121f8bf1 auto
// OFFSET: 0x00907750
FMOD_RESULT MusicChannelS3M::tremolo() 
{
	int 			delta;
	unsigned char	temp;
	MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();

	temp = (mTremoloPosition & 31);

	switch((mWaveControl >> 4) & 3)
	{
		case 0: delta = gSineTable[temp];   /* sine */
				break;
		case 1: temp <<= 3;                 /* ramp down */
				if (mTremoloPosition < 0) 
                {
                    temp=255-temp;
                }
				delta=temp;
				break;
		case 2: delta = 255;                /* square */
				break;
		case 3: delta = FMOD_RAND()&255;    /* random */
				break;
		default : 
				delta = 0;
				break;
	};

	delta *= mTremoloDepth;
	delta >>= 6;

	if (mTremoloPosition >= 0) 
	{
		if (vcptr->mVolume+delta > 64) 
        {
			delta = 64 - vcptr->mVolume;
        }
		vcptr->mVolumeDelta = delta;
	}
	else 
	{
		if (((short)vcptr->mVolume-delta) < 0) 
        {
			delta = vcptr->mVolume;
        }
		vcptr->mVolumeDelta = -delta;
	}

	mTremoloPosition += mTremoloDepth;
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
// VERIFY: aef22d33 auto
// OFFSET: 0x00907830
FMOD_RESULT MusicChannelS3M::fineVibrato() 
{
	int				delta;
	unsigned char	temp;
	MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();

	temp = (mVibPos & 31);

	switch (mWaveControl & 3)
	{
		case 0: delta = gSineTable[temp];       /* sine */
				break;
		case 1: temp <<= 3;                     /* ramp down */
				if (mVibPos < 0) 
                {
                    temp=255-temp;
                }
				delta=temp;
				break;
		case 2: delta = 255;                    /* square */
				break;
		case 3: delta = FMOD_RAND()&255;        /* random */
				break;
		default : 
				delta = 0;
				break;
	};

	delta *= mVibDepth;
	delta >>=7;

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

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 38c111be manual
// OFFSET: 0x009078F0
FMOD_RESULT CodecS3M::updateNote(bool audible)
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
	for (count = 0; count < mNumChannels; count++,current++)
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
            First store note and instrument number if there was one
        */
		if (current->mNumber) 
		{
			cptr->mInstrument = current->mNumber - 1; /* remember the Instrument # */

//		    if (mInstCallback[current->mNumber] && mInstCallback[current->mNumber]->callback)
//            {
//				FMUSIC_CheckCallback(mod, FMUSIC_CALLBACK_INSTRUMENT, current->mNumber);
//            }
		}
		if (current->mNote && current->mNote != FMUSIC_KEYOFF) 
        {
			cptr->mNote = current->mNote-1;	/* remember the note */
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
            if there is no more tremolo, set volume to volume + last tremolo delta
        */
		if (cptr->mRecentEffect == FMUSIC_S3M_TREMOLO && current->mEffect != FMUSIC_S3M_TREMOLO)
        {
			vcptr->mVolume += vcptr->mVolumeDelta;		
        }
		cptr->mRecentEffect = current->mEffect;

		vcptr->mVolumeDelta = 0;
		vcptr->mNoteControl	= 0;

		/*
            PROCESS NOTE
        */
		if (current->mNote && current->mNote != FMUSIC_KEYOFF) 
		{
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
                get period according to relative note, c2spd and finetune
            */
			cptr->mNote	= current->mNote-1;					/* now remember the note */
			if (sptr->mMiddleC)
            {
				cptr->mPeriod = 8363L * gPeriodTable[cptr->mNote] / sptr->mMiddleC;
            }
			else
            {
				cptr->mPeriod = gPeriodTable[cptr->mNote];
            }

			vcptr->mPan = mDefaultPan[count];
			
			/*
                Frequency only changes if there are no portamento effects
            */
			if (current->mEffect != FMUSIC_S3M_PORTATO && 
				current->mEffect != FMUSIC_S3M_PORTATOVOLSLIDE) 
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
			cptr->mTremorPosition	= 0;								/* retrigger tremor count */

			/*
                Retrigger tremolo and vibrato waveforms
            */
			if ((cptr->mWaveControl & 0xF) < 4) 
            {
				cptr->mVibPos=0;
            }
			if ((cptr->mWaveControl >> 4) < 4)  
            {
				cptr->mTremoloPosition = 0;
            }
		}

		vcptr->mFrequencyDelta	= 0;
		vcptr->mNoteControl		|= FMUSIC_FREQ | FMUSIC_VOLUME | FMUSIC_PAN;

		/*
            PROCESS VOLUME BYTE
        */
		if (current->mVolume) 
        {
			vcptr->mVolume = current->mVolume - 1;
        }

		/*
            PROCESS KEY OFF
        */
		if (current->mNote == FMUSIC_KEYOFF) 
        {
			vcptr->mVolume = 0;
        }

		/*
            TICK 0 EFFECTS
        */
		switch (current->mEffect) 
		{
			case FMUSIC_S3M_SETSPEED : 
			{
                if (current->mEffectParam)
                {
    				mSpeed = current->mEffectParam;
                }
				break;
			}
			case FMUSIC_S3M_PATTERNJUMP : /* --- 00 B00 : --- 00 D63 , should put us at ord=0, row=63 */
			{
				mNextOrder = current->mEffectParam;
				mNextRow = 0;
				
				if (mNextOrder >= mNumOrders) 
                {
					mNextOrder = 0;
                }
				
				jumpflag = 1;
				break;
			}
			case FMUSIC_S3M_PATTERNBREAK : 
			{
				mNextRow = (paramx*10) + paramy;
				
				if (mNextRow > 63) 
                {
					mNextRow = 0;
                }
				if (!breakflag && !jumpflag) 
                {
					mNextOrder = mOrder + 1;
                }
				if (mNextOrder >= mNumOrders) 
                {
					mNextOrder=0;
                }
				break;
			}
			case FMUSIC_S3M_VOLUMESLIDE : 
			{
				/*
                    Dxy - Volume slide, fine vol  DFx = slide down, DxF = slide up
                */
				if (current->mEffectParam) 
                {
					cptr->mVolumeSlide = current->mEffectParam;
                }

				/*
                    DFF is classed as a slide up so it gets priority
                */
				if ((cptr->mVolumeSlide &0xF) == 0xF) 
                {
					vcptr->mVolume += (cptr->mVolumeSlide >> 4);
                }
				else if ((cptr->mVolumeSlide >>4 ) == 0xF) 
                {
					vcptr->mVolume -= (cptr->mVolumeSlide & 0xF);
                }

				/*
                    Perform an extra slide if using old fast vol slides!
                */
				if (mMusicFlags == FMUSIC_FLAGS_FASTVOLUMESLIDES) 
				{
				    if (!(cptr->mVolumeSlide & 0xF)) 
                    {
					    vcptr->mVolume +=(cptr->mVolumeSlide >> 4);
                    }
				    if (!(cptr->mVolumeSlide >> 4))  
                    {
                        vcptr->mVolume -=(cptr->mVolumeSlide & 0xF);
                    }
				}
				if (vcptr->mVolume > 64) 
                {
					vcptr->mVolume = 64;
                }
				if (vcptr->mVolume < 0)  
                {
					vcptr->mVolume = 0;
                }
				break;				
			}
			case FMUSIC_S3M_PORTADOWN :
			{
				if (current->mEffectParam) 
                {
					cptr->mPortaUpDown = current->mEffectParam;
                }

				if ((cptr->mPortaUpDown >> 4)==0xF) 
                {
					vcptr->mFrequency += ((cptr->mPortaUpDown & 0xF) << 2);
                }
				if ((cptr->mPortaUpDown >> 4)==0xE) 
                {
					vcptr->mFrequency += (cptr->mPortaUpDown & 0xF);
                }
				break;
			}

			case FMUSIC_S3M_PORTAUP :
			{
				if (current->mEffectParam) 
                {
					cptr->mPortaUpDown = current->mEffectParam;
                }

				if ((cptr->mPortaUpDown >>4)==0xF) 
                {
					vcptr->mFrequency -= ((cptr->mPortaUpDown & 0xF) << 2);
                }
				if ((cptr->mPortaUpDown >>4)==0xE) 
                {
					vcptr->mFrequency -= (cptr->mPortaUpDown & 0xF);
                }
				break;
			}
			case FMUSIC_S3M_PORTATO :
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
			case FMUSIC_S3M_VIBRATO :
			{
				if (paramx) 
                {
					cptr->mVibSpeed = paramx;
                }
				if (paramy) 
                {
					cptr->mVibDepth = paramy;
                }
				break;
			}
			case FMUSIC_S3M_TREMOR :
			{
				if (current->mEffectParam) 
				{
					cptr->mTremorOn = (paramx+1);
					cptr->mTremorOff = (paramy+1);
				}
				if (cptr->mTremorPosition >= cptr->mTremorOn) 
                {
					vcptr->mVolumeDelta = -vcptr->mVolume;
                }
				cptr->mTremorPosition++;
				if (cptr->mTremorPosition >= (cptr->mTremorOn + cptr->mTremorOff)) 
                {
					cptr->mTremorPosition = 0;
                }
				vcptr->mNoteControl |= FMUSIC_VOLUME;
				break;
			}
			case FMUSIC_S3M_PORTATOVOLSLIDE :
			{
				if (current->mEffectParam) 
                {
					cptr->mVolumeSlide = current->mEffectParam;
                }
				cptr->mPortaTarget = cptr->mPeriod;
				vcptr->mNoteControl &= ~FMUSIC_TRIGGER;
				vcptr->mNoteControl &= ~FMUSIC_FREQ;
				break;
			}
			case FMUSIC_S3M_VIBRATOVOLSLIDE :
			{
				if (current->mEffectParam) 
                {
					cptr->mVolumeSlide = current->mEffectParam;
                }
				break;
			}
			case FMUSIC_S3M_ARPEGGIO :
			{
				if (current->mEffectParam) 
                {
					cptr->mArpeggio = current->mEffectParam;
                }
				break;
			}
			case FMUSIC_S3M_SETSAMPLEOFFSET : 
			{
				unsigned int offset;

				offset = (int)(current->mEffectParam) << 8;

				if (offset >= sptr->mLoopStart + sptr->mLoopLength) 
				{
					vcptr->mNoteControl &= ~FMUSIC_TRIGGER;
					vcptr->mNoteControl |= FMUSIC_STOP;
				}
				else 
                {
					vcptr->mSampleOffset = offset;
                }
				break;
			}
			case FMUSIC_S3M_RETRIGVOLSLIDE :
			{
				if (current->mEffectParam) 
				{
					cptr->mRetrigX = paramx;
					cptr->mRetrigY = paramy;
				}
				break;
			}
			case FMUSIC_S3M_TREMOLO : 
			{
				if (paramx) 
                {
					cptr->mTremoloDepth = paramx;
                }
				if (paramy) 
                {
					cptr->mTremoloDepth = paramy;
                }
				break;
			}

			case FMUSIC_S3M_SPECIAL :
			{
				switch (paramx)
				{
					case FMUSIC_S3M_SETFILTER : 
					case FMUSIC_S3M_NOTECUT :
					case FMUSIC_S3M_FUNKREPEAT :
					{
						break;
					}
					case FMUSIC_S3M_SETGLISSANDO :
					{
						break;
					}
					case FMUSIC_S3M_SETFINETUNE :
					{
						fineTune2Hz(paramy, &sptr->mMiddleC);
						break;
					}
					case FMUSIC_S3M_SETVIBRATOWAVE : 
					{
						cptr->mWaveControl &= 0xF0;
						cptr->mWaveControl |= paramy;
						break;
					}
					case FMUSIC_S3M_SETTREMOLOWAVE : 
					{
						cptr->mWaveControl &= 0xF;
						cptr->mWaveControl |= (paramy<<4);
						break;
					}
					case FMUSIC_S3M_SETPANPOSITION16 :
					{
						vcptr->mPan = paramy<<4;
						vcptr->mNoteControl |= FMUSIC_PAN;
						break;
					}
					case FMUSIC_S3M_STEREOCONTROL :
					{
						if (paramy > 7) 
                        {
							paramy-=8;
                        }
						else
                        {
							paramy+=8;
                        }

						vcptr->mPan = paramy<<4;
						vcptr->mNoteControl |= FMUSIC_PAN;
						break;
					}
					case FMUSIC_S3M_PATTERNLOOP :
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
					case FMUSIC_S3M_NOTEDELAY : 
					{
						vcptr->mVolume = oldvolume;
						vcptr->mFrequency   = oldfreq;
						vcptr->mNoteControl &= ~FMUSIC_FREQ;
						vcptr->mNoteControl &= ~FMUSIC_PAN;
						vcptr->mNoteControl &= ~FMUSIC_VOLUME;
						vcptr->mNoteControl &= ~FMUSIC_TRIGGER;
						break;
					}
					case FMUSIC_S3M_PATTERNDELAY : 
					{
						mPatternDelay = paramy;
						mPatternDelay *= mSpeed;
						break;
					}
				};
				break;
			}
			case FMUSIC_S3M_FINEVIBRATO :
			{
				if (paramx) 
                {
					cptr->mVibSpeed = paramx;
                }
				if (paramy) 
                {
					cptr->mVibDepth = paramy;
                }
				break;
			}
			case FMUSIC_S3M_GLOBALVOLUME : 
			{
				mGlobalVolume = current->mEffectParam;
				if (mGlobalVolume > 64) 
                {
					mGlobalVolume = 64;
                }
				break;
			}
			case FMUSIC_S3M_SETTEMPO : 
			{
				if (current->mEffectParam >= 0x20) 
                {
					setBPM(current->mEffectParam);
                }
				break;
			}
			case FMUSIC_S3M_SETPAN : 
			{
				vcptr->mPan = current->mEffectParam*2;
				vcptr->mNoteControl |= FMUSIC_PAN;
				break;
			}
			case FMUSIC_S3M_Z :
			{
//				FMUSIC_CheckCallback(mod, FMUSIC_CALLBACK_ZXX, current->mEffectParam);
				break;
			}

		};

        if (audible)
        {
            vcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext();
			
			if (!(vcptr->mFrequency+vcptr->mFrequencyDelta)) 
            {
				vcptr->mNoteControl &= ~FMUSIC_FREQ;
            }

			if (vcptr->mNoteControl & FMUSIC_TRIGGER)		
			{
                playSound(sptr, vcptr, false);
			}
			
			/*
                Volume = 6bit, global = 6bit.. halve volume for clipping 1 bit
            */
			if (vcptr->mNoteControl & FMUSIC_VOLUME)
            {
                vcptr->mChannel.setVolume((float)((vcptr->mVolume + vcptr->mVolumeDelta) * mGlobalVolume) / (64.0f * 64.0f) * 0.5f);
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
// VERIFY: 8e41b498 manual
// OFFSET: 0x00908320
FMOD_RESULT CodecS3M::updateEffects() 
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
		MusicChannelS3M         *cptr  = 0;
		MusicVirtualChannel     *vcptr = 0;
		MusicSample             *sptr  = 0;
		unsigned char			effect, paramx, paramy;

		cptr = (MusicChannelS3M *)mMusicChannel[count];
		
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

		switch (effect) 
		{
			case FMUSIC_S3M_VOLUMESLIDE :
			{
				cptr->volumeSlide();
				break;
			}
			case FMUSIC_S3M_PORTADOWN :
			{
				if (cptr->mPortaUpDown <0xE0) 
                {
					vcptr->mFrequency += (cptr->mPortaUpDown  << 2);
                }
				vcptr->mNoteControl |= FMUSIC_FREQ;
				break;
			}
			case FMUSIC_S3M_PORTAUP :
			{
				if (cptr->mPortaUpDown <0xE0) 
				{
					vcptr->mFrequency -= (cptr->mPortaUpDown << 2);
					if (vcptr->mFrequency < 1) 
                    {
						vcptr->mNoteControl |= FMUSIC_STOP;
                    }
					else 
                    {
						vcptr->mNoteControl |= FMUSIC_FREQ;
                    }
				}
				break;
			}
			case FMUSIC_S3M_PORTATO :
			{
				cptr->portamento();
				break;
			}
			case FMUSIC_S3M_VIBRATO :
			{
				cptr->vibrato();
				break;
			}
			case FMUSIC_S3M_TREMOR :
			{
				if (cptr->mTremorPosition >= cptr->mTremorOn) 
                {
					vcptr->mVolumeDelta = -vcptr->mVolume;
                }

				cptr->mTremorPosition++;
				if (cptr->mTremorPosition >= cptr->mTremorOn + cptr->mTremorOff) 
                {
					cptr->mTremorPosition = 0;
                }

				vcptr->mNoteControl |= FMUSIC_VOLUME;
				break;
			}
			case FMUSIC_S3M_ARPEGGIO : 
			{
				if (cptr->mArpeggio > 0)
				{
					paramx = cptr->mArpeggio >> 4;
					paramy = cptr->mArpeggio & 0xF;

					switch (mTick % 3) 
					{
						case 1: 
                        {
                            if (sptr->mMiddleC)
                            {                               
							    vcptr->mFrequencyDelta = (8363L * gPeriodTable[cptr->mNote+paramx] / sptr->mMiddleC) - (8363L * gPeriodTable[cptr->mNote] / sptr->mMiddleC);
                            }
                            else
                            {
							    vcptr->mFrequencyDelta = gPeriodTable[cptr->mNote+paramx] - gPeriodTable[cptr->mNote];
                            }
							break;
                        }
						case 2: 
                        {
                            if (sptr->mMiddleC)
                            {
							    vcptr->mFrequencyDelta = (8363L * gPeriodTable[cptr->mNote+paramy] / sptr->mMiddleC) - (8363L * gPeriodTable[cptr->mNote] / sptr->mMiddleC);
                            }
                            else
                            {
							    vcptr->mFrequencyDelta = gPeriodTable[cptr->mNote+paramy] - gPeriodTable[cptr->mNote];
                            }
							break;
                        }
					};

					vcptr->mNoteControl |= FMUSIC_FREQ;
				}
				break;
			}
			case FMUSIC_S3M_VIBRATOVOLSLIDE :
			{
				cptr->vibrato();
				cptr->volumeSlide();
				break;
			}
			case FMUSIC_S3M_PORTATOVOLSLIDE :
			{
				cptr->portamento();
				cptr->volumeSlide();
				break;
			}
			case FMUSIC_S3M_RETRIGVOLSLIDE : 
			{
				if (!cptr->mRetrigY) 
                {
					break; /* divide by 0 bugfix */
                }
				
				if (!(mTick % cptr->mRetrigY)) 
				{
					if (cptr->mRetrigX) 
					{
						switch (cptr->mRetrigX) 
						{
							case 1: vcptr->mVolume--;
									break;
							case 2: vcptr->mVolume -= 2;
									break;
							case 3: vcptr->mVolume -= 4;
									break;
							case 4: vcptr->mVolume -= 8;
									break;
							case 5: vcptr->mVolume -= 16;
									break;
							case 6: vcptr->mVolume *= 2/3;
									break;
							case 7: vcptr->mVolume >>= 1;
									break;
							case 8: /* ? */
									break;
							case 9: vcptr->mVolume++;
									break;
							case 0xA: vcptr->mVolume += 2;
									break;
							case 0xB: vcptr->mVolume += 4;
									break;
							case 0xC: vcptr->mVolume += 8;
									break;
							case 0xD: vcptr->mVolume += 16;
									break;
							case 0xE: vcptr->mVolume *= 3/2;
									break;
							case 0xF: vcptr->mVolume <<= 1;
									break;
						};
						if (vcptr->mVolume > 64) 
                        {
							vcptr->mVolume = 64;
                        }
						if (vcptr->mVolume < 0)  
                        {
							vcptr->mVolume = 0;
                        }
					}

					vcptr->mPan = mDefaultPan[count];
					vcptr->mFrequency = cptr->mPeriod;
					vcptr->mFrequencyDelta= 0;

					vcptr->mNoteControl |= FMUSIC_VOLUME;
					vcptr->mNoteControl |= FMUSIC_PAN;
					vcptr->mNoteControl |= FMUSIC_FREQ;
					vcptr->mNoteControl |= FMUSIC_TRIGGER;
				}
				break;
			}
			case FMUSIC_S3M_TREMOLO :
			{
				cptr->tremolo();
				break;
			}
			case FMUSIC_S3M_SPECIAL :
			{
				switch (paramx)
				{
					case FMUSIC_S3M_NOTECUT: 
					{
						if (mTick==paramy) 
						{
							vcptr->mVolume = 0;
							vcptr->mNoteControl |= FMUSIC_VOLUME;
						}
						break;
					}
					case FMUSIC_S3M_NOTEDELAY : 
					{
						if (mTick == paramy) 
						{
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

							if (current->mNumber) 
							{
								vcptr->mVolume = sptr->mDefaultVolume;
								/*
                                    Retrigger tremolo and vibrato waveforms
                                */
								if ((cptr->mWaveControl & 0xF) < 4) 
                                {
									cptr->mVibPos = 0;
                                }
								if ((cptr->mWaveControl >> 4) < 4)  
                                {
									cptr->mTremoloPosition = 0;
                                }

								cptr->mTremorPosition	= 0;			/* retrigger tremor count */

								vcptr->mNoteControl |= FMUSIC_VOLUME;
							}

							vcptr->mPan = mDefaultPan[count];
							vcptr->mFrequency = cptr->mPeriod;
							vcptr->mFrequencyDelta= 0;

							vcptr->mNoteControl |= FMUSIC_FREQ;
							vcptr->mNoteControl |= FMUSIC_PAN;
							if (current->mVolume) 
							{
								vcptr->mVolume = current->mVolume - 1;
								vcptr->mNoteControl |= FMUSIC_VOLUME;
							}
							vcptr->mNoteControl |= FMUSIC_TRIGGER;
						}
						else
						{
							vcptr->mNoteControl &= ~FMUSIC_VOLUME;
							vcptr->mNoteControl &= ~FMUSIC_FREQ;
							vcptr->mNoteControl &= ~FMUSIC_PAN;
							vcptr->mNoteControl &= ~FMUSIC_TRIGGER;
						}
						break;
					}
				};
				break;
			}
			case FMUSIC_S3M_FINEVIBRATO :
			{
				cptr->fineVibrato();
				break;
			}

		};

        {
            vcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext();

			if (!(vcptr->mFrequency+vcptr->mFrequencyDelta)) 
            {
				vcptr->mNoteControl &= ~FMUSIC_FREQ;
            }

			if (vcptr->mNoteControl & FMUSIC_TRIGGER)		
			{
                playSound(sptr, vcptr, false);
			}

			/*
                volume = 6bit, global = 6bit.. halve volume for clipping 1 bit
            */
			if (vcptr->mNoteControl & FMUSIC_VOLUME)
            {
                vcptr->mChannel.setVolume((float)((vcptr->mVolume + vcptr->mVolumeDelta) * mGlobalVolume) / (64.0f * 64.0f) * 0.5f);
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
// VERIFY: 358573ae manual
// OFFSET: 0x00908AF0
FMOD_RESULT CodecS3M::update(bool audible)
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

	mTick++;
	if (mTick >= mSpeed + mPatternDelay)
	{
		mPatternDelay = 0;
		mTick = 0;
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 710b822c manual
// OFFSET: 0x00909330
FMOD_RESULT CodecS3M::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
	unsigned char	remap[32], st3pan, mastervol, temp[4];
	unsigned short	temp16, songlength, filenumpatterns;
	unsigned short  parapoint[355];
	unsigned int    samplepoint[99], lengthbytes;
	int 			count;
    FMOD_RESULT     result = FMOD_OK;

    if (!mFile->mSeekable)
    {
        return FMOD_ERR_FORMAT;
    }

    init(FMOD_SOUND_TYPE_S3M);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecS3M::openInternal", "attempting to open as S3M..\n"));

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

	/*
        Verify Format
    */
	result = mFile->seek(0x2C, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->read(&temp, 1, 4, 0);
    if (result != FMOD_OK)
    {
        return result;
    }
	if (FMOD_strncmp((char *)temp, "SCRM", 4))
	{
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecS3M::openInternal", "'SCRM' ID check failed [%c%c%c%c]\n", temp[0], temp[1], temp[2], temp[3]));
		return FMOD_ERR_FORMAT;
	}

	/*
        Set a few default values for this format
    */
    for (count = 0; count < MUSIC_MAXCHANNELS; count++)
    {
        mMusicChannel[count] = 0;
    }
	mPattern                = 0;
	mPanSeparation          = 0.8f;
	mMasterSpeed            = 1.0f;
	mDefaultSpeed			= 6;
	mDefaultBPM				= 125;
	mDefaultGlobalVolume    = 64;
	mNumPatterns			= 0;
	mRestart				= 0;

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->read(mSongName, 1, 28, 0);		    /* read in module name. */
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->getByte();
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->seek(0x20, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->getWord(&songlength);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->getWord(&mNumSamples);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->getWord(&filenumpatterns);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->getWord(&temp16);
    if (result != FMOD_OK)
    {
        return result;
    }

	if (temp16 & 64) 
    {
        mMusicFlags = FMUSIC_FLAGS_FASTVOLUMESLIDES;
    }
	else
    {
        mMusicFlags = FMUSIC_FLAGS_NORMALVOLUMESLIDES;
    }

    result = mFile->getWord(&temp16);
    if (result != FMOD_OK)
    {
        return result;
    }
	if ((temp16 & 0xFFF) == 300) 
    {
        mMusicFlags = FMUSIC_FLAGS_FASTVOLUMESLIDES;     /* fast volume slides? */
    }

	/*
        Seek to offset 02Ch for SCRM tag
    */
	result = mFile->seek(0x2C, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->read(&temp, 1, 4, 0);
    if (result != FMOD_OK)
    {
        return result;
    }
	if (FMOD_strncmp((char *)temp, "SCRM", 4))
	{
		return FMOD_ERR_FORMAT;
	}

    result = mFile->getByte(&mGlobalVolume);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->getByte(&mDefaultSpeed);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->getByte(&mDefaultBPM);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->getByte(&mastervol);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->getByte();
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->getByte(&st3pan);
    if (result != FMOD_OK)
    {
        return result;
    }

	/*
        Find the number of channels and remap the used channels linearly
    */
	result = mFile->seek(0x40, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

	mNumChannels = 0;
	FMOD_memset(remap, 255, 32);

	for (count=0; count<32; count++) 
	{
		unsigned char temp;

        result = mFile->getByte(&temp);
        if (result != FMOD_OK)
        {
            return result;
        }
		if (temp < 16) 
		{
			remap[count] = (unsigned char)mNumChannels;
			if (temp <= 7) 
            {
				mDefaultPan[mNumChannels] = 0;
            }
			else
            {
				mDefaultPan[mNumChannels] = 255;
            }

			mNumChannels++;
		}
	}

    result = metaData(FMOD_TAGTYPE_FMOD, "Number of channels", &mNumChannels, sizeof(mNumChannels), FMOD_TAGDATATYPE_INT, false);
    if (result != FMOD_OK)
    {
        return result;
    }

	/*
        ALLOCATE MUSIC CHANNELS
    */
    for (count = 0; count < mNumChannels; count++)
    {
        mMusicChannel[count] = FMOD_Object_Calloc(MusicChannelS3M);
        if (!mMusicChannel[count])
        {
            return FMOD_ERR_MEMORY;
        }
    }

	/*
        Load order data & calculate number of physical patterns
    */
	result = mFile->seek(0x60, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

	result = mFile->read(mOrderList, 1, songlength, 0);
    if (result != FMOD_OK)
    {
        return result;
    }
	mNumOrders = 0;
	mNumPatterns = 0;
	for (count=0; count<songlength; count++) 
	{
		mOrderList[mNumOrders] = mOrderList[count];
		if (mOrderList[count]<254) 
		{
			mNumOrders++;
			if (mOrderList[count] > mNumPatterns)
            { 
				mNumPatterns = mOrderList[count];
            }
		}
	}
	mNumPatterns++;

	/*
        Load in instrument and pattern parapointers
    */
    result = mFile->read(parapoint, 2, mNumSamples + filenumpatterns, 0);
	if (result != FMOD_OK)
	{
		return result;
	}

	/*
        Check for default panning if the panning flag is set (252 = 0xFC :)
    */
	if (st3pan == 252) 
	{
		for (count=0; count<32; count++) 
		{
			unsigned char temp;
			
            result = mFile->getByte(&temp);
            if (result != FMOD_OK)
            {
                return result;
            }
			if (temp & 0x10) 
            {
				mDefaultPan[remap[count]] = ((temp & 0xF)<<4);
            }
		}
	}

	/*
        If stereo flag is not set then make song mono
    */
	if (!(mastervol & 128)) 
    {
		for (count=0; count<32; count++) 
        {
			mDefaultPan[count]= 128;
        }
    }

	/*
        Load instrument information
    */
	for (count = 0; count< mNumSamples; count++) 
	{
        char                sample_name[28];
        FMOD_MODE           sample_mode;
        unsigned int        sample_length;
        FMOD_SOUND_FORMAT   sample_format;
		unsigned char       instflag;
		unsigned char       temp;
        int                 channels = 1;

        FMOD_memset(&mSample[count], 0, sizeof(MusicSample));

		/*
            Jump to instrument parapointer.
        */
		result = mFile->seek((unsigned int)((parapoint[count]) << 4), SEEK_SET);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->seek(13, SEEK_CUR);             /* skip filename */
        if (result != FMOD_OK)
        {
            return result;
        }

		/*
            Find parapointer to actual sample data (3 bytes)
        */
        result = mFile->getByte(&temp);
        if (result != FMOD_OK)
        {
            return result;
        }
        result = mFile->getWord(&temp16);
        if (result != FMOD_OK)
        {
            return result;
        }
		samplepoint[count] = ((unsigned int )temp << 16) + temp16;

		/*
            Get rest of information
        */
		result = mFile->read(&sample_length, 4, 1, 0);		/* get length               */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(&mSample[count].mLoopStart, 4, 1, 0);	/* loop start               */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(&mSample[count].mLoopLength, 4, 1, 0);		/* loop end                 */
        if (result != FMOD_OK)
        {
            return result;
        }
		mSample[count].mLoopLength -= mSample[count].mLoopStart;

        result = mFile->getByte(&mSample[count].mDefaultVolume);
        if (result != FMOD_OK)
        {
            return result;
        }
        result = mFile->getWord();
        if (result != FMOD_OK)
        {
            return result;
        }
        result = mFile->getByte(&instflag);
        if (result != FMOD_OK)
        {
            return result;
        }
        result = mFile->getWord(&mSample[count].mMiddleC);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->seek(14, SEEK_CUR);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(sample_name, 28, 1, 0);		/* Inst name                */
        if (result != FMOD_OK)
        {
            return result;
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

		result = mFile->seek(4, SEEK_CUR);				/* 'SCRS'                   */
        if (result != FMOD_OK)
        {
            return result;
        }

		/*
            Any samples with a loop length of 2 or less have no loop.
        */
		sample_mode = FMOD_SOFTWARE | FMOD_2D;

		if (instflag & 1 && mSample[count].mLoopLength > 2) 
        {
            sample_mode |= FMOD_LOOP_NORMAL;
        }
		else 
		{
            sample_mode |= FMOD_LOOP_OFF;
			mSample[count].mLoopStart = 0;
			mSample[count].mLoopLength = sample_length;
		}

        sample_format = FMOD_SOUND_FORMAT_PCM8;
		if (instflag & 4) 
        {
            sample_format = FMOD_SOUND_FORMAT_PCM16;
            sample_length <<= 1;
        }
        if (instflag & 2)
        {
            channels = 2;
            sample_length <<= 1;
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
            exinfo.numchannels      = channels;
            exinfo.defaultfrequency = mSample[count].mMiddleC;
            exinfo.format           = sample_format;

            result = mSystem->createSound(0, sample_mode | FMOD_OPENUSER, &exinfo, &mSample[count].mSound);
            if (result != FMOD_OK)
            {
                return result;
            }

            if (sample_mode & FMOD_LOOP_NORMAL)
            {
                result = mSample[count].mSound->setLoopPoints(mSample[count].mLoopStart, FMOD_TIMEUNIT_PCM, mSample[count].mLoopStart + mSample[count].mLoopLength - 1, FMOD_TIMEUNIT_PCM);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }
		}

	}

	/*
        Alloc pattern array
    */
	mNumPatternsMem = (mNumPatterns > filenumpatterns ? mNumPatterns : filenumpatterns);
	mPattern = (MusicPattern *)FMOD_Memory_Calloc(mNumPatternsMem * sizeof(MusicPattern));
	if (!mPattern) 
	{
		return FMOD_ERR_MEMORY;
	}


	/*
        Load the pattern data
    */
	for (count=0; count < filenumpatterns; count++) 
	{
		MusicPattern *pptr;
		MusicNote    *nptr;
		int           row,len;

		/*
            Jump to pattern parapointer +2, skipping length bytes.
        */
		result = mFile->seek((((unsigned int )parapoint[mNumSamples+count])<<4) + 2, SEEK_SET);
        if (result != FMOD_OK)
        {
            return result;
        }

		/*
            Allocate memory for the pattern header
        */
		pptr = &mPattern[count];
		pptr->mRows = 64;

		/*
            Allocate memory for pattern buffer
        */
		len = mNumChannels * pptr->mRows * sizeof(MusicNote);
		pptr->mData = (MusicNote *)FMOD_Memory_Calloc(len);
		if (!pptr->mData)
		{
			return FMOD_ERR_MEMORY;
		}

		
		/*
            Unpack one pattern (pattern number 'count')
        */
		for (row=0; row<64;)
		{
			unsigned char	temp, channel;
			MusicNote		dummy;

            result = mFile->getByte(&temp);
            if (result != FMOD_OK)
            {
                return result;
            }

			/*
                If it not 0, unpack note, if it is 0 then end of row.
            */
			if(temp) 
			{
				channel=remap[temp & 31];

				/*
                    If it has told us to use a channel outside of the number of channels, load it into a dummy
                */
				if(channel < mNumChannels) 
                {
                    nptr = pptr->mData + (mNumChannels * row) + channel;
                }
				else
                {
                    nptr = &dummy; 
                }

				/*
                    If there is a note
                */
				if(temp & 32) 
				{
                    result = mFile->getByte(&temp16);
                    if (result != FMOD_OK)
                    {
                        return result;
                    }

					/*
                        Convert s3m note to internal type note
                    */

					switch(temp16) 
					{
						case 255 : nptr->mNote = 0;
								   break;
						case 254 : nptr->mNote = FMUSIC_KEYOFF;
								   break;
						 default : nptr->mNote = ((temp16>>4)*12)+(temp16&0xf) + 1;	/* starts at 1 */
					};
                    result = mFile->getByte(&nptr->mNumber);
                    if (result != FMOD_OK)
                    {
                        return result;
                    }
				}

				/*
                    If there is a volume byte
                */
				if (temp & 64) 
                {
                    result = mFile->getByte(&nptr->mVolume);
                    if (result != FMOD_OK)
                    {
                        return result;
                    }
                    nptr->mVolume++;
                }

				/*
                    If there is an effect
                */
				if(temp & 128) 
				{
                    result = mFile->getByte(&nptr->mEffect);
                    if (result != FMOD_OK)
                    {
                        return result;
                    }
                    result = mFile->getByte(&nptr->mEffectParam);
                    if (result != FMOD_OK)
                    {
                        return result;
                    }
				}
			}
			else row++;
		}
	}


	/*
        Allocate and clean out any extra patterns 
    */
	if (mNumPatterns > filenumpatterns)
	{
		for (count = filenumpatterns; count < mNumPatterns; count++) 
		{			
			MusicPattern *pptr;

			pptr = &mPattern[count];
			pptr->mRows = 64;

			/*
                Allocate memory for pattern buffer
            */
			pptr->mData = (MusicNote *)FMOD_Memory_Calloc(mNumChannels * pptr->mRows * sizeof(MusicNote));
			if (!pptr->mData)
			{
				return FMOD_ERR_MEMORY;
			}
		}
	}

	/*
        Load sample data
    */
	for (count = 0; count < mNumSamples; count++) 
	{
        MusicSample *sptr;

		result = mFile->seek(samplepoint[count] << 4, SEEK_SET);
        if (result != FMOD_OK)
        {
            return result;
        }

		sptr = &mSample[count];
    	if (sptr->mSound)
		{
            void *ptr1, *ptr2;
            unsigned int len1, len2, lenbytes;
    
            result = sptr->mSound->getLength(&lenbytes, FMOD_TIMEUNIT_PCMBYTES);
            if (result != FMOD_OK)
            {
                return result;
            }

            result = sptr->mSound->lock(0, lenbytes, &ptr1, &ptr2, &len1, &len2);
            if (result != FMOD_OK)
            {
                return result;
            }

            if (ptr1 && len1)
            {
                unsigned int count2;

                if (sptr->mSound->mFormat == FMOD_SOUND_FORMAT_PCM16)
                {
                    if (sptr->mSound->mChannels == 1)
                    {
				        result = mFile->read(ptr1, 2, len1 >> 1);
                    }
                    else
                    {
                        unsigned int count2;
                        
                        /*
                            S3M stereo samples are not interleaved!
                        */
                        unsigned int off, len;

                        /* LEFT */
                        off = 0;
                        len = len1 >> 2;
                        while (len)
                        {
                            unsigned int r;
                            signed short tmp[512];

                            r = len > 512 ? 512 : len;
    				        
                            result = mFile->read(tmp, 2, r);

                            for (count2 = 0; count2 < r; count2++)  
                            {
    				            ((signed short *)ptr1)[(off + count2) * 2] = tmp[count2];
                            }

                            len -= r;
                            off += r;
                        }

                        /* RIGHT */
                        off = 0;
                        len = len1 >> 2;
                        while (len)
                        {
                            unsigned int r;
                            signed short tmp[512];

                            r = len > 512 ? 512 : len;
    				        
                            result = mFile->read(tmp, 2, r);

                            for (count2 = 0; count2 < r; count2++)  
                            {
    				            ((signed short *)ptr1)[((off + count2) * 2) + 1] = tmp[count2];
                            }

                            len -= r;
                            off += r;
                        }
                    }

                    for (count2 = 0; count2 < len1 / 2; count2++)
                    {
                        ((unsigned short *)ptr1)[count2] ^= 32768;
                    }
                }
                else
                {
                    if (sptr->mSound->mChannels == 1)
                    {
    				    result = mFile->read(ptr1, 1, len1);
                        if (result != FMOD_OK)
                        {
                            return result;
                        }
                    }
                    else
                    {
                        unsigned int count2;

                        /*
                            S3M stereo samples are not interleaved!
                        */
                        unsigned int off, len;

                        /* LEFT */
                        off = 0;
                        len = len1 >> 2;
                        while (len)
                        {
                            unsigned int r;
                            signed char tmp[512];

                            r = len > 512 ? 512 : len;
    				        
                            result = mFile->read(tmp, 1, r);

                            for (count2 = 0; count2 < r; count2++)  
                            {
    				            ((signed char *)ptr1)[(off + count2) * 2] = tmp[count2];
                            }

                            len -= r;
                            off += r;
                        }

                        /* RIGHT */
                        off = 0;
                        len = len1 >> 2;
                        while (len)
                        {
                            unsigned int r;
                            signed char tmp[512];

                            r = len > 512 ? 512 : len;
    				        
                            result = mFile->read(tmp, 1, r);

                            for (count2 = 0; count2 < r; count2++)  
                            {
    				            ((signed char *)ptr1)[((off + count2) * 2) + 1] = tmp[count2];
                            }

                            len -= r;
                            off += r;
                        }
                    }
                       
                    for (count2 = 0; count2 < len1; count2++)
                    {
                        ((unsigned char *)ptr1)[count2] ^= 128;
                    }
                }
                if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
                {
                    return result;
                }
            }
		
            result = sptr->mSound->unlock(ptr1, ptr2, len1, len2);
            if (result != FMOD_OK)
            {
                return result;
            }
		}
	}

    mWaveFormatMemory = (FMOD_CODEC_WAVEFORMAT *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_WAVEFORMAT));
    if (!mWaveFormatMemory)
    {
        return FMOD_ERR_MEMORY;
    }

    waveformat = mWaveFormatMemory;
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
        FMOD_strcpy(descriptionex.name, "FMOD S3M Target Unit");
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: f0645717 auto
// OFFSET: 0x00908C10
FMOD_RESULT CodecS3M::closeInternal()
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 6747d9f4 manual
// OFFSET: 0x00908F40
FMOD_RESULT CodecS3M::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 0967af0e auto
// OFFSET: 0x00908DF0
FMOD_RESULT CodecS3M::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 621ceec4 manual
// OFFSET: 0x0090A560
FMOD_RESULT F_CALLBACK CodecS3M::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecS3M *cs3m = (CodecS3M *)codec;

    return cs3m->openInternal(usermode, userexinfo);
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
// VERIFY: a9a13aba manual
// OFFSET: 0x00908E90
FMOD_RESULT F_CALLBACK CodecS3M::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecS3M *cs3m = (CodecS3M *)codec;

    return cs3m->closeInternal();
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
// VERIFY: 09301395 auto
// OFFSET: 0x00909300
FMOD_RESULT F_CALLBACK CodecS3M::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecS3M *cs3m = (CodecS3M *)codec;

    return cs3m->readInternal(buffer, sizebytes, bytesread);
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
// VERIFY: 7f543ae1 auto
// OFFSET: 0x00908EC0
FMOD_RESULT F_CALLBACK CodecS3M::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype   )
{
    CodecS3M *cs3m = (CodecS3M *)codec;

    return cs3m->setPositionInternal(subsound, position, postype);
}

}

#endif


