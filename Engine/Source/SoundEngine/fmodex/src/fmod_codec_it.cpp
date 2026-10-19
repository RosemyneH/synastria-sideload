#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_IT

#include "fmod.h"

#include "fmod_channel_software.h"
#include "fmod_codec_it.h"
#include "fmod_debug.h"
#include "fmod_dspi.h"
#include "fmod_dsp_itecho.h"
#include "fmod_dsp_lowpass2.h"
#include "fmod_file.h"
#include "fmod_localcriticalsection.h"
#include "fmod_memory.h"
#include "fmod_systemi.h"
#include "fmod_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

namespace FMOD
{


FMOD_CODEC_DESCRIPTION_EX itcodec;


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
        return CodecIT::getDescriptionEx();
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
// VERIFY: 36959171 manual
// OFFSET: 0x00903A10
FMOD_CODEC_DESCRIPTION_EX *CodecIT::getDescriptionEx()
{
    FMOD_memset(&itcodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    itcodec.name            = "FMOD IT Codec";
    itcodec.version         = 0x00010100;
    itcodec.timeunits       = (FMOD_TIMEUNIT)(FMOD_TIMEUNIT_PCM | FMOD_TIMEUNIT_MODORDER | FMOD_TIMEUNIT_MODROW | FMOD_TIMEUNIT_MODPATTERN);
    itcodec.defaultasstream = 1;
    itcodec.open            = &CodecIT::openCallback;
    itcodec.close           = &CodecIT::closeCallback;
    itcodec.read            = &CodecIT::readCallback;
    itcodec.getlength       = &MusicSong::getLengthCallback;
    itcodec.setposition     = &CodecIT::setPositionCallback;
    itcodec.getposition     = &MusicSong::getPositionCallback;

    itcodec.mType = FMOD_SOUND_TYPE_IT;
    itcodec.mSize = sizeof(CodecIT);

    return &itcodec;
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
// VERIFY: 2eddfbf1 auto
// OFFSET: 0x00900F50
FMOD_RESULT CodecIT::calculateLength()
{
    waveformat[0].lengthpcm = 0;

    play();

    while (!mFinished)
    {
        update(false);

        waveformat[0].lengthpcm += mMixerSamplesPerTick;
    }

    stop();

    return FMOD_OK;
}


#define FMUSIC_ITLINEARPERIOD2HZ(_per) ( (int)(8363.0f*FMOD_POW(2.0f, (float)((5.0f*12.0f*64.0f - (float)_per) / (12.0*64.0)) )) )


#define FMUSIC_PLAYPACKED

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 866e5e99 auto
// OFFSET: 0x008FD3C0
FMOD_RESULT CodecIT::readBits(unsigned char b, unsigned int *result)	/* reads b bits from the stream */
{
	unsigned int value,value2;

	if (b <= mSrcRemBits)
	{
		value2 = value = *mSrcPos;
		
		#ifdef PLATFORM_ENDIAN_BIG
		value2 = value = FMOD_SWAPENDIAN_DWORD(value);
		#endif

		value &= ((1<<b)-1);
	
        value2 >>= b;

		#ifdef PLATFORM_ENDIAN_BIG
		value2 = FMOD_SWAPENDIAN_DWORD(value2);     /* put it BACK into little endia format for memory storage */
		#endif

		*mSrcPos = value2;
		mSrcRemBits -= b;
	}
	else
	{
		unsigned int nbits = b-mSrcRemBits;

		value  =  *mSrcPos++;

		#ifdef PLATFORM_ENDIAN_BIG
		value = FMOD_SWAPENDIAN_DWORD(value);
		#endif

		value2  =  *mSrcPos;

		#ifdef PLATFORM_ENDIAN_BIG
		value2 = FMOD_SWAPENDIAN_DWORD(value2);
		#endif

		value |= ((value2 & ((1<<nbits)-1)) << mSrcRemBits);
		value2 >>= nbits;
		
		#ifdef PLATFORM_ENDIAN_BIG
		value2 = FMOD_SWAPENDIAN_DWORD(value2);   /* put it BACK into little endian format */
		#endif

		*mSrcPos = value2;
		
		mSrcRemBits= (unsigned char)(32-nbits);
	}

    if (result)
    {
        *result = value;
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
// VERIFY: 817feae3 manual
// OFFSET: 0x008FD470
FMOD_RESULT CodecIT::readBlock(signed char **buff)  /* gets block of compressed data from file */
{
	unsigned short size;

    /*
        Read 1 byte at a time to save misaligned reads.
    */
    size  = ((*buff)[1] << 8);
    size |= ((*buff)[0] & 0xFF);
	(*buff)+=2;

	mSrcBuffer = (unsigned int *)FMOD_Memory_Alloc(size * 2);
	if (!mSrcBuffer) 
    {
		return FMOD_ERR_MEMORY;
    }

	FMOD_memcpy(mSrcBuffer, *buff, size);
	*buff+= size;

	mSrcPos     = mSrcBuffer;
	mSrcRemBits = 32;

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
FMOD_RESULT CodecIT::freeBlock()            /* frees that block again */
{
	if (mSrcBuffer) 
    {
		FMOD_Memory_Free(mSrcBuffer);
        mSrcBuffer = 0;
    }
  
	return FMOD_OK;
}


/*
[
	[DESCRIPTION]
	decompresses 8-bit it214 sample 

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 8ecacc20 manual
// OFFSET: 0x008FD4F0
FMOD_RESULT CodecIT::decompress8(void **src, void *dst, int len, bool it215, int channels)
{ 
	signed char		*destbuf;   /* the destination buffer which will be returned */
	unsigned short	blklen;     /* length of compressed data block in samples */
	unsigned short	blkpos;     /* position in block */
	unsigned char	width;      /* actual "bit width" */
	unsigned int    value;      /* value read from file to be processed */
	signed char		d1, d2;     /* integrator buffers (d2 for it2.15) */
	signed char		*destpos;

	destbuf = (signed char *)dst;
	if (!destbuf) 
    {
		return FMOD_ERR_INVALID_PARAM;
    }
	if (!src || !*src) 
    {
		return FMOD_ERR_INVALID_PARAM;
    }

	destpos=destbuf;		    /* position in output buffer */

	/*
        Now unpack data till the dest buffer is full
    */
	while (len)
	{
        FMOD_RESULT result;

		/*
            Read a new block of compressed data and reset variables
        */
        result = readBlock((signed char **)src);
		if (result != FMOD_OK) 
        {
			return result;
        }
		
		blklen=(len<0x8000)?len:0x8000;
		blkpos=0;

		width=9;  /* start with width of 9 bits */
		d1=d2=0;  /* reset integrator buffers */

		/*
            Now uncompress the data block
        */
		while (blkpos<blklen)
		{
			signed char v; /* sample value */

			readBits(width, &value);  /* read bits */

			if (width<7)                                            /* method 1 (1-6 bits) */
			{
				if (value == (1U << (width - 1))) /* check for "100..." */
				{
                    readBits(3, &value);
					value+=1;  /* yes -> read new width; */
					width = (value<width)?value:value+1;  /* and expand it */
					continue;                             /* ... next value */
				}
			}
			else if (width<9) /* method 2 (7-8 bits) */
			{
				unsigned char border = (0xFF>>(9-width)) - 4;  /* lower border for width chg */

				if (value > border && value <= (border + 8U))
				{
					value-=border;                        /* convert width to 1-8 */
					width = (value<width)?value:value+1;  /* and expand it */
					continue;                             /* ... next value */
				}
			}
			else if (width==9) /* method 3 (9 bits) */
			{
				if (value & 0x100) /* bit 8 set? */
				{
					width=(value+1)&0xff; /* new width... */
					continue;             /* ... and next value */
				}
			}
			else /* illegal width, abort */
			{
				freeBlock();
				return FMOD_ERR_FORMAT;
			}

			/* now expand value to signed byte */
			if (width<8)
			{
				signed char shift=8-width;
				
				v = (value<<shift);
				v>>=shift;
			}
			else 
				v = (signed char)value;

			/* integrate upon the sample values */
			d1+=v;
			d2+=d1;

			/* ... and store it into the buffer */
			*(destpos+=channels)=it215?d2:d1;
			blkpos++;
	    }

		/* 
            Now subtract block length from total length and go on
        */
		freeBlock();
		len-=blklen;
	}

	return FMOD_OK;
}


/*
[
	[DESCRIPTION]
	decompresses 8-bit it214 sample 

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: e018e959 manual
// OFFSET: 0x008FD6F0
FMOD_RESULT CodecIT::decompress16(void **src, void *dst, int len, bool it215, int channels)
{
	short			*destbuf;   /* the destination buffer which will be returned */
	unsigned short	blklen;     /* length of compressed data block in samples */
	unsigned short	blkpos;     /* position in block */
	unsigned char	width;      /* actual "bit width" */
	unsigned int 	value;      /* value read from file to be processed */
	short			d1, d2;		/* integrator buffers (d2 for it2.15) */
	short			*destpos;

	destbuf = (short *)dst;
	if (!destbuf) 
    {
        return FMOD_ERR_INVALID_PARAM;
    }
	if (!src || !*src) 
    {
        return FMOD_ERR_INVALID_PARAM;
    }

	destpos=destbuf;		    /* position in output buffer */

	/*
        Now unpack data till the dest buffer is full
    */
	while (len)
	{
        FMOD_RESULT result;

		/*
            Read a new block of compressed data and reset variables
        */
        result = readBlock((signed char **)src);
		if (result != FMOD_OK) 
        {
			return result;
        }
		blklen=(len<0x4000)?len:0x4000; /* 0x4000 samples => 0x8000 bytes again */
		blkpos=0;

		width=17; /* start with width of 17 bits */
		d1=d2=0;  /* reset integrator buffers */

		/*
            Now uncompress the data block
        */
		while (blkpos<blklen)
		{
			short v; /* sample value */

			readBits(width, &value);							/* read bits */

			if (width<7)												/* method 1 (1-6 bits) */
			{
				if (value == (unsigned int)(1L<<(width-1)))			    /* check for "100..." */
				{
                    readBits(4, &value);
					value += 1;					/* yes -> read new width; */
					width = (unsigned char)((value<width)?value:value+1);	/* and expand it */
					continue;											/* ... next value */
				}
			}
			else if (width<17)											/* method 2 (7-16 bits) */
			{
				unsigned short border = (0xFFFF>>(17-width)) - 8;		/* lower border for width chg */

				if (value > border && value <= (unsigned short)(border+16))
				{
					value-=border;										/* convert width to 1-8 */
					width = (unsigned char)((value<width)?value:value+1);/* and expand it */
					continue;											/* ... next value */
				}
			}
			else if (width==17)											/* method 3 (17 bits) */
			{
				if (value&0x10000)										/* bit 16 set? */
				{
					width = (unsigned char)((value+1)&0xff);			/* new width... */
					continue;											/* ... and next value */
				}
			}
			else														/* illegal width, abort */
			{
				freeBlock();
				return FMOD_ERR_FORMAT;
			}

			/*
                Now expand value to signed word
            */
			if (width<16)
			{
				unsigned char shift=16-width;
				v = (unsigned short)(value<<shift);
				v>>=shift;
			}
			else 
            {
                v = (short)value;
            }

			/*
                Integrate upon the sample values
            */
			d1+=v;
			d2+=d1;

			/*
                ... and store it into the buffer
            */
			*(destpos+=channels)=it215?d2:d1;
			blkpos++;
		}

		/*
            Now subtract block lenght from total length and go on
        */
		freeBlock();
		len-=blklen;

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
// VERIFY: 102e51d3 manual
// OFFSET: 0x008FD900
FMOD_RESULT MusicChannelIT::volumeSlide() 
{
    MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();

	if (!(mVolumeSlide & 0xF)) 
    {
		mVolume += (mVolumeSlide >> 4);
    }
	if (!(mVolumeSlide >>  4)) 
    {
		mVolume -= (mVolumeSlide & 0xF);
    }
	
	if (mVolume > 64) 
    {
		mVolume = 64;
    }
	if (mVolume < 0)  
    {
		mVolume = 0;
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
FMOD_RESULT MusicChannelIT::panSlide() 
{
    MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();

	if (!(mPanSlide & 0xF)) 
    {
		mPan -= (mPanSlide >> 4);
    }
	if (!(mPanSlide >>  4)) 
    {
		mPan += (mPanSlide & 0xF);
    }
	
	if (mPan > 64) 
    {
		mPan = 64;
    }
	if (mPan < 0)  
    {
		mPan = 0;
    }
	
	vcptr->mNoteControl |= FMUSIC_PAN;

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

put Gx volume column on a row and Gxx on a row TOGETHER
You'll notice the actual speed it uses
is Gxx + 1
instead of Gxx + Gx


Gx isn't just a simple equation to find the speed
you have to use a little algorithm
here is what it looks like


void CALLING mpTonePortaCmdITStyle ( usigned char arg )
{
	if ( mpTicks != 0 )
}
}
	mpTonePortamento ( 1 << 6 + ( 0x20 * ( arg - 6 ) ) );
	mpTonePortamento ( 1 << arg ); 
	else if ( arg < 7 )	mpTonePortamento ( arg ); 
	else if ( arg < 2 )

Thats for the volume column
( ( chan->info != 0 ) && ( arg != 0 ) ) ) arg++;
	if ( ( ( chan->cmd == mpcTonePortamento ) || ( chan->cmd == mpcToneVol ) ) &&

Oh IT doesn't like to tone portamento twice so you don't want to call 2 TonePortamento routines
like even if there is a Gx on the volume column and one on the effect. You only call it once
You just add to the speed +1 if there are 2.  There is a flag you have to set like slidAlready = 0;



*/
// VERIFY: caba842b manual
// OFFSET: 0x008FD960
FMOD_RESULT MusicChannelIT::portamento()
{
    MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();
    MusicSong *mod = mModule;

	if (mPortaReached) 
    {
		return FMOD_OK;
    }

	/*
        Slide pitch down if it needs to.
    */
	if (vcptr->mFrequency < mPortaTarget) 
	{
		if (mod->mMusicFlags & FMUSIC_ITFLAGS_EFFECT_G)
        {
			vcptr->mFrequency += (int)mPortaSpeed << 2;
        }
		else 
        {
			vcptr->mFrequency += (int)mPortaUpDown << 2;
        }

		if (vcptr->mFrequency >= mPortaTarget) 
		{
			vcptr->mFrequency = mPortaTarget;
			mPortaReached = true;
		}
	}

	/*
        Slide pitch up if it needs too.
    */
	else if (vcptr->mFrequency > mPortaTarget) 
	{
		if (mod->mMusicFlags & FMUSIC_ITFLAGS_EFFECT_G)
        {
			vcptr->mFrequency -= (int)mPortaSpeed << 2;
        }
		else
        {
			vcptr->mFrequency -= (int)mPortaUpDown << 2;
        }

		if (vcptr->mFrequency < mPortaTarget) 
		{
			vcptr->mFrequency = mPortaTarget;
			mPortaReached = true;
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
// VERIFY: 0a58e0c4 manual
// OFFSET: 0x008FDA10
FMOD_RESULT MusicChannelIT::vibrato() 
{
	int delta;
	unsigned char temp;
    MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();
    MusicSong *mod = mModule;

	temp = (unsigned char)(mVibPos & 31);

	switch (mWaveControlVibrato)
	{
		case 0: 
        {
            delta = gSineTable[temp];			/* sine */
			break;
        }
		case 1: 
        {
            temp <<= 3;								/* ramp down */
			if (mVibPos < 0) 
            {
				temp=(unsigned char)(255-temp);
            }
			delta=temp;
			break;
        }
		case 2: 
        {
            delta = 255;							/* square */
			break;
        }
		case 3: 
        {
            delta = FMOD_RAND()&255;				/* random */
			break;
        }
		default: 
        {
            delta = 0;
        }
	};

	delta *= mVibDepth;
	delta >>=7;
	delta <<=1;   /* we use 4*periods so make vibrato 4 times bigger */
	
	if (mod->mMusicFlags & FMUSIC_ITFLAGS_OLD_IT_EFFECTS) 
    {
		delta <<= 1;
    }

	mVibPos += mVibSpeed;
	if (mVibPos > 31) 
    {
		mVibPos -= 64;
    }

	if (mVibPos >= 0) 
    {
		vcptr->mFrequencyDelta += -delta;
    }
	else                   
    {
		vcptr->mFrequencyDelta += delta;
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
// VERIFY: b9210a46 auto
// OFFSET: 0x008FDAF0
FMOD_RESULT MusicChannelIT::fineVibrato() 
{
	int delta;
	unsigned char temp;
    MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();
    MusicSong *mod = mModule;

	temp = (unsigned char)(mVibPos & 31);

	switch (mWaveControlVibrato)
	{
		case 0: 
        {
            delta = gSineTable[temp];         /* sine */
			break;
        }
		case 1: 
        {
            temp <<= 3;								/* ramp down */
			if (mVibPos < 0) 
            {
				temp=255-temp;
            }
			delta=temp;
			break;
        }
		case 2: 
        {
            delta = 255;							/* square */
			break;
        }
		case 3: 
        {
            delta = FMOD_RAND()&255;						/* random */
			break;
        }
		default: 
        {
            delta = 0;
        }
	};

	delta *= mVibDepth;
	delta >>=7;
	
	if (mod->mMusicFlags & FMUSIC_ITFLAGS_OLD_IT_EFFECTS) 
    {
		delta <<= 1;
    }

	if (mVibPos >= 0) 
    {
		vcptr->mFrequencyDelta += delta;
    }
	else
    {
		vcptr->mFrequencyDelta += -delta;
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
// VERIFY: 0e2ec1da auto
// OFFSET: 0x008FDBC0
FMOD_RESULT MusicChannelIT::tremolo() 
{
	unsigned char temp;
    MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();

	temp = (unsigned char)(mTremoloPosition & 31);

	switch (mWaveControlTremolo)
	{
		case 0: 
        {
            mVolumeDelta = gSineTable[temp];		/* sine */
			break;
        }
		case 1: 
        {
            temp <<= 3;										/* ramp down */
			if (mTremoloPosition < 0) 
            {
				temp=255-temp;
            }
			mVolumeDelta=temp;
			break;
        }
		case 2: 
        {
            mVolumeDelta = 255;							/* square */
			break;
        }
		case 3: 
        {
            mVolumeDelta = gSineTable[temp];		/* random (just use sine for now) */
			break;
        }
	};

	mVolumeDelta *= mTremoloDepth;
	mVolumeDelta >>= 6;

	if (mTremoloPosition >= 0) 
	{
		if (mVolume + mVolumeDelta > 64) 
        {
			mVolumeDelta = 64 - mVolume;
        }
	}
	else 
	{
		if ((short)(mVolume-mVolumeDelta) < 0) 
        {
			mVolumeDelta = mVolume;
        }

		mVolumeDelta = -mVolumeDelta;	/* flip it */
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
	To carry out a panbrello at a certain depth and speed

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 54eb8835 manual
// OFFSET: 0x008FDCB0
FMOD_RESULT MusicChannelIT::panbrello() 
{
    MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();

	switch (mWaveControlPan)
	{
		case 0: 
        {
            vcptr->mPanDelta = (int)gFineSineTable[mPanbrelloPos];	/* sine */
			break;
        }
		case 1: 
        {
            vcptr->mPanDelta = (128-mPanbrelloPos)>>1;						/* ramp left */
			break;
        }
		case 2: 
        {
            if (mPanbrelloPos < 128) 
            {
				vcptr->mPanDelta =  64;					/* square */
            }
            else 
            {
				vcptr->mPanDelta = -64;								
            }
			break;
        }
		case 3: 
        {
            vcptr->mPanDelta = (int)gFineSineTable[mPanbrelloPos];	/* sine */
			break;
        }
	};
	vcptr->mPanDelta *= mPanbrelloDepth;
	vcptr->mPanDelta >>= 5;

	if (mPanbrelloPos >= 0) 
	{
		if (vcptr->mPan + vcptr->mPanDelta > 64) 
        {
			vcptr->mPanDelta = 64 - vcptr->mPan;
        }
	}
	else 
	{
		if ((short)(vcptr->mPan-vcptr->mPanDelta) < 0) 
        {
			vcptr->mPanDelta = vcptr->mPan;
        }
		vcptr->mPanDelta = -vcptr->mPanDelta;
	}

	mPanbrelloPos += mPanbrelloSpeed;
	if (mPanbrelloPos > 255) 
    {
		mPanbrelloPos -= 256;
    }

	vcptr->mNoteControl |= FMUSIC_PAN;

    return FMOD_OK;
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]
    Inumpoints = iptr->mVolumeNumPoints
    v          = &iptr->mVolumePoints
    type       = iptr->mVolumeType
    mLoopStart  = iptr->mVolumeSustainLoopStart
    loopend    = iptr->mVolumeSustainLoopEnd
    control    = FMUSIC_VOLUME

	[SEE_ALSO]
]
*/
// VERIFY: 622208e2 manual
// OFFSET: 0x008FDDB0
FMOD_RESULT CodecIT::processEnvelope(MusicEnvelopeState *env, MusicVirtualChannel *vcptr, int Inumpoints, MusicEnvelopeNode *v, int type, int mLoopStart, int loopend, int susmLoopStart, int susloopend, unsigned char control)
{
	if (env->mPosition < Inumpoints) 
	{
        if (env->mTick != v[env->mPosition].mTick)
        {
            env->mFraction += env->mDelta;				/* interpolate */

            if ((int)env->mFraction < 0 && type == FMUSIC_VOLUME)
            {
                env->mFraction = 0;
            }
        }
        else
        {
		    while (env->mTick == v[env->mPosition].mTick && env->mPosition < Inumpoints)	/* if we are at the correct tick for the position */
		    {
			    int currpos, nextpos;
			    int currtick, nexttick;
			    int currval, nextval, tickdiff;

		    mRestartenv:

			    currpos	= env->mPosition;
			    nextpos	= env->mPosition + 1;

			    currtick = v[currpos].mTick;			/* get tick at this point */
			    nexttick = v[nextpos].mTick;			/* get tick at next point */

			    currval = v[currpos].mValue << 16;	/* get val at this point << 16 */
			    nextval	= v[nextpos].mValue << 16;	/* get val at next point << 16 */

			    /*
                    Handle sustain loop
                */
			    if ((type & FMUSIC_ENVELOPE_SUSTAIN) && currpos >= susloopend && !vcptr->mKeyOff) 
			    {
				    if (susloopend == susmLoopStart)
				    {
					    env->mValue = v[currpos].mValue;
					    return FMOD_OK;
				    }
				    env->mPosition  = susmLoopStart;
				    env->mTick = v[env->mPosition].mTick - 1;

				    goto mRestartenv;
			    }

			    /*
                    Handle loop
                */
			    if ((type & FMUSIC_ENVELOPE_LOOP) && env->mPosition >= loopend) 
			    {
				    if (loopend <= mLoopStart)
				    {
					    env->mValue = v[mLoopStart].mValue;
					    return FMOD_OK;
				    }

				    env->mPosition  = mLoopStart;
				    env->mTick = v[env->mPosition].mTick - 1;
				    goto mRestartenv;
			    }

			    /*
                    If it is at the last position, abort the envelope and continue last val
                */
			    if (env->mPosition == Inumpoints - 1) 
			    {
				    env->mValue = v[currpos].mValue;
				    env->mStopped = true;
                    return FMOD_OK;
			    }

			    /*
                    Interpolate 2 points to find delta step
                */
			    tickdiff = nexttick - currtick;
			    if (tickdiff) 
                {
				    env->mDelta = (nextval-currval) / tickdiff;
                }
			    else
                {
				    env->mDelta = 0;
                }

			    env->mFraction  = currval;
			    env->mPosition++;
            }
		}
	}

	env->mValue = env->mFraction >> 16;
	env->mTick++;

	vcptr->mNoteControl |= control;

    return FMOD_OK;
}

#define FMUSIC_IT_GETPERIOD(_note)  ((int)gITLogPeriodTable[_note] * 8363L / vcptr->mSample->mMiddleC)

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: bb2e03a2 manual
// OFFSET: 0x008FDF30
FMOD_RESULT CodecIT::processPitchEnvelope(MusicVirtualChannel *vcptr, MusicInstrument *iptr, int note)
{
	/*
        Pan Envelope
    */
	if (vcptr->mEnvPitchPos < iptr->mPitchNumpoints) 
	{
		MusicEnvelopeNode *v = (MusicEnvelopeNode *)&iptr->mPitchPoints;

        if (vcptr->mEnvPitchTick != v[vcptr->mEnvPitchPos].mTick)
        {
            vcptr->mEnvPitchFrac += vcptr->mEnvPitchDelta;				/* interpolate */
        }
        else
        {
		    while (vcptr->mEnvPitchTick == v[vcptr->mEnvPitchPos].mTick)	/* if we are at the correct tick for the position */
		    {
			    int currpos, nextpos;
			    int currtick, nexttick;
			    int currpitch, nextpitch, tickdiff;

		    mRestartpitchenv:

			    currpos	= vcptr->mEnvPitchPos;
			    nextpos	= vcptr->mEnvPitchPos + 1;

			    currtick = v[currpos].mTick;			/* get tick at this point */
			    nexttick = v[nextpos].mTick;			/* get tick at next point */

			    if (mMusicFlags & FMUSIC_ITFLAGS_LINEARFREQUENCY || iptr->mPitchType & FMUSIC_ENVELOPE_FILTER)
			    {
				    currpitch = v[currpos].mValue * 32;
				    nextpitch = v[nextpos].mValue * 32;
			    }
			    else
			    {
                    if (v[currpos].mValue & 1)
                    {
                        int a,b,c;
                        a = FMUSIC_IT_GETPERIOD(note);
                        b = FMUSIC_IT_GETPERIOD(note + (v[currpos].mValue >> 1));
                        c = FMUSIC_IT_GETPERIOD(note + (v[currpos].mValue >> 1) + 1);
    					currpitch = a - ((b+c) / 2);
                    }
                    else
                    {
        				currpitch = FMUSIC_IT_GETPERIOD(note) - FMUSIC_IT_GETPERIOD(note + (v[currpos].mValue >> 1));
                    }

                    if (v[nextpos].mValue & 1)
                    {
                        int a,b,c;
                        a = FMUSIC_IT_GETPERIOD(note);
                        b = FMUSIC_IT_GETPERIOD(note + (v[nextpos].mValue >> 1));
                        c = FMUSIC_IT_GETPERIOD(note + (v[nextpos].mValue >> 1) + 1);
    					nextpitch = a - ((b+c) / 2);
                    }
                    else
                    {
    				    nextpitch = FMUSIC_IT_GETPERIOD(note) - FMUSIC_IT_GETPERIOD(note + (v[nextpos].mValue >> 1));
                    }			    
                }

			    currpitch <<= 16;
			    nextpitch <<= 16;

			    /*
                    Handle sustain loop
                */
			    if ((iptr->mPitchType & FMUSIC_ENVELOPE_SUSTAIN) && currpos >= iptr->mPitchSustainLoopEnd && !vcptr->mKeyOff) 
			    {
				    if (iptr->mPitchSustainLoopEnd == iptr->mPitchSustainLoopStart)
				    {
					    if (mMusicFlags & FMUSIC_ITFLAGS_LINEARFREQUENCY || iptr->mPitchType & FMUSIC_ENVELOPE_FILTER) 
					    {
						    vcptr->mEnvPitch = v[currpos].mValue * 32;
					    }
					    else
					    {
                            if (v[currpos].mValue & 1)
                            {
                                int a,b,c;
                                a = FMUSIC_IT_GETPERIOD(note);
                                b = FMUSIC_IT_GETPERIOD(note + (v[currpos].mValue >> 1));
                                c = FMUSIC_IT_GETPERIOD(note + (v[currpos].mValue >> 1) + 1);
    					        vcptr->mEnvPitch = a - ((b+c) / 2);
                            }
                            else
                            {
        					    vcptr->mEnvPitch = (FMUSIC_IT_GETPERIOD(note) - FMUSIC_IT_GETPERIOD(note + (v[currpos].mValue >> 1)));
                            }					    
                        }
					    return FMOD_OK;
				    }
				    vcptr->mEnvPitchPos  = iptr->mPitchSustainLoopStart;
				    vcptr->mEnvPitchTick = v[vcptr->mEnvPitchPos].mTick-1;
				    goto mRestartpitchenv;
			    }

			    /*
                    Handle loop
                */
			    if ((iptr->mPitchType & FMUSIC_ENVELOPE_LOOP) && vcptr->mEnvPitchPos >= iptr->mPitchLoopEnd) 
			    {
				    if (iptr->mPitchLoopEnd <= iptr->mPitchLoopStart)
				    {
					    if (mMusicFlags & FMUSIC_ITFLAGS_LINEARFREQUENCY || iptr->mPitchType & FMUSIC_ENVELOPE_FILTER) 
					    {
						    vcptr->mEnvPitch = v[iptr->mPitchLoopStart].mValue * 32;
					    }
					    else
					    {
                            if (v[currpos].mValue & 1)
                            {
                                int a,b,c;
                                a = FMUSIC_IT_GETPERIOD(note);
                                b = FMUSIC_IT_GETPERIOD(note + (v[currpos].mValue >> 1));
                                c = FMUSIC_IT_GETPERIOD(note + (v[currpos].mValue >> 1) + 1);
    					        vcptr->mEnvPitch = a - ((b+c) / 2);
                            }
                            else
                            {
        					    vcptr->mEnvPitch = (FMUSIC_IT_GETPERIOD(note) - FMUSIC_IT_GETPERIOD(note + (v[currpos].mValue >> 1)));
                            }
					    }
					    return FMOD_OK;
				    }
				    vcptr->mEnvPitchPos  = iptr->mPitchLoopStart;
				    vcptr->mEnvPitchTick = v[vcptr->mEnvPitchPos].mTick-1;
				    goto mRestartpitchenv;
			    }

			    /*
                    If it is at the last position, abort the envelope and continue last pitch
                */
			    if (vcptr->mEnvPitchPos == iptr->mPitchNumpoints - 1) 
			    {
				    if (mMusicFlags & FMUSIC_ITFLAGS_LINEARFREQUENCY || iptr->mPitchType & FMUSIC_ENVELOPE_FILTER) 
				    {
					    vcptr->mEnvPitch = v[currpos].mValue * 32;
				    }
				    else
				    {
                        if (v[currpos].mValue & 1)
                        {
                            int a,b,c;
                            a = FMUSIC_IT_GETPERIOD(note);
                            b = FMUSIC_IT_GETPERIOD(note + (v[currpos].mValue >> 1));
                            c = FMUSIC_IT_GETPERIOD(note + (v[currpos].mValue >> 1) + 1);
    					    vcptr->mEnvPitch = a - ((b+c) / 2);
                        }
                        else
                        {
    					    vcptr->mEnvPitch = FMUSIC_IT_GETPERIOD(note) - FMUSIC_IT_GETPERIOD(note + (v[currpos].mValue >> 1));
                        }
				    }
				    vcptr->mEnvPitchStopped = true;
				    return FMOD_OK;
			    }

			    /*
                    Interpolate 2 points to find delta step
                */
			    tickdiff = nexttick - currtick;
			    if (tickdiff) 
                {
				    vcptr->mEnvPitchDelta = (nextpitch-currpitch) / tickdiff;
                }
			    else
                {
				    vcptr->mEnvPitchDelta = 0;
                }

			    vcptr->mEnvPitchFrac  = currpitch;

			    vcptr->mEnvPitchPos++;
		    }
        }
	}

    if (!(iptr->mPitchType & FMUSIC_ENVELOPE_FILTER))
    {
    	vcptr->mNoteControl |= FMUSIC_FREQ;
    }

    vcptr->mEnvPitch = vcptr->mEnvPitchFrac >> 16;
	vcptr->mEnvPitchTick++;
    
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
	to carry out an instrument vibrato at a certain depth and speed            

	[PARAMETERS]
	track - the track number to do the vibrato too           
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: bdae8ebb manual
// OFFSET: 0x008FE4D0
FMOD_RESULT CodecIT::sampleVibrato(MusicVirtualChannel *vcptr) 
{
	int delta = 0;
	MusicSample *sptr = vcptr->mSample;

	switch (sptr->mVibType)
	{
		case 0: delta = (int)gFineSineTable[vcptr->mIVibPos];	/* sine */
				break;
		case 1: delta = (128-((vcptr->mIVibPos+128)%256))>>1;
				break;
		case 2: if (vcptr->mIVibPos < 128) 
					delta=64;						/* square */
				else 
					delta = -64;								
				break;
		case 3: delta = (int)gFineSineTable[vcptr->mIVibPos];	/* sine */
				break;
	};

	delta *= sptr->mVibDepth;
	delta = delta * vcptr->mIVibSweepPos >> 16;
	delta >>= 7;

	vcptr->mFrequencyDelta -= delta;
	
	vcptr->mIVibSweepPos += sptr->mVibRate << 1;
	if (vcptr->mIVibSweepPos > 256L << 8) 
    {
		vcptr->mIVibSweepPos = 256L << 8;
    }

	vcptr->mIVibPos += sptr->mVibSpeed;
	if (vcptr->mIVibPos > 255) 
    {
		vcptr->mIVibPos -= 256;
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
// VERIFY: 824ee61c manual
// OFFSET: 0x008FE5C0
FMOD_RESULT MusicChannelIT::processVolumeByte(MusicNote *current, bool newrow)
{
    MusicVirtualChannel *vcptr = (MusicVirtualChannel *)mVirtualChannelHead.getNext();
    MusicSong *mod = mModule;
	unsigned char val, volume = current->mVolume - 1;
	
	if (newrow)
	{
		/*
            Set volume
        */
		if (volume <= 64) 
		{
			mVolume = volume;
		}
		/*
            Fine volume slide up
        */
		if (volume >= 65 && volume <= 74) 
		{
			val = volume - 65;
			if (val) 
            {
				mVolumeColumnVolumeSlide = val;
            }

			mVolume += mVolumeColumnVolumeSlide;
			if (mVolume > 0x40) 
            {
				mVolume = 0x40;
            }
		}
		/*
            Fine volume slide down
        */
		if (volume >= 75 && volume <= 84) 
		{
			val = volume - 75;
			if (val) 
            {
				mVolumeColumnVolumeSlide = val;
            }

			mVolume -= mVolumeColumnVolumeSlide;
			if (mVolume < 0) 
            {
				mVolume = 0;
            }
		}
		/*
            Pan set
        */
		if (volume >= 128 && volume <= 192) 
		{
			mPan = (volume - 128);
			vcptr->mPan = mPan;
			vcptr->mNoteControl |= FMUSIC_PAN;
		}
	}

	/*
        Volume slide up
    */
	if (volume >= 85 && volume <= 94) 
	{
		val = volume - 85;
		if (val) 
        {
            mVolumeColumnVolumeSlide = val;
        }

		/*
            On other ticks beside 0
        */
		if (!newrow)
		{
			mVolume += mVolumeColumnVolumeSlide;
			if (mVolume > 0x40) 
            {
				mVolume = 0x40;
            }
		}
	}
	/*
        Volume slide down
    */
	if (volume >= 95 && volume <= 104) 
	{
		val = volume - 95;
		if (val)
        {
			mVolumeColumnVolumeSlide = val;
        }
		
		/*
            On other ticks beside 0
        */
		if (!newrow)
		{
			mVolume -= mVolumeColumnVolumeSlide;
			if (mVolume < 0) 
            {
				mVolume = 0;
            }
		}
	}

	/*
        Pitch slide up
    */
	if (volume >= 105 && volume <= 114) 
	{
		val = volume - 105;
		if (val) 
        {
			mPortaUpDown = val;
        }

		vcptr->mFrequency += mPortaUpDown << 4;
	}

	/*
        Pitch slide down
    */
	if (volume >= 115 && volume <= 124) 
	{
		val = volume - 115;
		if (val) 
        {
			mPortaUpDown = val;
        }

		vcptr->mFrequency -= mPortaUpDown << 4;
		if (vcptr->mFrequency < 1) 
        {
			vcptr->mNoteControl |= FMUSIC_STOP;
        }
		else 
        {
			vcptr->mNoteControl |= FMUSIC_FREQ;
        }
	}


	/*
        Portamento
    */
	if (volume >= 193 && volume <= 202) 
	{
		val = volume - 193;

		if (!mod->mTick)
		{
			if (val) 
			{
				if (mod->mMusicFlags & FMUSIC_ITFLAGS_EFFECT_G)
                {
					mPortaSpeed = val << 4;
                }
				else
                {
					mPortaUpDown = val << 4;
                }
			}
			mPortaTarget = mPeriod;
			
			if (current->mNote) 
            {
				mPortaReached = false;
            }
		}
		else 
        {
			portamento();
        }
	}

	/*
        Vibrato
    */
	if (volume >= 203 && volume <= 212) 
	{
		val = volume - 203;

		if (!mod->mTick)
		{
			if (val) 
            {
				mVibDepth = val;
            }

			if (val) 
            {
				mVibType = FMUSIC_IT_VIBRATO;
            }
		
			if (vcptr->mBackground) 
            {
				return FMOD_OK;	
            }

			if (!(mod->mMusicFlags & FMUSIC_ITFLAGS_OLD_IT_EFFECTS)) 
			{
				if (mVibType == FMUSIC_IT_FINEVIBRATO) 
                {
					fineVibrato();
                }
				else 
                {
					vibrato();
                }
			}
		}
		else 
		{
			if (vcptr->mBackground) 
            {
				return FMOD_OK;
            }

			if (mVibType == FMUSIC_IT_FINEVIBRATO)
            {
				fineVibrato();
            }
			else 
            {
				vibrato();
            }
		}
	}

    return FMOD_OK;
}



#ifdef FMUSIC_PLAYPACKED
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 78c3bb1b manual
// OFFSET: 0x008FE800
FMOD_RESULT CodecIT::unpackRow() 
{
	unsigned char	channelvariable;
	MusicNote       *nptr;

	if (!mPatternPtr)
    {
		return FMOD_ERR_INTERNAL;
    }

	FMOD_memset(mNote, 0, sizeof(MusicNote) * mNumChannels);

	do
	{
		channelvariable = *mPatternPtr++;

		if (channelvariable)
		{
			unsigned char channel, maskvariable;

			channel = (channelvariable-1) & 63;	

			nptr = &mNote[channel];

			if (channelvariable & 128) 
			{
				maskvariable = *mPatternPtr++;
				mPreviousMaskVariable[channel] = maskvariable;
			}
			else 
				maskvariable = mPreviousMaskVariable[channel];

			if (maskvariable & 1) 
			{
				unsigned char note = *mPatternPtr++;

				if (note >= 254)
                {
					nptr->mNote = note;
                }
				else 
                {
					nptr->mNote = note + 1;
                }
				mLastNote[channel] = nptr->mNote;
			}
			if (maskvariable & 2) 
			{
				nptr->mNumber = *mPatternPtr++;
				mLastNumber[channel] = nptr->mNumber;
			}
			if (maskvariable & 4) 
			{
				nptr->mVolume = (*mPatternPtr++) + 1;
				mLastVolume[channel] = nptr->mVolume;
			}
			if (maskvariable & 8) 
			{ 
				nptr->mEffect = *mPatternPtr++;
				nptr->mEffectParam = *mPatternPtr++;
				mLastEffect[channel] = nptr->mEffect;
				mLastEffectParam[channel] = nptr->mEffectParam;
			}
			if (maskvariable & 16)  nptr->mNote	 = mLastNote[channel];
			if (maskvariable & 32)  nptr->mNumber = mLastNumber[channel];
			if (maskvariable & 64)  nptr->mVolume = mLastVolume[channel];
			if (maskvariable & 128) 
			{ 
				nptr->mEffect = mLastEffect[channel]; 
				nptr->mEffectParam = mLastEffectParam[channel];
			}
		}
	} while (channelvariable);

    return FMOD_OK;
}
#endif




/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: dc6daf87 manual
// OFFSET: 0x008FE980
FMOD_RESULT CodecIT::updateRow(bool audible)
{
	MusicNote    *current;
	bool          breakflag = false;
	bool          jumpflag  = false;
	int           count;
	MusicPattern *pptr;

	/*
        Point our note pointer to the correct pattern buffer, and to the
	    correct offset in this buffer indicated by row and number of channels
    */
	pptr = &mPattern[mOrderList[mOrder]];
	if (!pptr) 
    {
		return FMOD_OK;
    }

#ifndef FMUSIC_PLAYPACKED
	FMOD_memcpy(mNote, pptr->mData + (mRow * mNumChannels), sizeof(MusicNote) * mNumChannels);
#endif

    if (!mTick && mVisited)
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
	for (count = 0; count < mNumChannels; count++)
	{
		MusicChannelIT        *cptr  = 0;
		MusicVirtualChannel   *vcptr = 0;
		bool                   porta  = false;
        bool                   trig   = false;
        bool                   newrow = false;
		unsigned char          paramx, paramy;
      	
		current = &mNote[count];
		paramx = current->mEffectParam >> 4;			/* get effect param x */
		paramy = current->mEffectParam & 0xF;			/* get effect param y */

		/*
            Dont do shit if it is a delay note (maybe wrong .. may need to update vibratos etc)
        */
		if (current->mEffect == FMUSIC_IT_SPECIAL && paramx == FMUSIC_IT_NOTEDELAY && (mTick < paramy || (!mTick && !paramy)) ) 
        {
			continue;
        }
		
		/*
            Just entered after a delay note
        */
		if (mTick == 0) 
        {
			newrow = true;
        }
		else if (current->mEffect == FMUSIC_IT_SPECIAL && paramx == FMUSIC_IT_NOTEDELAY && (mTick == paramy || (mTick == 1 && !paramy))) 
        {
			newrow = true;
        }
		
		cptr = (MusicChannelIT *)mMusicChannel[count];
		if (cptr->mVirtualChannelHead.isEmpty()) 
		{
			vcptr             = &gDummyVirtualChannel; /* no channels allocated yet */
			vcptr->mSample    = &gDummySample;
		}
        else
        {
            vcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext();
        }

		if (newrow)
		{
			/*  
                If there is no more tremolo, set volume to volume + last tremolo delta
            */
			if (cptr->mRecentEffect == FMUSIC_IT_TREMOLO && current->mEffect != FMUSIC_IT_TREMOLO)
            {
				cptr->mVolume += cptr->mVolumeDelta;
            }
			
			cptr->mRecentEffect  = current->mEffect;

			cptr->mVolumeDelta  = 0;
			vcptr->mPanDelta    = 0;
			vcptr->mNoteControl = 0;

			/*
                RETRIEVE NEW INSTRUMENT NUMBER AND / OR NOTE
            */
			if (current->mNote && current->mNote != FMUSIC_KEYOFF && current->mNote != FMUSIC_KEYCUT) 
			{
				trig = true;
				cptr->mNote = current->mNote-1;		/* remember the note */
			}

			if (current->mNumber) 
			{
				cptr->mInstrument = current->mNumber-1;		/* remember the Instrument # */

				/*
                    another crazy bugfix (2.09)
                */
				if (vcptr == &gDummyVirtualChannel)
                {
					trig = true;
                }
                
//			    if (InstCallback[current->mNumber] && InstCallback[current->mNumber]->callback)
//                {
//					FMUSIC_CheckCallback(mod, FMUSIC_CALLBACK_INSTRUMENT, current->mNumber);
//                }
			}

			if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS) 
			{
				if (cptr->mInstrument >= mNumInstruments) 
                {
					trig = false;
                }
			}

			/*
                RESET STUFF
            */
			cptr->mRecentEffect  = current->mEffect;
			vcptr->mNoteControl		= 0;

			if (mDefaultPan[count] & 128) 
            {
				trig = false;
            }

			porta = (current->mEffect == FMUSIC_IT_PORTATO || current->mEffect == FMUSIC_IT_PORTATOVOLSLIDE || ((current->mVolume-1) >= 193 && (current->mVolume-1) <= 202) );
			
			/*
                bugfix (fmod 2.09) 
            */
			if (porta && vcptr == &gDummyVirtualChannel) 
            {
				porta = false;
            }

			/*
                PROCESS NOTE
            */
			if (trig)
			{
				int samp;
				MusicInstrument *iptr = &mInstrument[cptr->mInstrument];
				
				if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS) 
				{
					samp = iptr->mNoteTable[(cptr->mNote<<1)+1];
				}
				else
                { 
					samp = cptr->mInstrument+1;
                }
				
				if (samp)
				{
					MusicVirtualChannel *childvcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext();
                                        
					
					cptr->mSample = samp-1;

					/*
                        DTC / DCA's
                    */
					if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS) 
					{
						if (iptr->mDupCheckType)
						{
							/*
                                Search through child channels for any duplicate actions, if so set cut,fade or off
                            */                            
							for (childvcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext(); childvcptr != &cptr->mVirtualChannelHead; childvcptr = (MusicVirtualChannel *)childvcptr->getNext())
							{
								switch (iptr->mDupCheckType)
								{
									case 1 :	/* NOTE */
									{
										if (cptr->mNote == childvcptr->mLastNote)
										{
											switch (iptr->mDupCheckAction)
											{
												case 0: 
                                                {
                                                    childvcptr->mNoteControl |= FMUSIC_STOP; 
                                                    break;
                                                }
												case 1: 
												{
													MusicInstrument *iptr = &mInstrument[childvcptr->mLastInstrument];
													childvcptr->mKeyOff = true; 
													if (!(vcptr->mVolType & FMUSIC_ENVELOPE_ON)) 
                                                    {
														childvcptr->mFade = true;
                                                    }
													else if (iptr->mVolumeType & FMUSIC_ENVELOPE_LOOP) 
                                                    {
														childvcptr->mFade = true;	/* undocumented */
                                                    }
													break;
												}
												case 2: 
                                                {
                                                    childvcptr->mFade = true; 
                                                    break;
                                                }
											}
										}
										break;
									}
									case 2 :	/* SAMPLE */
									{
										if (cptr->mSample == childvcptr->mLastSample)
										{
											switch (iptr->mDupCheckAction)
											{
												case 0: 
                                                {
                                                    childvcptr->mNoteControl |= FMUSIC_STOP; 
                                                    break;
                                                }
												case 1: 
												{
													MusicInstrument *iptr = &mInstrument[childvcptr->mLastInstrument];
													childvcptr->mKeyOff = true; 
													if (!(vcptr->mVolType & FMUSIC_ENVELOPE_ON)) 
                                                    {
														childvcptr->mFade = true;
                                                    }
													else if (iptr->mVolumeType & FMUSIC_ENVELOPE_LOOP) 
                                                    {
														childvcptr->mFade = true;	/* undocumented */
                                                    }
													break;
												}
												case 2: 
                                                {
                                                    childvcptr->mFade = true; 
                                                    break;
                                                }
											}
										}
										break;
									}
									case 3 :	/* INSTRUMENT */
									{
										if (cptr->mInstrument == childvcptr->mLastInstrument)
										{
											switch (iptr->mDupCheckAction)
											{
												case 0: 
                                                {
                                                    childvcptr->mNoteControl |= FMUSIC_STOP; 
                                                    break;
                                                }
												case 1: 
												{
													MusicInstrument *iptr = &mInstrument[childvcptr->mLastInstrument];
													childvcptr->mKeyOff = true; 
													if (!(vcptr->mVolType & FMUSIC_ENVELOPE_ON)) 
                                                    {
														childvcptr->mFade = true;
                                                    }
													else if (iptr->mVolumeType & FMUSIC_ENVELOPE_LOOP) 
                                                    {
														childvcptr->mFade = true;	/* undocumented */
                                                    }
													break;
												}
												case 2: 
                                                {
                                                    childvcptr->mFade = true; 
                                                    break;
                                                }
											}
										}
										break;
									}
								}
							}
						}
					}		

					/* 
                        CHANNEL ALLOCATION, NNA'S
                    */
					if (!porta) /* dont spawn any new channels if it is a portamento */
					{
                        vcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext();
						
						/*
                            If the channel has not got any children, then just spawn a new one
                        */
						if (vcptr == &cptr->mVirtualChannelHead)
						{
                            FMOD_RESULT result;

							result = spawnNewVirtualChannel(cptr, 0, &vcptr);
							if (result != FMOD_OK)
							{
				                vcptr             = &gDummyVirtualChannel; /* no channels allocated yet */
                				vcptr->mSample    = &gDummySample;
							}
						}
						else if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
						{
							MusicVirtualChannel *newvcptr;
							MusicInstrument     *iptr = &mInstrument[vcptr->mLastInstrument];

							switch (vcptr->mNNA)
							{
								case FMUSIC_CUT :
								{
									/*
                                        nothing special here.. just use this channel and exit
                                    */
									break;
								}
								case FMUSIC_OFF :
								{
                                    FMOD_RESULT result;

									vcptr->mBackground = true;
									vcptr->mKeyOff = true;
									if (!(vcptr->mVolType & FMUSIC_ENVELOPE_ON)) 
                                    {
										vcptr->mFade = true;
                                    }
									else if (iptr->mVolumeType & FMUSIC_ENVELOPE_LOOP) 
                                    {
										vcptr->mFade = true;	/* undocumented */
                                    }

									if (!cptr->mVolume) 
                                    {
										vcptr->mNoteControl |= FMUSIC_STOP;	/* remove if it is silent */
                                    }

									result = spawnNewVirtualChannel(cptr, 0, &newvcptr);
									if (!newvcptr)
									{
									    result = result;
									}
									
									if (result == FMOD_OK) 
                                    {
										vcptr = newvcptr;
                                    }
									else
									{
			            	            vcptr             = &gDummyVirtualChannel; /* no channels allocated yet */
            			        	    vcptr->mSample    = &gDummySample;
									}
									vcptr->mVolume = cptr->mVolume;

									break;
								}
								case FMUSIC_CONTINUE :
								{
                                    FMOD_RESULT result;

									vcptr->mBackground = true;
									if (!cptr->mVolume) 
                                    {
										vcptr->mNoteControl |= FMUSIC_STOP;	/* remove if it is silent */
                                    }

									result = spawnNewVirtualChannel(cptr, 0, &newvcptr);
									if (!newvcptr)
									{
									    result = result;
									}
									if (result == FMOD_OK) 
                                    {
										vcptr = newvcptr;
                                    }
									else
									{
				                        vcptr             = &gDummyVirtualChannel; /* no channels allocated yet */
				                        vcptr->mSample    = &gDummySample;
									}
									vcptr->mVolume = cptr->mVolume;

									break;
								}
								case FMUSIC_FADEOUT :
								{
                                    FMOD_RESULT result;

									vcptr->mBackground = true;
									vcptr->mFade = true;
									if (!cptr->mVolume) 
                                    {
										vcptr->mNoteControl |= FMUSIC_STOP;	/* remove if it is silent */
                                    }

									result = spawnNewVirtualChannel(cptr, 0, &newvcptr);
									if (!newvcptr)
									{
									    result = result;
									}
									if (result == FMOD_OK) 
                                    {
										vcptr = newvcptr;
                                    }
									else
									{
				                        vcptr             = &gDummyVirtualChannel; /* no channels allocated yet */
				                        vcptr->mSample    = &gDummySample;
									}
									vcptr->mVolume = cptr->mVolume;

									break;
								}
							};
						}

						if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
						{
							vcptr->mBackground = false;
							vcptr->mNNA	       = mInstrument[cptr->mInstrument].mNNA;
							vcptr->mVolType    = mInstrument[cptr->mInstrument].mVolumeType;
						}

						vcptr->mLastSample = cptr->mSample;
						vcptr->mLastInstrument = cptr->mInstrument;
						vcptr->mLastNote = cptr->mNote;
						if (cptr->mSample >= mNumSamples || !mSample[cptr->mSample])
                        {
							vcptr->mSample = &gDummySample;
                        }
						else
                        {
							vcptr->mSample = mSample[cptr->mSample];
                        }
					}

					if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS) 
                    {
						cptr->mRealNote = iptr->mNoteTable[(cptr->mNote<<1)];	/* get note according to relative note */
                    }
					else 
                    {
						cptr->mRealNote = cptr->mNote;
                    }
					
					/*
                        Get period according to mRealNote and finetune (linear freqs are just 6bit fixed point)
                    */
					if (mMusicFlags & FMUSIC_ITFLAGS_LINEARFREQUENCY) 
					{
						float scalar, logs;

						/*
                            10 octaves, 12 notes per octave, 64 linear steps per note
                        */
						cptr->mPeriod = (10*12*64) - (cptr->mRealNote*64);

						/* 
                            Fine tune - as mMiddleC hz is logarithmic and our periods are linear.. special conversion is needed
                        */
						scalar = (float)vcptr->mSample->mMiddleC / 8363.0f;
						logs = (float)FMOD_LOG(scalar);	
						logs /= 0.693147f; /*log(2.0f);	 */

						logs *= 768.0f;	

						cptr->mPeriod -= (int)logs;	
					}
					else 
                    {
						cptr->mPeriod = FMUSIC_IT_GETPERIOD(cptr->mRealNote);
                    }

					/*
                        frequency only changes if there are no portamento effects
                    */
					if (!porta)
					{

						/*
                            retrigger tremolo / vibrato panbrello waveforms
                        */
						cptr->mVibPos           = 0;
						cptr->mTremoloPosition  = 0;
						cptr->mPanbrelloPos     = 0;
						cptr->mTremorPosition	= 0;			/* retrigger tremor count */
						cptr->mRetrigCount      = 0;
			
						vcptr->mFrequency = cptr->mPeriod;
                        vcptr->mPan	      = cptr->mPan;
                        vcptr->mDirection = 1;
						
						if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
                        {
							if (!(iptr->mDefaultPan & 128)) 
                            {
								vcptr->mPan = iptr->mDefaultPan * 64 / 63;
                            }
                        }

						if (vcptr->mSample->mDefaultPan & 128) 
                        {
							vcptr->mPan = (vcptr->mSample->mDefaultPan & 127);
                        }

						if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
						{
							vcptr->mPan = vcptr->mPan + ((cptr->mNote - iptr->mPitchPanCenter) * iptr->mPitchPanSep / 8);
							vcptr->mSampGlobalVol = vcptr->mSample->mGlobalVolume * iptr->mGlobalVolume / 128;
						}
						else 
                        {
							vcptr->mSampGlobalVol = vcptr->mSample->mGlobalVolume;
                        }

						vcptr->mNoteControl = FMUSIC_TRIGGER | FMUSIC_PAN;
					}

					if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS && ((porta && (mMusicFlags & FMUSIC_ITFLAGS_EFFECT_G)) || !porta))
					{
						vcptr->mKeyOff = false;

						if (!(mInstrument[cptr->mInstrument].mVolumeType & FMUSIC_ENVELOPE_CARRY) || vcptr->mNNA == FMUSIC_CUT)
						{
							vcptr->mEnvVolume.mValue    = 64;
							vcptr->mEnvVolume.mPosition = 0;
							vcptr->mEnvVolume.mTick     = 0;
							vcptr->mEnvVolume.mDelta    = 0;
						}

						if (!(mInstrument[cptr->mInstrument].mPanType & FMUSIC_ENVELOPE_CARRY) || vcptr->mNNA == FMUSIC_CUT)
						{
							vcptr->mEnvPan.mValue       = 0;
							vcptr->mEnvPan.mPosition    = 0;
							vcptr->mEnvPan.mTick        = 0;
							vcptr->mEnvPan.mDelta       = 0;
						}

						if (!(mInstrument[cptr->mInstrument].mPitchType & FMUSIC_ENVELOPE_CARRY) || vcptr->mNNA == FMUSIC_CUT)
						{
							vcptr->mEnvPitch      = 0;
							vcptr->mEnvPitchPos   = 0;
							vcptr->mEnvPitchTick  = 0;
							vcptr->mEnvPitchDelta = 0;
						}

						vcptr->mFade			    = false;
						vcptr->mFadeOutVolume	    = 1024;
						vcptr->mEnvVolume.mStopped  = false;
						vcptr->mEnvPan.mStopped     = false;
						vcptr->mIVibSweepPos	    = 0;
						vcptr->mIVibPos		        = 0;
					}
				}
			}

			/*
                PROCESS SAMPLE / INSTRUMENT NUMBER
            */
			if (vcptr->mSample && current->mNumber) 
            {
				cptr->mVolume = vcptr->mSample->mDefaultVolume;
            }
		}


/*		vcptr->mFrequencyDelta = 0;
*/
		/*
            bugfix - 3.10 - child channels still had their freqdelta so some child channels were out of tune etc.
        */
		{
			MusicVirtualChannel *childvcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext();
            for (childvcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext(); childvcptr != &cptr->mVirtualChannelHead; childvcptr = (MusicVirtualChannel *)childvcptr->getNext())
            {
				childvcptr->mFrequencyDelta = 0;
            }
		}

		vcptr->mNoteControl	|= FMUSIC_FREQ;
		vcptr->mNoteControl |= FMUSIC_VOLUME;

		/*
            PROCESS VOLUME BYTE
        */
		if (current->mVolume) 
        {
			cptr->processVolumeByte(current, newrow);
        }

		/*
            PROCESS KEY OFF
        */
		if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
		{
			MusicInstrument *iptr = &mInstrument[cptr->mInstrument];

			if (current->mNote == FMUSIC_KEYOFF) 
			{
				vcptr->mKeyOff = true;
				if (!(vcptr->mVolType & FMUSIC_ENVELOPE_ON)) 
                {
					vcptr->mFade = true;
                }
				else if (iptr->mVolumeType & FMUSIC_ENVELOPE_LOOP) 
                {
					vcptr->mFade = true;	/* undocumented */
                }
			}
		}

		if (current->mNote == FMUSIC_KEYCUT) 
        {
			vcptr->mNoteControl |= FMUSIC_STOP;
        }

		/*
            PROCESS EFFECTS
        */
		switch (current->mEffect) 
		{
			case FMUSIC_IT_SETSPEED : 
			{
				if (!mTick && current->mEffectParam)  /* 3.41 BUGFIX - speed 0 was being parsed */
                {
					mSpeed = current->mEffectParam;
                }
				break;
			}
			case FMUSIC_IT_PATTERNJUMP : /* --- 00 B00 : --- 00 D63 , should put us at ord=0, row=63 */
			{
				if (!mTick) 
				{
					mNextOrder = current->mEffectParam;
					mNextRow = 0;
					if (mNextOrder >= mNumOrders) 
                    {
						mNextOrder=0;
                    }
					jumpflag = 1;
				}
				break;
			}
			case FMUSIC_IT_PATTERNBREAK : 
			{
				if (!mTick) 
				{
					mNextRow = current->mEffectParam; 
					if (mNextRow > pptr->mRows) 
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
				}
				break;
			}
			case FMUSIC_IT_VOLUMESLIDE : 
			{
				if (!mTick)
				{
					/* Dxy - Volume slide, fine vol  DFx = slide down, DxF = slide up */
					if (current->mEffectParam) 
                    {
						cptr->mVolumeSlide = current->mEffectParam;
                    }

					if (vcptr->mBackground) 
                    {
						break;
                    }

					/* DFF is classed as a slide up so it gets priority */
					if ((cptr->mVolumeSlide &0xF) == 0xF) 
                    {
						cptr->mVolume += (cptr->mVolumeSlide >> 4);
                    }
					else if ((cptr->mVolumeSlide >>4 ) == 0xF) 
                    {
						cptr->mVolume -= (cptr->mVolumeSlide & 0xF);
                    }
					if (cptr->mVolume > 64) 
                    {
						cptr->mVolume = 64;
                    }
					if (cptr->mVolume < 0)  
                    {
						cptr->mVolume = 0;
                    }
				}
				else 
				{
					if (vcptr->mBackground) 
                    {
						break;
                    }

					cptr->volumeSlide();
				}
				break;				
			}
			case FMUSIC_IT_PORTADOWN :
			{
				if (!mTick)
				{
					if (current->mEffectParam) 
                    {
						cptr->mPortaUpDown = current->mEffectParam;
                    }

					if (vcptr->mBackground) 
                    {
						break;
                    }

					if ((cptr->mPortaUpDown >> 4)==0xF) 
                    {
						vcptr->mFrequency += ((cptr->mPortaUpDown & 0xF) << 2);
                    }
					if ((cptr->mPortaUpDown >> 4)==0xE) 
                    {
						vcptr->mFrequency += (cptr->mPortaUpDown & 0xF);
                    }
				}
				else if (cptr->mPortaUpDown <0xE0) 
				{
					if (vcptr->mBackground) 
                    {
						break;
                    }

					vcptr->mFrequency += (cptr->mPortaUpDown << 2);
				}

				break;
			}

			case FMUSIC_IT_PORTAUP :
			{
				if (!mTick)
				{
					if (current->mEffectParam) 
                    {
                        cptr->mPortaUpDown = current->mEffectParam;
                    }

					if (vcptr->mBackground) 
                    {
						break;
                    }

					if ((cptr->mPortaUpDown >>4)==0xF) 
                    {
						vcptr->mFrequency -= ((cptr->mPortaUpDown & 0xF) << 2);
                    }
					if ((cptr->mPortaUpDown >>4)==0xE) 
                    {
						vcptr->mFrequency -= (cptr->mPortaUpDown & 0xF);
                    }
				}
				else 
				{
					if (vcptr->mBackground) 
                    {
						break;
                    }

					if (cptr->mPortaUpDown <0xE0) 
					{
						vcptr->mFrequency -= cptr->mPortaUpDown << 2;
						if (vcptr->mFrequency < 1) 
                        {
							vcptr->mNoteControl |= FMUSIC_STOP;
                        }
						else 
                        {
							vcptr->mNoteControl |= FMUSIC_FREQ;
                        }
					}
				}
				break;
			}
			case FMUSIC_IT_PORTATO :
			{
				if (!mTick)
				{
					if (current->mEffectParam) 
					{
						if (mMusicFlags & FMUSIC_ITFLAGS_EFFECT_G)
                        {
							cptr->mPortaSpeed = current->mEffectParam;
                        }
						else
                        {
							cptr->mPortaUpDown = current->mEffectParam;
                        }
					}
					cptr->mPortaTarget = cptr->mPeriod;
					
					if (current->mNote) 
                    {
						cptr->mPortaReached = false;
                    }
				}
				else
				{
					if (vcptr->mBackground) 
                    {
						break;
                    }

					cptr->portamento();
				}
				break;
			}

			case FMUSIC_IT_VIBRATO :
			{
				if (!mTick)
				{
					if (paramx) 
						cptr->mVibSpeed = paramx;
					if (paramy) 
						cptr->mVibDepth = paramy;

					if (paramy) 
						cptr->mVibType = FMUSIC_IT_VIBRATO;
				
					if (vcptr->mBackground) 
                    {
						break;
                    }

					if (!(mMusicFlags & FMUSIC_ITFLAGS_OLD_IT_EFFECTS)) 
					{
						if (cptr->mVibType == FMUSIC_IT_FINEVIBRATO) 
                        {
							cptr->fineVibrato();
                        }
						else 
                        {
							cptr->vibrato();
                        }
					}
				}
				else 
				{
					if (vcptr->mBackground) 
                    {
						break;
                    }

					if (cptr->mVibType == FMUSIC_IT_FINEVIBRATO) 
                    {
						cptr->fineVibrato();
                    }
					else 
                    {
						cptr->vibrato();
                    }
				}

				break;
			}
			case FMUSIC_IT_TREMOR :
			{
				if (!mTick && current->mEffectParam) 
				{
					cptr->mTremorOn = (paramx+1);
					cptr->mTremorOff = (paramy+1);
				}

	    		if (vcptr->mBackground) 
                {
    				break;
                }

				if (cptr->mTremorPosition >= cptr->mTremorOn) 
                {
					cptr->mVolumeDelta = -cptr->mVolume;
                }
				cptr->mTremorPosition++;
				if (cptr->mTremorPosition >= (cptr->mTremorOn + cptr->mTremorOff)) 
                {
					cptr->mTremorPosition = 0;
                }
				vcptr->mNoteControl |= FMUSIC_VOLUME;
				break;
			}
			case FMUSIC_IT_ARPEGGIO :
			{
				if (!mTick && current->mEffectParam) 
                {
					cptr->mArpeggio = current->mEffectParam;
                }

				if (vcptr->mBackground) 
                {
					break;
                }

				if (cptr->mArpeggio > 0)
				{
					paramx = cptr->mArpeggio >> 4;
					paramy = cptr->mArpeggio & 0xF;

					switch (mTick % 3) 
					{
						case 1: 
                        {
							if (mMusicFlags & FMUSIC_ITFLAGS_LINEARFREQUENCY) 
                            {
								vcptr->mFrequencyDelta -= (int)(paramx) << 6;
                            }
							else 
                            {
								vcptr->mFrequencyDelta += (FMUSIC_IT_GETPERIOD(cptr->mNote+paramx) -FMUSIC_IT_GETPERIOD(cptr->mNote));
                            }
							break;
                        }
						case 2: 
                        {
							if (mMusicFlags & FMUSIC_ITFLAGS_LINEARFREQUENCY) 
                            {
								vcptr->mFrequencyDelta -= (int)(paramy) << 6;
                            }
							else 
                            {
								vcptr->mFrequencyDelta += (FMUSIC_IT_GETPERIOD(cptr->mNote+paramy) - FMUSIC_IT_GETPERIOD(cptr->mNote));
                            }
							break;
                        }
					};
					vcptr->mNoteControl |= FMUSIC_FREQ;
				}
				break;
			}
			case FMUSIC_IT_VIBRATOVOLSLIDE :
			{
				if (!mTick)
				{
					if (current->mEffectParam) 
                    {
						cptr->mVolumeSlide = current->mEffectParam;
                    }

					if (vcptr->mBackground) 
                    {
						break;
                    }

					/*
                        DFF is classed as a slide up so it gets priority
                    */
					if ((cptr->mVolumeSlide &0xF) == 0xF) 
                    {
						cptr->mVolume += (cptr->mVolumeSlide >> 4);
                    }
					else if ((cptr->mVolumeSlide >>4 ) == 0xF) 
                    {
						cptr->mVolume -= (cptr->mVolumeSlide & 0xF);
                    }

					if (cptr->mVolume > 64) 
                    {
						cptr->mVolume = 64;
                    }
					if (cptr->mVolume < 0)  
                    {
						cptr->mVolume = 0;
                    }

					if (!(mMusicFlags & FMUSIC_ITFLAGS_OLD_IT_EFFECTS)) 
					{
						if (cptr->mVibType == FMUSIC_IT_FINEVIBRATO) 
                        {
							cptr->fineVibrato();
                        }
						else 
                        {
							cptr->vibrato();
                        }
					}

				}
				else
				{
					if (vcptr->mBackground) 
                    {
						break;
                    }

					if (cptr->mVibType == FMUSIC_IT_FINEVIBRATO) 
                    {
						cptr->fineVibrato();
                    }
					else 
                    {
						cptr->vibrato();
                    }
					
					cptr->volumeSlide();
				}
				break;
			}
			case FMUSIC_IT_PORTATOVOLSLIDE :
			{
				if (!mTick)
				{
					if (current->mEffectParam) 
                    {
						cptr->mVolumeSlide = current->mEffectParam;
                    }
					cptr->mPortaTarget = cptr->mPeriod;
					if (current->mNote) 
                    {
						cptr->mPortaReached = false;
                    }

					if (vcptr->mBackground) 
                    {
						break;
                    }

					/*
                        DFF is classed as a slide up so it gets priority
                    */
					if  ((cptr->mVolumeSlide &0xF) == 0xF) 
                    {
						cptr->mVolume += (cptr->mVolumeSlide >> 4);
                    }
					else if ((cptr->mVolumeSlide >>4 ) == 0xF) 
                    {
						cptr->mVolume -= (cptr->mVolumeSlide & 0xF);
                    }

					if (cptr->mVolume > 64) 
                    {
						cptr->mVolume = 64;
                    }
					if (cptr->mVolume < 0)  
                    {
						cptr->mVolume = 0;
                    }
				}
				else
				{
					if (vcptr->mBackground) 
                    {
						break;
                    }

					cptr->portamento();
					cptr->volumeSlide();
				}
				break;
			}
			case FMUSIC_IT_SETCHANNELVOLUME :
			{
				if (!mTick)
				{
					cptr->mGlobalVolume = current->mEffectParam;
					if (cptr->mGlobalVolume > 0x40) 
                    {
						cptr->mGlobalVolume = 0x40;
                    }
				}
				break;
			}
			case FMUSIC_IT_CHANNELVOLSLIDE:
			{
				if (!mTick)
				{
					/*
                        Nxy - Volume slide, fine vol  NFx = slide down, NxF = slide up
                    */
					if (current->mEffectParam) 
                    {
						cptr->mChannelVolumeSlide = current->mEffectParam;
                    }

					if (vcptr->mBackground) 
                    {
						break;
                    }

					/*
                        NFF is classed as a slide up so it gets priority
                    */
					if ((cptr->mChannelVolumeSlide &0xF) == 0xF) 
                    {
						cptr->mGlobalVolume += (cptr->mChannelVolumeSlide >> 4);
                    }
					else if ((cptr->mChannelVolumeSlide >>4 ) == 0xF) 
                    {
						cptr->mGlobalVolume -= (cptr->mChannelVolumeSlide & 0xF);
                    }
				}
				else
				{
					if (vcptr->mBackground) 
                    {
						break;
                    }

					if (!(cptr->mChannelVolumeSlide & 0xF)) 
                    {
						cptr->mGlobalVolume += (cptr->mChannelVolumeSlide >> 4);
                    }
					if (!(cptr->mChannelVolumeSlide >>  4)) 
                    {
						cptr->mGlobalVolume -= (cptr->mChannelVolumeSlide & 0xF);
                    }
				}

				if (cptr->mGlobalVolume > 64) 
                {
					cptr->mGlobalVolume = 64;
                }
				if (cptr->mGlobalVolume < 0)  
                {
					cptr->mGlobalVolume = 0;
                }
				break;				
			}
			case FMUSIC_IT_SETSAMPLEOFFSET : 
			{
				unsigned int offset;

				if (!mTick)
				{
					if (!vcptr->mSample) 
                    {
						break;
                    }

					if (current->mEffectParam) 
                    {
						cptr->mSampleOffset = current->mEffectParam;
                    }

					offset = cptr->mHighOffset;
					offset <<= 16;
					offset += (int)(cptr->mSampleOffset) << 8;

					if (offset >= vcptr->mSample->mLoopStart + vcptr->mSample->mLoopLength)
                    {
						offset = vcptr->mSample->mLoopStart + vcptr->mSample->mLoopLength - 1;
                    }

					vcptr->mSampleOffset = offset;
				}
				break;
			}
			case FMUSIC_IT_RETRIGVOLSLIDE :
			{
				if (!mTick && current->mEffectParam) 
				{
					cptr->mRetrigX = paramx;
					cptr->mRetrigY = paramy;
				}
				
				if (cptr->mRetrigCount)
				{
					if (vcptr->mBackground)
                    {
						break;
                    }

					if (!cptr->mRetrigY) 
                    {
						break; /* divide by 0 bugfix */
                    }
					
					if (!(cptr->mRetrigCount % cptr->mRetrigY)) 
					{
						if (cptr->mRetrigX) 
						{
							switch (cptr->mRetrigX) 
							{
								case 1: cptr->mVolume--;
										break;
								case 2: cptr->mVolume -= 2;
										break;
								case 3: cptr->mVolume -= 4;
										break;
								case 4: cptr->mVolume -= 8;
										break;
								case 5: cptr->mVolume -= 16;
										break;
								case 6: cptr->mVolume *= 2/3;
										break;
								case 7: cptr->mVolume >>= 1;
										break;
								case 8: /* ? */
										break;
								case 9: cptr->mVolume++;
										break;
								case 0xA: cptr->mVolume += 2;
										break;
								case 0xB: cptr->mVolume += 4;
										break;
								case 0xC: cptr->mVolume += 8;
										break;
								case 0xD: cptr->mVolume += 16;
										break;
								case 0xE: cptr->mVolume *= 3/2;
										break;
								case 0xF: cptr->mVolume <<= 1;
										break;
							};
							if (cptr->mVolume > 64) 
                            {
								cptr->mVolume = 64;
                            }
							if (cptr->mVolume < 0)  
                            {
								cptr->mVolume = 0;
                            }

							vcptr->mNoteControl |= FMUSIC_VOLUME;
						}
						vcptr->mNoteControl |= FMUSIC_PAN;
						vcptr->mNoteControl |= FMUSIC_TRIGGER;
					}
				}
				cptr->mRetrigCount ++;
				break;
			}
			case FMUSIC_IT_TREMOLO : 
			{
				if (!mTick)
				{
					if (paramx) 
                    {
						cptr->mTremoloSpeed = paramx;
                    }
					if (paramy) 
                    {
						cptr->mTremoloDepth = paramy;
                    }
				}
				
				if (vcptr->mBackground) 
                {
					break;
                }

				cptr->tremolo();

				break;
			}

			case FMUSIC_IT_SPECIAL :
			{
				if (current->mEffectParam) 
                {
                    cptr->mSpecialParam = current->mEffectParam;
                }

				paramx = cptr->mSpecialParam >> 4;			/* get effect param x */
				paramy = cptr->mSpecialParam & 0xF;			/* get effect param y */

				switch (paramx)
				{
					case FMUSIC_IT_NOTECUT: 
					{
						if (!mTick) 
                        {
							break;
                        }

						if (mTick==paramy || (mTick == 1 && !paramy)) 
						{
							cptr->mVolume = 0;
							vcptr->mNoteControl |= FMUSIC_STOP;
						}
						break;
					}
					case FMUSIC_IT_SETVIBRATOWAVE : 
					{
						if (!mTick)	
                        {
							cptr->mWaveControlVibrato = paramy & 3;
                        }
						break;
					}
					case FMUSIC_IT_SETTREMOLOWAVE : 
					{
						if (!mTick)	
                        {
							cptr->mWaveControlTremolo = paramy & 3;
                        }
						break;
					}
					case FMUSIC_IT_SETPANBRELLOWAVE : 
					{
						if (!mTick)	
                        {
							cptr->mWaveControlPan = paramy & 3;
                        }
						break;
					}
					case FMUSIC_IT_PATTERNDELAYTICKS :
					{
						if (!mTick) 
                        {
							mPatternDelayTicks = paramy;
                        }
						break;
					}
					case FMUSIC_IT_S7SPECIAL :
					{
						switch (paramy)
						{
							case FMUSIC_IT_PASTNOTECUT :
							{
								if (!mTick && mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
								{
									MusicVirtualChannel *childvcptr;
									
                                    for (childvcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext(); childvcptr != &cptr->mVirtualChannelHead; childvcptr = (MusicVirtualChannel *)childvcptr->getNext())
									{
										if (childvcptr->mBackground) 
                                        {
											childvcptr->mNoteControl |= FMUSIC_STOP;
                                        }
                                    }
								}
								break;
							}
							case FMUSIC_IT_PASTNOTEOFF :
							{
								if (!mTick && mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
								{
									MusicVirtualChannel *childvcptr;

									for (childvcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext(); childvcptr != &cptr->mVirtualChannelHead; childvcptr = (MusicVirtualChannel *)childvcptr->getNext())
									{
										MusicInstrument *ciptr;

										if (childvcptr->mBackground)
										{
											ciptr = &mInstrument[childvcptr->mLastInstrument];
											childvcptr->mKeyOff = true;
											if (!(childvcptr->mVolType & FMUSIC_ENVELOPE_ON)) 
                                            {
												childvcptr->mFade = true;
                                            }
											else if (ciptr->mVolumeType & FMUSIC_ENVELOPE_LOOP) 
                                            {
												childvcptr->mFade = true;	/* undocumented */
                                            }
										}
									}
								}
								break;
							}
							case FMUSIC_IT_PASTNOTEFADE :
							{
								if (!mTick && mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
								{
									MusicVirtualChannel *childvcptr;
									
									for (childvcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext(); childvcptr != &cptr->mVirtualChannelHead; childvcptr = (MusicVirtualChannel *)childvcptr->getNext())
                                    {
										if (childvcptr->mBackground) 
                                        {
											childvcptr->mFade = true;
                                        }
                                    }
								}
								break;
							}

							case FMUSIC_IT_SETNNACUT :
							{
								if (!mTick && mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
								{
									vcptr->mNNA = FMUSIC_CUT;
								}
								break;
							}
							case FMUSIC_IT_SETNNACONTINUE :
							{
								if (!mTick && mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
								{
									vcptr->mNNA = FMUSIC_CONTINUE;
								}
								break;
							}
							case FMUSIC_IT_SETNNANOTEOFF :
							{
								if (!mTick && mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
								{
									vcptr->mNNA = FMUSIC_OFF;
								}
								break;
							}
							case FMUSIC_IT_SETNNANOTEFADE :
							{
								if (!mTick && mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
								{
									vcptr->mNNA = FMUSIC_FADEOUT;
								}
								break;
							}

							case FMUSIC_IT_VOLENVELOPEOFF :
							{
								if (!mTick && mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
								{
									vcptr->mVolType &= ~FMUSIC_ENVELOPE_ON;
								}
								break;
							}
							case FMUSIC_IT_VOLENVELOPEON :
							{
								if (!mTick && mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
								{
									vcptr->mVolType |= FMUSIC_ENVELOPE_ON;
								}
								break;
							}
						}
						break;
					}
					case FMUSIC_IT_SETPANPOSITION16:
					{
						if (!mTick)
						{
							cptr->mPan = paramy<<2;

							if (vcptr->mBackground) 
                            {
								break;
                            }

							vcptr->mPan = cptr->mPan;
							vcptr->mNoteControl |= FMUSIC_PAN;
							vcptr->mNoteControl |= FMUSIC_SURROUNDOFF;
						}
						break;
					}
					case FMUSIC_IT_S9SPECIAL:
					{
						if (!mTick)
						{
							if (vcptr->mBackground) 
                            {
								break;
                            }

    						switch (paramy)
                            {
                                case FMUSIC_IT_DISABLESURROUND:
                                {
							        vcptr->mNoteControl |= FMUSIC_SURROUNDOFF;
                                    break;
                                }
                                case FMUSIC_IT_ENABLESURROUND:
                                {
							        vcptr->mNoteControl |= FMUSIC_SURROUND;
                                    break;
                                }
                                case FMUSIC_IT_PLAYFORWARD:
                                {
							        vcptr->mNoteControl |= FMUSIC_FREQ;
                                    vcptr->mDirection = 1;
                                    break;
                                }
                                case FMUSIC_IT_PLAYBACKWARD:
                                {
							        vcptr->mNoteControl |= FMUSIC_FREQ;
                                    vcptr->mDirection = -1;
                                    break;
                                }
                            };
						}
						break;
					}
					case FMUSIC_IT_SETHIGHOFFSET :
					{
						if (!mTick)
						{
							cptr->mHighOffset = paramy;
						}
						break;
					}
					case FMUSIC_IT_PATTERNLOOP :
					{
						if (!mTick)
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
						}
						break;
					}
					case FMUSIC_IT_PATTERNDELAY : 
					{
						if (!mTick)
						{
							mPatternDelay = paramy;
							mPatternDelay *= mSpeed;
						}
						break;
					}
				};
				break;
			}
			case FMUSIC_IT_SETTEMPO : 
			{
				if (!mTick && current->mEffectParam > 0x1f)
				{
					setBPM(current->mEffectParam);
				}
				else if (current->mEffectParam <= 0x1f)
				{
					if (paramx == 0)
                    {
						setBPM(mBPM - paramy);
                    }
					else if (paramy == 1)
                    {
						setBPM(mBPM + paramy);
                    }
				}
				break;
			}
			case FMUSIC_IT_FINEVIBRATO :
			{
				if (!mTick)
				{
					if (paramx) 
                    {
						cptr->mVibSpeed = paramx;
                    }
					if (paramy) 
                    {
						cptr->mVibDepth = paramy;
                    }

					if (paramy) 
                    {
						cptr->mVibType = FMUSIC_IT_FINEVIBRATO;
                    }

					if (vcptr->mBackground) 
                    {
						break;
                    }

					if (!(mMusicFlags & FMUSIC_ITFLAGS_OLD_IT_EFFECTS)) 
					{
						if (cptr->mVibType == FMUSIC_IT_FINEVIBRATO) 
                        {
							cptr->fineVibrato();
                        }
						else 
                        {
							cptr->vibrato();
                        }
					}
				}
				else 
				{
					if (vcptr->mBackground) 
                    {
						break;
                    }

					if (cptr->mVibType == FMUSIC_IT_FINEVIBRATO) 
                    {
						cptr->fineVibrato();
                    }
					else 
                    {
						cptr->vibrato();
                    }
				}

				break;
			}
			case FMUSIC_IT_SETGLOBALVOLUME : 
			{
				if (!mTick)
				{
					mGlobalVolume = current->mEffectParam;
					if (mGlobalVolume > 128) 
                    {
						mGlobalVolume = 128;
                    }
				}
				break;
			}
			case FMUSIC_IT_GLOBALVOLUMESLIDE : 
			{
				if (!mTick)
				{
					/*
                        Wxy - Volume slide, fine vol  WFx = slide down, WxF = slide up
                    */
					if (current->mEffectParam) 
                    {
						mGlobalVolumeSlide = current->mEffectParam;
                    }

					/*
                        WFF is classed as a slide up so it gets priority
                    */
					if ((mGlobalVolumeSlide &0xF) == 0xF) 
                    {
						mGlobalVolume += (mGlobalVolumeSlide >> 4);
                    }
					else if ((mGlobalVolumeSlide >>4 ) == 0xF) 
                    {
						mGlobalVolume -= (mGlobalVolumeSlide & 0xF);
                    }
				}
				else 
				{
					if (!(mGlobalVolumeSlide & 0xF)) 
                    {
						mGlobalVolume += (mGlobalVolumeSlide >> 4);
                    }
					if (!(mGlobalVolumeSlide >>  4)) 
                    {
						mGlobalVolume -= (mGlobalVolumeSlide & 0xF);
                    }
				}
				
				if (mGlobalVolume > 128) 
                {
					mGlobalVolume = 128;
                }
				if (mGlobalVolume < 0)  
                {
					mGlobalVolume = 0;
                }

				break;				
			}
			case FMUSIC_IT_SETPAN : 
			{
				if (!mTick)
				{
					cptr->mPan = current->mEffectParam >> 2;

					if (vcptr->mBackground) 
                    {
						break;
                    }

					vcptr->mPan = cptr->mPan;
					vcptr->mNoteControl |= FMUSIC_PAN;
					vcptr->mNoteControl |= FMUSIC_SURROUNDOFF;
				}
				break;
			}
			case FMUSIC_IT_PANBRELLO : 
			{
				if (!mTick)
				{
					if (paramx) 
                    {
						cptr->mPanbrelloSpeed = paramx;
                    }
					if (paramy) 
                    {
						cptr->mPanbrelloDepth = paramy;
                    }
				}
				
				if (vcptr->mBackground) 
                {
					break;
                }

				cptr->panbrello();

				break;
			}
			case FMUSIC_IT_MIDIMACROS :
			{
//				if (!mTick)
//				{
//					FMUSIC_CheckCallback(mod, FMUSIC_CALLBACK_ZXX, current->mEffectParam);
//				}
				break;
			}
		};
	}

    if (!audible)
    {
        return FMOD_OK;
    }

	for (count=0; count < mNumChannels; count++)
	{
        bool filter;
		MusicChannel        *cptr  = 0;
		MusicVirtualChannel	*vcptr = 0;

		if (mDefaultPan[count] & 128) 
        {
			continue;
        }

		/*
            GET MUSIC CURRENT CHANNEL POINTER
        */
		cptr = (MusicChannelIT *)mMusicChannel[count];

        vcptr = (MusicVirtualChannel *)cptr->mVirtualChannelHead.getNext();
        while (vcptr != &cptr->mVirtualChannelHead)
		{
			int              volumedelta;
			MusicInstrument	*iptr = 0;
            MusicVirtualChannel *next = (MusicVirtualChannel *)vcptr->getNext();

			if (!vcptr->mSample) 
            {
				vcptr->mNoteControl &= ~FMUSIC_TRIGGER;
            }
			
			if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
			{
				iptr = &mInstrument[vcptr->mLastInstrument];

				/*
                    PROCESS ENVELOPES
                */
				if (vcptr->mVolType & FMUSIC_ENVELOPE_ON)
				{
					if (!vcptr->mEnvVolume.mStopped) 
                    {
                        processEnvelope(&vcptr->mEnvVolume, vcptr, iptr->mVolumeNumPoints, (MusicEnvelopeNode *)iptr->mVolumePoints, iptr->mVolumeType, iptr->mVolumeLoopStart, iptr->mVolumeLoopEnd, iptr->mVolumeSustainLoopStart, iptr->mVolumeSustainLoopEnd, FMUSIC_VOLUME);
                    }
					else 
					{
						vcptr->mFade = true;
						if (vcptr->mEnvVolume.mStopped && !vcptr->mEnvVolume.mValue) 
                        {
							vcptr->mNoteControl |= FMUSIC_STOP;
                        }
					}
				}

				if (iptr->mPanType & FMUSIC_ENVELOPE_ON) 
				{
					if (!vcptr->mEnvPan.mStopped) 
                    {
                        processEnvelope(&vcptr->mEnvPan, vcptr, iptr->mPanNumPoints, (MusicEnvelopeNode *)iptr->mPanPoints, iptr->mPanType, iptr->mPanLoopStart, iptr->mPanLoopEnd, iptr->mPanSustainLoopStart, iptr->mPanSustainLoopEnd, FMUSIC_PAN);
                    }
				}

				if (iptr->mPitchType & FMUSIC_ENVELOPE_ON) 
				{
					if (!vcptr->mEnvPitchStopped) 
                    {
						processPitchEnvelope(vcptr, iptr, cptr->mNote);
                    }
				}

				/*
                    PROCESS VOLUME FADEOUT
                */
				if (vcptr->mFade) 
				{
					vcptr->mFadeOutVolume -= iptr->mVolumeFade;
					if (vcptr->mFadeOutVolume < 0) 
                    {
						vcptr->mFadeOutVolume = 0;
                    }

					if (vcptr->mFadeOutVolume) 
                    {
						vcptr->mNoteControl |= FMUSIC_VOLUME;
                    }
					else 
                    {
						vcptr->mNoteControl |= FMUSIC_STOP;
                    }
				}
            }

				/*
                    SAMPLE VIBRATO
                */
				sampleVibrato(vcptr);	/* this gets added to previous freqdeltas */
			

			if (!(vcptr->mFrequency + vcptr->mFrequencyDelta)) 
            {
				vcptr->mNoteControl &= ~FMUSIC_FREQ;	/* divide by 0 check */
            }

			/*
                set up volume values
            */
			if (vcptr->mBackground) 
            {
				volumedelta = 0;
            }
			else 
			{
				volumedelta = cptr->mVolumeDelta;
				vcptr->mVolume = cptr->mVolume;
			}

			if (vcptr->mNoteControl & FMUSIC_TRIGGER)
			{
                FMOD_RESULT   result;
                bool          filter = false;
                SNDMIXPLUGIN *plugin = 0;
            
                if (iptr)
                {
                    if (iptr->mFilterCutOff & 128)
                    {
                        filter = true;
                    }
                 
                    if (!(iptr->mPitchType & FMUSIC_ENVELOPE_FILTER) && iptr->mFilterCutOff == 0xFF)
                    {
                        filter = false;
                    }

                    if (iptr->mMIDIOutput & 128)
                    {
                        plugin = mMixPlugin[(iptr->mMIDIOutput & 127) - 1];
                    }
                }

                if (mChannelPlugin[count])
                {
                    plugin = mMixPlugin[mChannelPlugin[count] - 1];
                }

                if (plugin && plugin->Info.dwInputRouting & 0x1)
                {
                    plugin = 0; /* It is already in as the final mix, don't add it again here */
                }

                result = playSound(vcptr->mSample, vcptr, filter, plugin);

                /*
                    If a negative frequency is selected, according to MPT it should start from the end.
                */
                if (vcptr->mDirection < 0)
                {
                    vcptr->mChannel.setPosition(vcptr->mSample->mSound->mLength - 1, FMOD_TIMEUNIT_PCM);
                }
			}

			if (vcptr->mNoteControl & FMUSIC_VOLUME)	
			{
                float finalvol;
				
				if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)	/* bugfix (fmod 2.21) */
				{
            		finalvol = (float)vcptr->mEnvVolume.mValue;		/*  6 bits (  64) */
					finalvol *= (vcptr->mVolume+volumedelta);		/*  6 bits (  64) */
					finalvol *= vcptr->mFadeOutVolume;				/* 10 bits (1024) */
					finalvol *= vcptr->mSampGlobalVol;				/*  6 bits (  64) */
					finalvol *= cptr->mGlobalVolume;				/*  6 bits (  64) */
					finalvol *= mGlobalVolume;					    /*  7 bits ( 128) */
					finalvol *= mMasterVolume;					    /*  7 bits ( 128) */
															    /* ============== */
																/* 48 bits        */			
					/* 
                        Any half arsed compiler will convert this into 1 constant at compile time.
                    */
					finalvol *= (1.0f / (64.0f * 64.0f * 1024.0f * 64.0f * 64.0f * 128.0f * 128.0f) * 0.5f);
				}
				else
				{
					finalvol = (float)(vcptr->mVolume+volumedelta);		/*  6 bits (  64) */
					finalvol *= vcptr->mSampGlobalVol;				    /*  6 bits (  64) */
					finalvol *= cptr->mGlobalVolume;					/*  6 bits (  64) */
					finalvol *= mGlobalVolume;					        /*  7 bits ( 128) */
					finalvol *= mMasterVolume;					        /*  7 bits ( 128) */
																    /* ============== */
																    /* 32 bits        */
					/*
                        Any half arsed compiler will convert this into 1 constant at compile time.
                    */
					finalvol *= (1.0f / (64.0f * 64.0f * 64.0f * 128.0f * 128.0f) * 0.5f);
				}

        	    vcptr->mChannel.setVolume(finalvol);
			}
			if (vcptr->mNoteControl & FMUSIC_PAN)
			{
				int finalpan;
				
				if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
                {
					finalpan = (vcptr->mPan + vcptr->mPanDelta + vcptr->mEnvPan.mValue);
                }
				else
                {
					finalpan = (vcptr->mPan + vcptr->mPanDelta);
                }

        	    vcptr->mChannel.setPan(((float)finalpan / 31.5f) - 1.0f);
			}
			if (vcptr->mNoteControl & FMUSIC_FREQ)
			{
				int finalfreq;
				
				if (mMusicFlags & FMUSIC_ITFLAGS_INSTRUMENTS)
                {
                    if (iptr->mPitchType & FMUSIC_ENVELOPE_FILTER)
                    {                          
					    finalfreq = vcptr->mFrequency + vcptr->mFrequencyDelta;
                    }
                    else
                    {
					    finalfreq = vcptr->mFrequency + vcptr->mFrequencyDelta - vcptr->mEnvPitch;
                    }
                }
				else
                {
					finalfreq = vcptr->mFrequency + vcptr->mFrequencyDelta;
                }

				if (mMusicFlags & FMUSIC_ITFLAGS_LINEARFREQUENCY) 
                {
                    finalfreq = FMUSIC_ITLINEARPERIOD2HZ(finalfreq);
                }
				else
                {
					finalfreq = period2HZ(finalfreq);
                }
                
                if (vcptr->mDirection < 0)
                {
                    finalfreq = -finalfreq;
                }

                vcptr->mChannel.setFrequency((float)finalfreq);
			}

            filter = false;

            if (iptr)
            {
                if (iptr->mFilterCutOff & 128)
                {
                    filter = true;
                }
                if (!(iptr->mPitchType & FMUSIC_ENVELOPE_FILTER) && iptr->mFilterCutOff == 0xFF)
                {
                    filter = false;
                }
            }

            if (filter)
            {
	            float Fc;
                float cutoffhz, resonance;
                int modifier = 256;
                
                if (iptr->mPitchType & FMUSIC_ENVELOPE_FILTER)
                {
                    modifier = vcptr->mEnvPitch / 4;
                }

	            if (mMusicFlags & FMUSIC_ITFLAGS_EXTENDFILTERRANGE)
                {
		            Fc = 110.0f * FMOD_POW(2.0f, 0.25f + ((float)((iptr->mFilterCutOff & 127) * (modifier+256)))/(20.0f*512.0f));
                }
	            else
                {
		            Fc = 110.0f * FMOD_POW(2.0f, 0.25f + ((float)((iptr->mFilterCutOff & 127) *(modifier+256)))/(24.0f*512.0f));
                }
	            
                cutoffhz = Fc;
	            if (cutoffhz < 120) 
                {
                    cutoffhz = 120;
                }
	            if (cutoffhz > 20000) 
                {
                    cutoffhz = 20000;
                }
	            if (cutoffhz * 2 > waveformat[0].frequency) 
                {
                    cutoffhz = (float)waveformat[0].frequency / 2.0f;
                }

                resonance = (float)(iptr->mFilterResonance & 127);

                mLowPass[vcptr->mChannel.mIndex]->setParameter(FMOD_DSP_ITLOWPASS_CUTOFF, cutoffhz);
                mLowPass[vcptr->mChannel.mIndex]->setParameter(FMOD_DSP_ITLOWPASS_RESONANCE, resonance);
            }

			if (vcptr->mNoteControl & FMUSIC_SURROUND)
            {
//				FMOD_SetSurround(channel, true);
            }
			if (vcptr->mNoteControl & FMUSIC_SURROUNDOFF)	
            {
//				FMOD_SetSurround(channel, false);
            }

			if (vcptr->mNoteControl & FMUSIC_STOP)		
			{
                if (mLowPass)
                {
                    mLowPass[vcptr->mChannel.mIndex]->remove();
                }
	            vcptr->mChannel.stopEx(false, false, true, true, false, false, false);
        	    vcptr->mSampleOffset = 0;	/* if this channel gets stolen it will be safe */
			}

			vcptr->mNoteControl = 0;
			vcptr->cleanUp();

            vcptr = next;
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
// VERIFY: 1c5eba88 manual
// OFFSET: 0x009008D0
FMOD_RESULT CodecIT::update(bool audible)
{
	if (mTick == 0)		/* new note */
	{
//        if (mFinished && !mLooping)
//        {
//            stop();
//        }
//        else
        {
		    /*
                Process any rows commands to set the next order/row
            */
		    if (mNextOrder >= 0) 
		    {
			    mOrder = mNextOrder;

//			    FMUSIC_CheckCallback(mod, FMUSIC_CALLBACK_ORDER, (unsigned char)mOrder);
    		    if (mNextOrder >= 0) 
	    	    {
		    	    mOrder = mNextOrder;
                }

			    /*
                    Skip any 'skip to next order' patterns
                */
			    while (mOrderList[mOrder] == 254) 
			    {
                    mOrder++;

                    if (mOrder >= mNumOrders)
                    {
                        if (!mLooping)
                        {
                            stop();
                        }
			            mOrder = mRestart;
                    }
			    }
			    if (mOrderList[mOrder] == 255)
			    {
				    mOrder = mRestart;
    #ifdef FMUSIC_PLAYPACKED
				    mPatternPtr = (signed char *)mPattern[mOrderList[mOrder]].mData;
    #endif
			    }
		    }

    #ifdef FMUSIC_PLAYPACKED
		    if ((mNextRow >=0 && mNextRow != mRow + 1) || mNextOrder >= 0)
		    {
			    int count;	
			    
			    /*
                    Reset pointer to start of pattern data
                */
			    mPatternPtr = (signed char *)mPattern[mOrderList[mOrder]].mData;

			    for (count=0; count < mNextRow; count++)
                {
				    unpackRow();
                }
		    }
    #endif


		    if (mNextRow >= 0)   
		    {
			    mRow = mNextRow;

//			    FMUSIC_CheckCallback(mod, FMUSIC_CALLBACK_ROW, (unsigned char)mRow);
    		    if (mNextRow >= 0)   
	    	    {
		    	    mRow = mNextRow;
                }

    #ifdef FMUSIC_PLAYPACKED
			    unpackRow();
    #endif
            }

  		    mNextOrder = -1;
  		    mNextRow = -1;
		    updateRow(audible);					/* Update and play the note */

		    /*
                If there were no row commands
            */
		    if (mNextRow == -1)
		    {
			    mNextRow = mRow+1;
			    if (mNextRow >= mPattern[mOrderList[mOrder]].mRows)	/* if end of pattern  */
			    { 
				    mNextOrder = mOrder+1;			/* so increment the order */
				    if (mNextOrder >= mNumOrders) 
				    {
					    mNextOrder = mRestart;
				    }
				    mNextRow = 0;						/* start at top of pattn */
			    }
		    }

//		    FMUSIC_CheckCallback(mod, FMUSIC_CALLBACK_ROW, (unsigned char)mRow);
        }
	}
	else 
    {
		updateRow(audible);						/* Else update the inbetween row effects */
    }

	mTick++;
	if (mTick >= mSpeed + mPatternDelay + mPatternDelayTicks)
	{
		mPatternDelay = 0;
		mPatternDelayTicks = 0;
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
// VERIFY: 78a76b52 manual
// OFFSET: 0x00900AD0
FMOD_RESULT CodecIT::play()
{
    MusicSong::play();

#ifdef FMUSIC_PLAYPACKED
    int pattern;

    do
    {
        pattern = mOrderList[mOrder];
        if (pattern >= mNumPatternsMem)
        {
            mOrder++;
        }
    }
    while (pattern >= mNumPatternsMem && mOrder < mNumOrders && mOrder < 255);

    if (pattern < mNumPatternsMem)
    {
        mPatternPtr = (signed char *)mPattern[pattern].mData;
        unpackRow();
    }
    else
    {
		mFinished = true;
        mPlaying = false;
        return FMOD_ERR_FORMAT;
    }
#endif

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
// VERIFY: 88712690 manual
// OFFSET: 0x009013B0
FMOD_RESULT CodecIT::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
	int 			count, highestorder;
	char		    str[256];
	unsigned char	sampleformat[256];
	unsigned char  *buff = 0;
	unsigned int    largestsample = 0;
	unsigned short  filenumpatterns=0;
    unsigned int    filesize, filepos, lengthbytes;
	int 			totalpatternsize = 0;
    int             version;
	static int      ioffset[256], poffset[256], soffset[256], sdataoffset[256];
    bool            lowpassused = false;
    FMOD_RESULT     result = FMOD_OK;
    int             messageoffset;
    int             messagelength;

    if (!(mFile->mSeekable))
    {
        return FMOD_ERR_FORMAT;
    }

    init(FMOD_SOUND_TYPE_IT);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecIT::openInternal", "attempting to open as IT..\n"));

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
    result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

	result = mFile->read(str, 1, 4);
    if (result != FMOD_OK)
    {
        return result;
    }
	if (FMOD_strncmp(str, "IMPM", 4))
	{
		FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "CodecIT::openInternal", "'IMPM' etc ID check failed [%c%c%c%c]\n", str[0], str[1], str[2], str[3]));
		return FMOD_ERR_FORMAT;
	}
    
    result = mFile->getSize(&filesize);
    if (result != FMOD_OK)
    {
        return result;
    }

	/*
        Set a few default values for this format
    */
    for (count = 0; count < MUSIC_MAXCHANNELS; count++)
    {
        mMusicChannel[count] = 0;
    }
	mPattern                = 0;
	mMasterSpeed            = 1.0f;
	mDefaultSpeed			= 6;
	mDefaultBPM				= 125;
	mDefaultGlobalVolume    = 64;
	mNumPatterns			= 0;
	mRestart				= 0;
	mNumSamples		        = 0;
    if (userexinfo && userexinfo->maxpolyphony)
    {
        mNumChannels        = userexinfo->maxpolyphony;
    }
    else
    {
        mNumChannels        = 64;
    }
	mMasterVolume           = 128;
	mPatternPtr             = 0;

	FMOD_memset(mLastNote,               0 ,64);
	FMOD_memset(mLastNumber,             0 ,64);
	FMOD_memset(mLastVolume,             0 ,64);
	FMOD_memset(mLastEffect,             0 ,64);
	FMOD_memset(mLastEffectParam,        0 ,64);
	FMOD_memset(mPreviousMaskVariable,   0 ,64);
    FMOD_memset(sdataoffset,            -1, 256);
	
    mWaveFormatMemory = (FMOD_CODEC_WAVEFORMAT *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_WAVEFORMAT));
    if (!mWaveFormatMemory)
    {
        return FMOD_ERR_MEMORY;
    }
    
    waveformat = mWaveFormatMemory;
    waveformat[0].lengthbytes = lengthbytes;

    mSystem->getSoftwareFormat(&waveformat[0].frequency, 0, 0, 0, 0, 0);

    if (userexinfo && userexinfo->defaultfrequency)
    {
        waveformat[0].frequency = userexinfo->defaultfrequency;
    }

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecIT::openInternal", "read song information..\n"));

    /*
        Start loading data from the file.
    */
	result = mFile->read(mSongName, 1, 26);     /* read in module name. */
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->seek(2, SEEK_CUR);					/* xx                   */
    if (result != FMOD_OK)
    {
        return result;
    }
	
    result = mFile->getWord(&mNumOrders);   	    /* num/orders           */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getWord(&mNumInstruments);	        /* num/instruments      */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getWord(&mNumSamples);          /* num/samples          */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getWord(&filenumpatterns);	        /* num/patterns         */
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->seek(2, SEEK_CUR);					/* cwt                  */
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mFile->getWord(&version);		            /* cmwt                 */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getWord(&mMusicFlags);               /* Flags                */
    if (result != FMOD_OK)
    {
        return result;
    }                                                                

	result = mFile->seek(2, SEEK_CUR);					        /* Special              */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getByte(&mDefaultGlobalVolume    	    /* Global volume        */);
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getByte(&mMasterVolume);	                /* Master/mixing volume */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getByte(&mDefaultSpeed);                    /* Default speed        */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getByte((unsigned int *)&mDefaultBPM);      /* Default bpm          */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getByte();                                  /* panning seperation (1 byte)  */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getByte();                                  /* 0                  (1 byte)  */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getWord(&messagelength);                   /* message length     (2 bytes) */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getDword(&messageoffset);                  /* message offset     (4 bytes) */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->getDword();                                /* reserved           (4 bytes) */
    if (result != FMOD_OK)
    {
        return result;
    }    
	result = mFile->read(mDefaultPan, 1, 64);               /* Default pan          */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->read(&mDefaultVolume, 1, 64);            /* Default volume       */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->read(&mOrderList, 1, mNumOrders);	/* Orderlist        */
    if (result != FMOD_OK)
    {
        return result;
    }

	highestorder = 0;
	for (count=0; count<mNumOrders; count++) 
	{
		if (mOrderList[count] != 255 && count > highestorder) 
        {
			highestorder = count;
        }
	}
	mNumOrders = highestorder+1;

	result = mFile->read(&ioffset, 4, mNumInstruments);	       /* Instrument data offset */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->read(&soffset, 4, mNumSamples);        /* Sample data offset */
    if (result != FMOD_OK)
    {
        return result;
    }
	result = mFile->read(&poffset, 4, filenumpatterns);        /* Pattern data offset */
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        IT EXTRA INFO
    */
    {
        int numfilters;

        result = mFile->getWord(&numfilters);
        if (result != FMOD_OK)
        {
            return result;
        }
        
        result = mFile->seek(numfilters * 8, SEEK_CUR);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    result = mFile->tell(&filepos);    
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        SKIP MIDI CONFIG
    */
    if ((!messageoffset || filepos < (unsigned int)messageoffset) && 
        (!ioffset[0]    || filepos < (unsigned int)ioffset[0]) && 
        filepos < (unsigned int)soffset[0] && 
        filepos < (unsigned int)poffset[0])
    {
        if (mMusicFlags & FMUSIC_ITFLAGS_EMBEDMIDICFG)
        {
            result = mFile->seek(sizeof(MODMIDICFG), SEEK_CUR);
            if (result != FMOD_OK)
            {
                return result;
            }
	    }
    }

    result = mFile->tell(&filepos);    
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        PATTERN NAMES
    */
    FMOD_memset(str, 0, 4);

    if ((!messageoffset || filepos < (unsigned int)messageoffset) && 
        (!ioffset[0]    || filepos < (unsigned int)ioffset[0]) && 
        filepos < (unsigned int)soffset[0] && 
        filepos < (unsigned int)poffset[0])
    {
        result = mFile->read(str, 1, 4);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (!FMOD_strncmp(str, "PNAM", 4))
        {
            unsigned int length;

            result = mFile->getDword(&length);
            if (result != FMOD_OK)
            {
                return result;
            }

            result = mFile->seek(length, SEEK_CUR);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        else
        {
            result = mFile->seek(filepos, SEEK_SET);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
    }

    result = mFile->tell(&filepos);    
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        CHANNEL NAMES
    */
    if ((!messageoffset || filepos < (unsigned int)messageoffset) && 
        (!ioffset[0]    || filepos < (unsigned int)ioffset[0]) && 
        filepos < (unsigned int)soffset[0] && 
        filepos < (unsigned int)poffset[0])
    {
        result = mFile->read(str, 1, 4);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (!FMOD_strncmp(str, "CNAM", 4))
        {
            unsigned int length;

            result = mFile->getDword(&length);
            if (result != FMOD_OK)
            {
                return result;
            }

            result = mFile->seek(length, SEEK_CUR);
            if (result != FMOD_OK)
            {
                return result;
            }

            result = mFile->read(str, 1, 4);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        else
        {
            result = mFile->seek(filepos, SEEK_SET);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
    }

    result = mFile->tell(&filepos);    
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        RUDIMENTARY CROSS PLATFORM MPT PLUGIN SUPPORT.
        CURRENTLY ONLY ECHO IS SUPPORTED, BUT HEY! ITS CROSS PLATFORM AND DOESN'T RELY ON DIRECTX!
    */
    while ((!messageoffset || filepos < (unsigned int)messageoffset) && 
        (!ioffset[0]    || filepos < (unsigned int)ioffset[0]) && 
        filepos < (unsigned int)soffset[0] && 
        filepos < (unsigned int)poffset[0])
    {
        char id[4];
        unsigned int pluginsize;
        int pluginid;

        result = mFile->read(id, 1, 4);
        if (result != FMOD_OK)
        {
            return result;
        }
        result = mFile->getDword(&pluginsize);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (id[0] == 'C' && id[1] == 'H' && id[2] == 'F' && id[3] == 'X')
        {
			for (count = 0; count < 64; count++) 
            {
                if (count * 4 < (int)pluginsize)
			    {
				    result = mFile->getDword(&mChannelPlugin[count]);
                    if (result != FMOD_OK)
                    {
                        return result;
                    }
			    }
            }
        }
        else if (id[0] == 'E' && id[1] == 'Q' && id[2] == 'F' && id[3] == 'X')
		{
            result = mFile->seek(pluginsize, SEEK_CUR);
            if (result != FMOD_OK)
            {
                return result;
            }
		} 
    	else 
        {
            if (id[0] != 'F' || id[1] != 'X' || id[2] < '0' || id[3] < '0')
            {
                break;
            }

		    pluginid = (id[2]-'0') * 10 + (id[3]-'0');

            /*
                If we have a valid plugin, attempt to load its data.
            */
		    if ((pluginid < MAX_MIXPLUGINS) && (pluginsize >= sizeof(SNDMIXPLUGININFO)+4))
		    {
                char *plugindata;
			    unsigned int dwExtra;
                SNDMIXPLUGININFO *plugininfo;
    
                plugindata = (char *)FMOD_Memory_Calloc(pluginsize);
                if (!plugindata)
                {
                    return FMOD_ERR_INVALID_PARAM;
                }

                result = mFile->read(plugindata, 1, pluginsize);
                if (result != FMOD_OK)
                {
                    return result;
                }

                plugininfo = (SNDMIXPLUGININFO *)plugindata;

			    dwExtra = *(unsigned int *)(plugindata+sizeof(SNDMIXPLUGININFO));

                #ifdef PLATFORM_ENDIAN_BIG
                dwExtra = FMOD_SWAPENDIAN_DWORD(dwExtra);
                #endif

                if (!FMOD_strncmp(plugininfo->szLibraryName, "Echo", 4) && dwExtra == sizeof(SNDMIXPLUGINDATA_ECHO))
                {		
                    if (!mMixPlugin[pluginid])
                    {
                        mMixPlugin[pluginid] = (SNDMIXPLUGIN *)FMOD_Memory_Calloc(sizeof(SNDMIXPLUGIN));
                        if (!mMixPlugin[pluginid])
                        {
                            return FMOD_ERR_MEMORY;
                        }
                    }

                    new (&mMixPlugin[pluginid]->mChannelGroup) ChannelGroupI();

                    mMixPlugin[pluginid]->Info = *plugininfo;

                    #ifdef PLATFORM_ENDIAN_BIG
                    mMixPlugin[pluginid]->Info.dwInputRouting = FMOD_SWAPENDIAN_DWORD(mMixPlugin[pluginid]->Info.dwInputRouting);
                    mMixPlugin[pluginid]->Info.dwOutputRouting = FMOD_SWAPENDIAN_DWORD(mMixPlugin[pluginid]->Info.dwOutputRouting);
                    mMixPlugin[pluginid]->Info.dwPluginId1 = FMOD_SWAPENDIAN_DWORD(mMixPlugin[pluginid]->Info.dwPluginId1);
                    mMixPlugin[pluginid]->Info.dwPluginId2 = FMOD_SWAPENDIAN_DWORD(mMixPlugin[pluginid]->Info.dwPluginId2);
                    #endif

                    if (dwExtra && (dwExtra <= pluginsize - sizeof(SNDMIXPLUGININFO)-4))
			        {
                        DSPI *dsp;
                        SNDMIXPLUGINDATA_ECHO *echo = 0;

                        echo = (SNDMIXPLUGINDATA_ECHO *)FMOD_Memory_Calloc(dwExtra);
                        if (!echo)
                        {
                            return FMOD_ERR_MEMORY;
                        }
                				        
				        FMOD_memcpy(echo, plugindata+sizeof(SNDMIXPLUGININFO)+4, dwExtra);               

                        #ifdef PLATFORM_ENDIAN_BIG
                        FMOD_SWAPENDIAN_FLOAT(echo->mFeedback);
                        FMOD_SWAPENDIAN_FLOAT(echo->mLeftDelay);
                        FMOD_SWAPENDIAN_FLOAT(echo->mPanDelay);
                        FMOD_SWAPENDIAN_FLOAT(echo->mRightDelay);
                        FMOD_SWAPENDIAN_FLOAT(echo->mWetDryMix);
                        FMOD_SWAPENDIAN_FLOAT(echo->unknown);
                        #endif

                        mMixPlugin[pluginid]->mChannelGroup.mVolume  = 1.0f;

                        result = mSystem->createDSPByType(FMOD_DSP_TYPE_ITECHO, &mMixPlugin[pluginid]->mChannelGroup.mDSPHead);
                        if (result != FMOD_OK)
                        {
                            return result;
                        }

                        dsp = mMixPlugin[pluginid]->mChannelGroup.mDSPMixTarget = mMixPlugin[pluginid]->mChannelGroup.mDSPHead;
                        dsp->mDefaultFrequency = (float)waveformat[0].frequency;
                   
                        #ifdef PLATFORM_PS3
                        /*
                            This itecho is happening on the PPU, point it back to PPU address.
                        */
                        dsp->mDescription.read = DSPITEcho::readCallback;
                        #endif

                        result = dsp->setParameter(FMOD_DSP_ITECHO_WETDRYMIX, echo->mWetDryMix * 100.0f);
                        if (result != FMOD_OK)
                        {
                            return result;
                        }
                        result = dsp->setParameter(FMOD_DSP_ITECHO_FEEDBACK,  echo->mFeedback * 100.0f);
                        if (result != FMOD_OK)
                        {
                            return result;
                        }
                        result = dsp->setParameter(FMOD_DSP_ITECHO_LEFTDELAY, echo->mLeftDelay * 2000.0f);
                        if (result != FMOD_OK)
                        {
                            return result;
                        }
                        result = dsp->setParameter(FMOD_DSP_ITECHO_RIGHTDELAY, echo->mRightDelay * 2000.0f);
                        if (result != FMOD_OK)
                        {
                            return result;
                        }
                        result = dsp->setParameter(FMOD_DSP_ITECHO_PANDELAY, echo->mPanDelay);
                        if (result != FMOD_OK)
                        {
                            return result;
                        }

                        if (echo)
                        {
                            FMOD_Memory_Free(echo);
                        }
			        }
                }
            
                FMOD_Memory_Free(plugindata);
            }
            else
            {
                result = mFile->seek(pluginsize, SEEK_CUR);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }
        }
    }

    /*
        READ MESSAGE AS TAG
    */
    if (messagelength)
    {
        char *message;

		result = mFile->seek(messageoffset, SEEK_SET);
        if (result != FMOD_OK)
        {
            return result;
        }

        message = (char *)FMOD_Memory_Calloc(messagelength + 1);
        if (!message)
        {
            return FMOD_ERR_MEMORY;
        }

        result = mFile->read(message, messagelength, 1);
        if (result != FMOD_OK)
        {
            return result;
        }
        
        for (count = 0; count < messagelength; count++)
        {
            if (message[count] == 0xd)
            {
                message[count] = '\n';
            }
        }

        result = metaData(FMOD_TAGTYPE_FMOD, "Song message", message, messagelength, FMOD_TAGDATATYPE_STRING, false);
        if (result != FMOD_OK)
        {
            return result;
        }

        FMOD_Memory_Free(message);
    }

	/*
        ALLOCATE MUSIC CHANNELS
    */
    for (count = 0; count < mNumChannels; count++)
    {
        MusicChannelIT *cptr;

        mMusicChannel[count] = FMOD_Object_Calloc(MusicChannelIT);
        if (!mMusicChannel[count])
        {
            return FMOD_ERR_MEMORY;
        }
        cptr = (MusicChannelIT *)mMusicChannel[count];
        cptr->mModule = this;
    }

    result = metaData(FMOD_TAGTYPE_FMOD, "Number of channels", &mNumChannels, sizeof(mNumChannels), FMOD_TAGDATATYPE_INT, false);
    if (result != FMOD_OK)
    {
        return result;
    }

	/*
        LOAD INSTRUMENTS
    */

	/*
        Alloc instrument array
    */
	mInstrument = (MusicInstrument *)FMOD_Memory_Calloc((mNumInstruments) * sizeof(MusicInstrument));
	if (!mInstrument) 
	{
		return FMOD_ERR_MEMORY;
	}

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecIT::openInternal", "read instrument information..\n"));

	/*
        Load instrument information
    */
	for (count = 0; count < mNumInstruments; count++) 
	{
		MusicInstrument	*iptr;
		unsigned char    dat;
		int              count2;

		/*
            Point a pointer to that particular instrument
        */
		iptr = &mInstrument[count];

		result = mFile->seek(ioffset[count], SEEK_SET);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(str, 1, 4);
        if (result != FMOD_OK)
        {
            return result;
        }

		if (FMOD_strncmp(str, "IMPI", 4))
		{
            return FMOD_ERR_FORMAT;
		}

		result = mFile->seek(12, SEEK_CUR);					/* skip dos filename      */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte();							/* 0                      */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mNNA					/* NNA                    */);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mDupCheckType			/* duplicate check type   */);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mDupCheckAction		/* duplicate check action */);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getWord(&iptr->mVolumeFade);	    /* fade out value         */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mPitchPanSep				);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mPitchPanCenter	);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mGlobalVolume		);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mDefaultPan);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mVolumeVariation		);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mPanVariation		);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->seek(2, SEEK_CUR);					/* instrument version       */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mNumSamples);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte();							/* x                        */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(iptr->mName, 1, 26);			/* instrument name          */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mFilterCutOff);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (iptr->mFilterCutOff & 128)
        {
            lowpassused = true;
        }

		result = mFile->getByte(&iptr->mFilterResonance);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mMIDIOutput);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte();							/* midi program (instrument)*/
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->seek(2, SEEK_CUR);					/* xx                       */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(iptr->mNoteTable, 1, 240);      /* keymap layout            */
        if (result != FMOD_OK)
        {
            return result;
        }

		/*
            Volume envelope
        */
		iptr->mVolumeType = 0;
		result = mFile->getByte(&dat);
        if (result != FMOD_OK)
        {
            return result;
        }
		if (dat & 1) iptr->mVolumeType |= FMUSIC_ENVELOPE_ON;
		if (dat & 2) iptr->mVolumeType |= FMUSIC_ENVELOPE_LOOP;
		if (dat & 4) iptr->mVolumeType |= FMUSIC_ENVELOPE_SUSTAIN;
		if (dat & 8) iptr->mVolumeType |= FMUSIC_ENVELOPE_CARRY;
		result = mFile->getByte(&iptr->mVolumeNumPoints		);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mVolumeLoopStart		);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mVolumeLoopEnd		);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mVolumeSustainLoopStart	);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mVolumeSustainLoopEnd		);
        if (result != FMOD_OK)
        {
            return result;
        }

		for (count2=0; count2 < 25; count2++)
		{
            unsigned short val;
            MusicEnvelopeNode *v = (MusicEnvelopeNode *)&iptr->mVolumePoints;
			
			result = mFile->getByte(&v[count2].mValue);
            if (result != FMOD_OK)
            {
                return result;
            }
			result = mFile->getWord(&val);
            if (result != FMOD_OK)
            {
                return result;
            }
            v[count2].mTick = val;
		}

		result = mFile->getByte();
        if (result != FMOD_OK)
        {
            return result;
        }
		if (iptr->mVolumeNumPoints < 2) 
        {
            iptr->mVolumeType = FMUSIC_ENVELOPE_OFF;
        }

		/*
            Pan envelope
        */
		iptr->mPanType = 0;
		result = mFile->getByte(&dat);
        if (result != FMOD_OK)
        {
            return result;
        }
		if (dat & 1) iptr->mPanType |= FMUSIC_ENVELOPE_ON;
		if (dat & 2) iptr->mPanType |= FMUSIC_ENVELOPE_LOOP;
		if (dat & 4) iptr->mPanType |= FMUSIC_ENVELOPE_SUSTAIN;
		if (dat & 8) iptr->mPanType |= FMUSIC_ENVELOPE_CARRY;
		result = mFile->getByte(&iptr->mPanNumPoints);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mPanLoopStart);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mPanLoopEnd);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mPanSustainLoopStart	);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mPanSustainLoopEnd);
        if (result != FMOD_OK)
        {
            return result;
        }
		for (count2=0; count2 < 25; count2++)
		{
            unsigned short val;
            MusicEnvelopeNode *v = (MusicEnvelopeNode *)&iptr->mPanPoints;
			
			result = mFile->getByte(&v[count2].mValue);
            if (result != FMOD_OK)
            {
                return result;
            }
			result = mFile->getWord(&val);
            if (result != FMOD_OK)
            {
                return result;
            }
            v[count2].mTick = val;
		}
		result = mFile->getByte();
        if (result != FMOD_OK)
        {
            return result;
        }
		if (iptr->mPanNumPoints < 2) 
        {
            iptr->mPanType = FMUSIC_ENVELOPE_OFF;
        }

		/*
            Pitch envelope
        */
		iptr->mPitchType = 0;
		result = mFile->getByte(&dat);
        if (result != FMOD_OK)
        {
            return result;
        }
		if (dat & 1)	iptr->mPitchType |= FMUSIC_ENVELOPE_ON;
		if (dat & 2)	iptr->mPitchType |= FMUSIC_ENVELOPE_LOOP;
		if (dat & 4)	iptr->mPitchType |= FMUSIC_ENVELOPE_SUSTAIN;
		if (dat & 8)	iptr->mPitchType |= FMUSIC_ENVELOPE_CARRY;
		if (dat & 0x80) iptr->mPitchType |= FMUSIC_ENVELOPE_FILTER;

		result = mFile->getByte(&iptr->mPitchNumpoints	);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mPitchLoopStart	);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mPitchLoopEnd		);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mPitchSustainLoopStart);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&iptr->mPitchSustainLoopEnd	);
        if (result != FMOD_OK)
        {
            return result;
        }
		for (count2=0; count2 < 25; count2++)
		{
            unsigned short val;
            MusicEnvelopeNode *v = (MusicEnvelopeNode *)&iptr->mPitchPoints;
			
			result = mFile->getByte(&v[count2].mValue);
            if (result != FMOD_OK)
            {
                return result;
            }
			result = mFile->getWord(&val);
            if (result != FMOD_OK)
            {
                return result;
            }
            v[count2].mTick = val;
		}
		result = mFile->getByte();
        if (result != FMOD_OK)
        {
            return result;
        }
		if (iptr->mPitchNumpoints < 2) 
        {
            iptr->mPitchType = FMUSIC_ENVELOPE_OFF;
        }
	}


	/*
        LOAD SAMPLES
    */

	/*
        Alloc sample array
    */
	mSample = 0;
	if (mNumSamples)
	{
		mSample = (MusicSample **)FMOD_Memory_Calloc(mNumSamples * sizeof(MusicSample *));
		if (!mSample) 
		{
			return FMOD_ERR_MEMORY;
		}
	}

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecIT::openInternal", "read sample information..\n"));

	/*
        Load sample information
    */
	for (count=0; count < mNumSamples; count++) 
	{
		MusicSample *sptr;
        char                sample_name[26];
        FMOD_MODE           sample_mode;
        unsigned int        sample_length;
        FMOD_SOUND_FORMAT   sample_format;
        int                 sample_channels;
		
        mSample[count] = &mSampleMem[count];
		sptr = mSample[count];

		result = mFile->seek(soffset[count], SEEK_SET);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(str, 1, 4);
        if (result != FMOD_OK)
        {
            return result;
        }
		if (FMOD_strncmp(str, "IMPS", 4))
		{
			return FMOD_ERR_FORMAT;
		}
		result = mFile->seek(12, SEEK_CUR);			/* skip dos filename */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte();					/* 0 */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&sptr->mGlobalVolume 	/* global volume (0-64) */);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&sptr->mFlags);              /* flags */
        if (result != FMOD_OK)
        {
            return result;
        }
        
	    sample_mode     = FMOD_SOFTWARE | FMOD_2D;
        sample_channels = 1;

        if (sptr->mFlags & 2)
        {
            sample_format   = FMOD_SOUND_FORMAT_PCM16;
        }
        else
        {
            sample_format   = FMOD_SOUND_FORMAT_PCM8;
        }

		if (sptr->mFlags & 4) 
        {
			sample_channels = 2;
        }

		if (sptr->mFlags & 16) 
		{
			if (sptr->mFlags & 64) 
            {
                sample_mode |= FMOD_LOOP_BIDI;
            }
			else
            {
                sample_mode |= FMOD_LOOP_NORMAL;
            }
		}
        else
        {
   	        sample_mode |= FMOD_LOOP_OFF;
        }

		/* if (sptr->mFlags & 128) sptr-> */
		result = mFile->getByte(&sptr->mDefaultVolume 			/* default volume (0-64) */);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(sample_name, 1, 26);	    /* sample name           */
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

		result = mFile->getByte(&sampleformat[count] 	/* cvt                   */);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&sptr->mDefaultPan        	/* default pan (0-64)    */);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(&sample_length, 4, 1);	    /* length in samples     */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(&sptr->mLoopStart, 4, 1);   /* loop start            */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(&sptr->mLoopLength, 4, 1);	    /* loop end              */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(&sptr->mMiddleC, 4, 1);	    /* C5spd                 */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(&sptr->mSusLoopBegin, 4, 1);/* sustain loop begin    */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(&sptr->mSusLoopEnd, 4, 1);	/* sustain loop end      */
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->read(&sdataoffset[count], 4, 1);
        if (result != FMOD_OK)
        {
            return result;
        }

		result = mFile->getByte(&sptr->mVibSpeed);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&sptr->mVibDepth);
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&sptr->mVibRate );
        if (result != FMOD_OK)
        {
            return result;
        }
		result = mFile->getByte(&sptr->mVibType );
        if (result != FMOD_OK)
        {
            return result;
        }

		sptr->mLoopLength-=sptr->mLoopStart;

		if (!sptr->mMiddleC)
        {
			sptr->mMiddleC = 8363;
        }

		if (!sptr->mLoopLength || sample_mode & FMOD_LOOP_OFF) 
		{
			sptr->mLoopStart = 0;
			sptr->mLoopLength = sample_length;
			sample_mode &= ~FMOD_LOOP_NORMAL;
			sample_mode &= ~FMOD_LOOP_BIDI;
			sample_mode |= FMOD_LOOP_OFF;
		}

		/*
            ALLOCATE MEMORY FOR THE SAMPLE BUFFER
        */
        if (sample_length)
		{
            FMOD_CREATESOUNDEXINFO exinfo;
            unsigned int lenbytes;

            SoundI::getBytesFromSamples(sample_length, &lenbytes, sample_channels, sample_format);

            FMOD_memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));            
            exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
            exinfo.length           = lenbytes;
            exinfo.numchannels      = sample_channels;
            exinfo.defaultfrequency = sptr->mMiddleC;
            exinfo.format           = sample_format;

            result = mSystem->createSound(0, sample_mode | FMOD_OPENUSER, &exinfo, &sptr->mSound);
            if (result != FMOD_OK)
            {
                return result;
            }

            if (sample_mode & FMOD_LOOP_NORMAL || sample_mode & FMOD_LOOP_BIDI)
            {
                result = sptr->mSound->setLoopPoints(sptr->mLoopStart, FMOD_TIMEUNIT_PCM, sptr->mLoopStart + sptr->mLoopLength - 1, FMOD_TIMEUNIT_PCM);
                if (result != FMOD_OK)
                {
//                            return result;
                }
            }
		}
	}
    sdataoffset[count] = filesize;


	/*
        LOAD PATTERNS
    */

	/*
        Get the true number of patterns
    */
	mNumPatterns = 0;
	for (count=0; count<mNumOrders; count++)
	{
		if (mOrderList[count] < 254 && mOrderList[count] >= mNumPatterns) 
        {
            mNumPatterns = mOrderList[count]+1;
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
	
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecIT::openInternal", "read pattern information..\n"));

	/*
        Unpack and read patterns
    */
	for (count=0; count < filenumpatterns; count++) 
	{
		MusicPattern *pptr;
		unsigned short  rows;
#ifndef FMUSIC_PLAYPACKED
		unsigned short	row;
		unsigned char	channelvariable;
#endif

		pptr = &mPattern[count];

		if (!poffset[count]) 
		{
			/*
                Allocate memory for pattern buffer
            */
			pptr->mRows = 64;

#ifdef FMUSIC_PLAYPACKED
			pptr->mData = (MusicNote *)FMOD_Memory_Calloc(pptr->mRows);
#else
			pptr->mData = (MusicNote *)FMOD_Memory_Calloc(mNumChannels * pptr->mRows * sizeof(MusicNote));
#endif
			if (!pptr->mData)
			{
				return FMOD_ERR_MEMORY;
			}
		}
		else
		{
			unsigned short packedpatlen;

			result = mFile->seek(poffset[count], SEEK_SET);
            if (result != FMOD_OK)
            {
                return result;
            }
			result = mFile->getWord(&packedpatlen);	    /* length of packed pattern */
            if (result != FMOD_OK)
            {
                return result;
            }
			result = mFile->getWord(&rows);             /* length of pattern        */
            if (result != FMOD_OK)
            {
                return result;
            }
			result = mFile->seek(4, SEEK_CUR);			/* xxxx                     */
            if (result != FMOD_OK)
            {
                return result;
            }
			pptr->mRows = rows;

#ifdef FMUSIC_PLAYPACKED

			pptr->mData = (MusicNote *)FMOD_Memory_Calloc(packedpatlen);
			result = mFile->read(pptr->mData, 1, packedpatlen);
            if (result != FMOD_OK)
            {
                return result;
            }
			totalpatternsize += packedpatlen;
#else
			/* 
                this method uses 20k per pattern.. ow.. (255 patterns = 5.1mb!)
			    allocate memory for pattern buffer
            */
			pptr->mData = (MusicNote *)FMOD_Memory_Calloc(mNumChannels * pptr->mRows * sizeof(MusicNote));
			if (!pptr->mData)
			{
				FMOD_ErrorNo = FMOD_ERR_MEMORY;
				return false;
			}
			totalpatternsize += mNumChannels * pptr->mRows * sizeof(MusicNote);

			for (row=0; row < pptr->mRows; )
			{
				result = mFile->getByte(&channelvariable);
                if (result != FMOD_OK)
                {
                    return result;
                }

				if (channelvariable)
				{
					unsigned char channel, maskvariable;
					MusicNote *nptr;

					channel = (channelvariable-1) & 63;
					
					nptr = pptr->mData+(row*mNumChannels)+channel;

					if (channelvariable & 128) 
					{
						result = mFile->getByte(&maskvariable);
                        if (result != FMOD_OK)
                        {
                            return result;
                        }
						mPreviousMaskVariable[channel] = maskvariable;
					}
					else maskvariable = mPreviousMaskVariable[channel];

					if (maskvariable & 1) 
					{
						result = mFile->getByte(&unsigned char note);
                        if (result != FMOD_OK)
                        {
                            return result;
                        }
						
						if (note >= 254) 
                        {
							nptr->mNote = note;
                        }
						else 
                        {
							nptr->mNote = note + 1;
                        }

						mLastNote[channel] = nptr->mNote;
					}
					if (maskvariable & 2) 
					{
						result = mFile->getByte(&nptr->mNumber);
                        if (result != FMOD_OK)
                        {
                            return result;
                        }
						mLastNumber[channel] = nptr->mNumber;
					}
					if (maskvariable & 4) 
					{
						nptr->mVolume = FMOD_File_GetChar(fp) + 1;
						mLastVolume[channel] = nptr->mVolume;
					}
					if (maskvariable & 8) 
					{ 
						result = mFile->getByte(&nptr->mEffect);
                        if (result != FMOD_OK)
                        {
                            return result;
                        }
						result = mFile->getByte(&nptr->mEffectParam );
                        if (result != FMOD_OK)
                        {
                            return result;
                        }
						mLastEffect[channel] = nptr->mEffect;
						mLastEffectParam[channel] = nptr->mEffectParam;
					}
					if (maskvariable & 16)  nptr->mNote = mLastNote[channel];
					if (maskvariable & 32)  nptr->mNumber = mLastNumber[channel];
					if (maskvariable & 64)  nptr->mVolume = mLastVolume[channel];
					if (maskvariable & 128) 
					{ 
						nptr->mEffect = mLastEffect[channel]; 
						nptr->mEffectParam = mLastEffectParam[channel];
					}

				}
				else 
                {
					row++;
                }
			}
#endif
		}
	}

	/*
        Allocate and clean out any extra patterns 
    */
	if (mNumPatterns > filenumpatterns)
	{
		for (count=filenumpatterns; count < mNumPatterns; count++) 
		{			
			MusicPattern *pptr;

			pptr = &mPattern[count];
			pptr->mRows = 64;

			/*
                Allocate memory for pattern buffer
            */
#ifdef FMUSIC_PLAYPACKED
			pptr->mData = (MusicNote *)FMOD_Memory_Calloc(pptr->mRows);
#else
			pptr->mData = (MusicNote *)FMOD_Memory_Calloc(mNumChannels * pptr->mRows * sizeof(MusicNote));
#endif
			if (!pptr->mData)
			{
				return FMOD_ERR_MEMORY;
			}
		}
	}

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecIT::openInternal", "load sample data..\n"));

	/*
        LOAD SAMPLES
    */

	for (count = 0; count < mNumSamples; count++) 
	{
		MusicSample  *sptr = mSample[count];
		unsigned int srclenbytes = 0;
        
        if (count == 8)
        {
            count = count;
        }

        if (sdataoffset[count+1] > sdataoffset[count])
        {
            srclenbytes = sdataoffset[count+1] - sdataoffset[count];
        }
        else
        {
            if (sptr->mSound)
            {
                mSample[count]->mSound->getLength(&srclenbytes, FMOD_TIMEUNIT_PCMBYTES);
            }
        }

		if (sptr->mSound)
		{
            void             *ptr1, *ptr2;
            unsigned int      len1, len2;
            bool              signeddata = false;
            unsigned int      lenbytes;
            int               compression = 0;
            FMOD_SOUND_FORMAT format;

            mSample[count]->mSound->getLength(&lenbytes, FMOD_TIMEUNIT_PCMBYTES);

            sptr->mSound->getFormat(0, &format, 0, 0);

			if (sampleformat[count] & 1) 
            {
				signeddata = true;
            }

            #if 1
			if (sptr->mFlags & 8)	
            {
                compression = ((version >= 0x215) && (sampleformat[count] & 4)) ? 215 : 214;
            }
            #else
			if (version >= 0x0214)
            {
			    if (version == 0x0215) 
                {
                    if (sampleformat[count] & 4)
                    {
	    			    compression = 215;
                    }
                    else
                    {
                        compression = 214;
                    }
                }
                else
                {
                    compression = 214;
                }			
            }

            if (srclenbytes == lenbytes)
            {
                compression = 0;
            }
            #endif

			result = mFile->seek(sdataoffset[count], SEEK_SET);
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
                if (!compression && signeddata)
                {
                    if (format == FMOD_SOUND_FORMAT_PCM16)
                    {
                        if (sptr->mSound->mChannels == 1)
                        {
				            result = mFile->read(ptr1, 2, len1 >> 1);
                        }
                        else
                        {
                            unsigned int count2;
                            
                            /*
                                IT stereo samples are not interleaved!
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

                        if (result != FMOD_OK)
                        {
                            return result;
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
                                IT stereo samples are not interleaved!
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
                    }
                }
                else
                {
				    if (srclenbytes > largestsample)
				    {
					    if (buff) 
                        {
                            FMOD_Memory_Free(buff);
                        }

					    buff = (unsigned char *)FMOD_Memory_Alloc(srclenbytes);
					    if (!buff) 
					    {
						    return FMOD_ERR_MEMORY;
					    }
					    largestsample = srclenbytes;
				    }
                
                    FMOD_memset(buff, 0, largestsample);
				    result = mFile->read(buff, 1, srclenbytes);	        /*  if from file, load and then upload */
                    if (result != FMOD_OK)
                    {
                        return result;
                    }

                    if (format == FMOD_SOUND_FORMAT_PCM8)
                    {
                        int count2;
                        void *src = buff;
                        for (count2 = 0; count2 < sptr->mSound->mChannels; count2++)
                        {
			                decompress8(&src, (char *)ptr1 + count2, lenbytes / sptr->mSound->mChannels, compression == 215, sptr->mSound->mChannels);
                        }
                    }
		            else
                    {
                        int count2;
                        void *src = buff;
                        for (count2 = 0; count2 < sptr->mSound->mChannels; count2++)
                        {
    			            decompress16(&src, (short *)ptr1 + count2, lenbytes / 2 / sptr->mSound->mChannels, compression == 215, sptr->mSound->mChannels);
                        }
                    }
                }
            }

            result = sptr->mSound->unlock(ptr1, ptr2, len1, len2);
            if (result != FMOD_OK)
            {
                return result;
            }
		}
	}

	if (buff) 
    {
		FMOD_Memory_Free(buff);
    }

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
	mSrcDataOffset  = 0;

    SoundI::getBytesFromSamples(1, (unsigned int *)&waveformat[0].blockalign, waveformat[0].channels, waveformat[0].format);

    /*
        Create final target unit.
    */
    {
        FMOD_DSP_DESCRIPTION_EX descriptionex;
        
        FMOD_memset(&descriptionex, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));
        descriptionex.version       = 0x00010100;
        descriptionex.channels      = waveformat[0].channels;
        descriptionex.mFormat       = waveformat[0].format;
        descriptionex.mCategory     = FMOD_DSP_CATEGORY_SOUNDCARD;

        FMOD_strcpy(descriptionex.name, "FMOD IT final mixdown unit");
        result = mSystem->createDSP(&descriptionex, &mDSPFinalHead);
        if (result != FMOD_OK)
        {
            return result;
        }
        mDSPFinalHead->mDefaultFrequency = (float)waveformat[0].frequency;
        mDSPFinalHead->setActive(true);
    }

    /*
        Create mixdown unit that other effect units can connect to along side the mDSPHead.
    */
    {
        FMOD_DSP_DESCRIPTION_EX descriptionex;
        
        FMOD_memset(&descriptionex, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));
        descriptionex.version       = 0x00010100;
        descriptionex.mCategory     = FMOD_DSP_CATEGORY_FILTER;
        FMOD_strcpy(descriptionex.name, "FMOD IT global effect head unit");
        result = mSystem->createDSP(&descriptionex, &mDSPEffectHead);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = mDSPFinalHead->addInput(mDSPEffectHead);
        if (result != FMOD_OK)
        {
            return result;
        }

        mDSPEffectHead->mDefaultFrequency = (float)waveformat[0].frequency;
        mDSPEffectHead->setActive(true);
    }

    /*
        Create head units that software channels connect to.
    */
    {
        FMOD_DSP_DESCRIPTION_EX descriptionex;
        
        FMOD_memset(&descriptionex, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));
        descriptionex.version       = 0x00010100;
        descriptionex.mCategory     = FMOD_DSP_CATEGORY_FILTER;
        FMOD_strcpy(descriptionex.name, "FMOD IT default channel head unit");
        result = mSystem->createDSP(&descriptionex, &mDSPHead);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = mDSPEffectHead->addInput(mDSPHead);
        if (result != FMOD_OK)
        {
            return result;
        }

        mDSPHead->mDefaultFrequency = (float)waveformat[0].frequency;
        mDSPHead->setActive(true);
    }


    /*
        ADD DSP EFFECTS IF THEY EXIST.
    */

    for (count = 0; count < MAX_MIXPLUGINS; count++)
    {
        if (mMixPlugin[count] && mMixPlugin[count]->mChannelGroup.mDSPHead)
        {
            DSPI *effect;

            effect = mMixPlugin[count]->mChannelGroup.mDSPHead;

            /*
                Final mix or not.
            */
            if (mMixPlugin[count]->Info.dwInputRouting & 0x1)
            {
                DSPI *nexteffect;

                result = mDSPFinalHead->getInput(0, &nexteffect);
                if (result != FMOD_OK)
                {
                    return result;
                }

                result = mDSPFinalHead->disconnectFrom(nexteffect);
                if (result != FMOD_OK)
                {
                    return result;
                }

                result = mDSPFinalHead->addInput(effect);
                if (result != FMOD_OK)
                {
                    return result;
                }

                result = effect->addInput(nexteffect);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }
            else
            {
                result = mDSPEffectHead->addInput(effect);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }

            /*
                Bypass flag.
            */
            if (mMixPlugin[count]->Info.dwInputRouting & 0x2)
            {
                result = effect->setBypass(true);
                if (result != FMOD_OK)
                {
                    return result;
                }                   
            }

            /*
                Mix in the dry as well.
                This could be done by creating another dummy unit which just passes the dry signal into the mix along side the echo.
                Its pretty annoying, but i don't think many musicians are really using it.
            */
            if (mMixPlugin[count]->Info.dwInputRouting & 0x4)
            {
                // ????
            }

            result = effect->setActive(true);
            if (result != FMOD_OK)
            {
                return result;
            }                   
        }
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
    mNumRealChannels = mNumVirtualChannels * 2;

    mChannelPool = FMOD_Object_Calloc(ChannelPool);
    if (!mChannelPool)
    {
        return FMOD_ERR_MEMORY;
    }

    result = mChannelPool->init(mSystem, 0, mNumRealChannels);
    if (result != FMOD_OK)
    {
        return result;
    }

    mChannelSoftware = (ChannelSoftware *)FMOD_Memory_Calloc(sizeof(ChannelSoftware) * mNumRealChannels);
    if (!mChannelSoftware)
    {
        return FMOD_ERR_MEMORY;
    }

    for (count = 0; count < mNumRealChannels; count++)
    {
        new (&mChannelSoftware[count]) ChannelSoftware;
        mChannelPool->setChannel(count, &mChannelSoftware[count], mDSPHead);
        mChannelSoftware[count].allowReverb(false);
    }

    if (lowpassused)
    {
        mLowPass = (DSPI **)FMOD_Memory_Calloc(sizeof(DSPI *) * mNumRealChannels);
        if (!mLowPass)
        {
            return FMOD_ERR_MEMORY;
        }

        for (count = 0; count < mNumRealChannels; count++)
        {
            result = mSystem->createDSPByType(FMOD_DSP_TYPE_ITLOWPASS, &mLowPass[count]);
            if (result != FMOD_OK)
            {
                return result;
            }

            #ifdef PLATFORM_PS3
            /*
                This lowpass is happening on the PPU, point it back to PPU address.
            */
            mLowPass[count]->mDescription.read = DSPLowPass2::readCallback;
            #endif
        }
    }

    if ((usermode & FMOD_ACCURATETIME) || (usermode & FMOD_CREATESAMPLE))
    {
        /*
            Calculate the length of the song by quickly playing through it.
        */
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

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecIT::openInternal", "done.\n"));

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
// VERIFY: 14bb52fc auto
// OFFSET: 0x00900B40
FMOD_RESULT CodecIT::closeInternal()
{
    int count;

    stop();

    if (mSample)
    {
	    for (count = 0; count < mNumSamples; count++)
	    {
            if (mSample[count] && mSample[count]->mSound)
            {
                mSample[count]->mSound->release();
                mSample[count]->mSound = 0;
                mSample[count] = 0;
            }
        }

        FMOD_Memory_Free(mSample);
        mSample = 0;
    }

    if (mLowPass)
    {
        for (count = 0; count < mNumRealChannels; count++)
        {
            if (mLowPass[count])
            {
                mLowPass[count]->release();
            }
        }

        FMOD_Memory_Free(mLowPass);
        mLowPass = 0;
    }

    if (mInstrument)
    {
        FMOD_Memory_Free(mInstrument);
        mInstrument = 0;
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
	    for (count = 0; count < mNumPatternsMem; count++)
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

	for (count = 0; count < MAX_MIXPLUGINS; count++)
	{
        if (mMixPlugin[count])
        {
            mMixPlugin[count]->mChannelGroup.mDSPHead->release();

            FMOD_Memory_Free(mMixPlugin[count]);
        }
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

    if (mDSPFinalHead)
    {
        mDSPFinalHead->release();
        mDSPFinalHead = 0;
    }

    if (mDSPEffectHead)
    {
        mDSPEffectHead->release();
        mDSPEffectHead = 0;
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
// VERIFY: a556f635 manual
// OFFSET: 0x00900FC0
FMOD_RESULT CodecIT::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
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
                result = mDSPFinalHead->execute(outbuffer, &outbuffer, &read, numchannels, &numchannels, FMOD_SPEAKERMODE_STEREO_LINEAR);
                if (result != FMOD_OK)
                {
                    FMOD_OS_CriticalSection_Leave(crit);
                    return result;
                }

                mDSPFinalHead->resetVisited();
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
// VERIFY: 91f3e263 auto
// OFFSET: 0x00900E50
FMOD_RESULT CodecIT::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    if (postype == FMOD_TIMEUNIT_MODORDER)
    {
        play();
        mNextOrder = mOrder = position;

        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_PCM)
    {
        bool mRestarted = false;

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
            mRestarted = true;
        }

        while (mPCMOffset < position)
        {
            update(true);
        }

        if (mRestarted)
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
// VERIFY: 9a273d9c manual
// OFFSET: 0x009039D0
FMOD_RESULT F_CALLBACK CodecIT::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecIT *cit = (CodecIT *)codec;

    return cit->openInternal(usermode, userexinfo);
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
// VERIFY: 6e1332df manual
// OFFSET: 0x00900EF0
FMOD_RESULT F_CALLBACK CodecIT::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecIT *cit = (CodecIT *)codec;

    return cit->closeInternal();
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
// VERIFY: 8d8078cd auto
// OFFSET: 0x00901380
FMOD_RESULT F_CALLBACK CodecIT::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecIT *cit = (CodecIT *)codec;

    return cit->readInternal(buffer, sizebytes, bytesread);
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
// VERIFY: ec6e5506 auto
// OFFSET: 0x00900F20
FMOD_RESULT F_CALLBACK CodecIT::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecIT *cit = (CodecIT *)codec;

    return cit->setPositionInternal(subsound, position, postype);
}

}

#endif


