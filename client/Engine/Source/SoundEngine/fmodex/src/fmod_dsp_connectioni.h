#ifndef _FMOD_DSP_CONNECTION_H
#define _FMOD_DSP_CONNECTION_H

#include "fmod_settings.h"
#include "fmod.hpp"
#include "fmod_channeli.h"
#include "fmod_linkedlist.h"

#include <stdlib.h>

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    #if defined(PLATFORM_PSP) || defined(PLATFORM_PS2) || defined(PLATFORM_GC) || defined(PLATFORM_IPHONE)
        #define DSP_MAXLEVELS_OUT 2     /* Save memory on these platforms.  They'll never see more than stereo. */
    #elif (defined(PLATFORM_XENON) && !defined(FMOD_SUPPORT_NEURAL)) || defined(PLATFORM_XBOX) || defined(PLATFORM_WII) /* DPL2 for Wii */
        #define DSP_MAXLEVELS_OUT 6     /* Never see more than 5.1. */
    #elif defined(PLATFORM_PS3) || defined(PLATFORM_WINDOWS_PS3MODE) || (defined(PLATFORM_XENON) && defined(FMOD_SUPPORT_NEURAL))
        #define DSP_MAXLEVELS_OUT 8     /* Never see more than 7.1. */
    #else
        #define DSP_MAXLEVELS_OUT 16
    #endif

    #define DSP_MAXLEVELS_IN      FMOD_CHANNEL_MAXINPUTCHANNELS
    #define DSP_MAXLEVELS_MAX     16    /* This should be whatever is the largest out of DSP_MAXLEVELS_IN and DSP_MAXLEVELS_OUT */

    #define DSP_DEFAULTLEVELS_IN  6     /* Enough for a 5.1 input sound. */

    const int DSP_RAMPCOUNT     = 64;   /* All volume ramps will happen over this number of samples to remove clicks. */

    #ifdef PLATFORM_XENON
        #define DSP_LEVEL_PRECISION 16      /* Half floats - 5 bit exponent, 15bit mantissa. */
    #else
        #define DSP_LEVEL_PRECISION 32
    #endif

    #define DSP_LEVEL_SMALLVAL 0.0001f

    #if (DSP_LEVEL_PRECISION == 32)
        #define DSP_LEVEL_TYPE float

        #define DSP_LEVEL_COMPRESS(_val) _val
        #define DSP_LEVEL_DECOMPRESS(_val) _val
    #else
        #define DSP_LEVEL_TYPE unsigned short

        #ifdef PLATFORM_XENON
            FMOD_INLINE unsigned short FMOD_DSP_ConnectionI_Single2HalfP(float source)
            {
                __vector4 a;
                a.y = source;
                return __vpkd3d( a, a, VPACK_FLOAT16_4, VPACK_64LO, 0 ).u[2];
            }       
            FMOD_INLINE float FMOD_DSP_ConnectionI_HalfP2Single(unsigned short h)
            {
                __vector4 a;
                a.u[3] = h;
                return __vupkd3d( a, VPACK_FLOAT16_4).w;
            }
        #else
            unsigned short FMOD_DSP_ConnectionI_Single2HalfP(float source);
            float          FMOD_DSP_ConnectionI_HalfP2Single(unsigned short h);
        #endif

        #define DSP_LEVEL_COMPRESS(_val)   FMOD_DSP_ConnectionI_Single2HalfP(_val)
        #define DSP_LEVEL_DECOMPRESS(_val) FMOD_DSP_ConnectionI_HalfP2Single(_val)
    #endif

	class DSPI;
         
    class DSPConnectionI
    {
        DECLARE_MEMORYTRACKER

      public:

        LinkedListNode   mInputNode;
        LinkedListNode   mOutputNode;
        LinkedListNode  *mNode;         /* Make it a pointer so we can store the node data externally.  PS3 will corrupt it otherwise. */

        short           mMaxOutputLevels FMOD_PACKED;
        short           mMaxInputLevels  FMOD_PACKED;

        DSP_LEVEL_TYPE *mLevel[DSP_MAXLEVELS_OUT];
        DSP_LEVEL_TYPE *mLevelCurrent[DSP_MAXLEVELS_OUT];
        DSP_LEVEL_TYPE *mLevelDelta[DSP_MAXLEVELS_OUT];

      public:
        
        DSPI           *mInputUnit;
        DSPI           *mOutputUnit;

        short           mRampCount       FMOD_PACKED;
        short           mSetLevelsUsed   FMOD_PACKED;

        #ifdef PLATFORM_PS3
        unsigned short  mInputUnitSize   FMOD_PACKED;
        unsigned short  mOutputUnitSize  FMOD_PACKED;
        
        FMOD_PPCALIGN16(unsigned int    mMramAddress);
        FMOD_PPCALIGN16(unsigned int    mMramAddressLevels);

        #endif

        float           mVolume;

                                    
        FMOD_RESULT     init        (DSP_LEVEL_TYPE * &levelmemory, int maxoutputlevels, int maxinputlevels);
                                    
        FMOD_RESULT     mix         (float * FMOD_RESTRICT outbuffer, float * FMOD_RESTRICT inbuffer, int outchannels, int inchannels, unsigned int length);
        FMOD_RESULT     mixAndRamp  (float * FMOD_RESTRICT outbuffer, float * FMOD_RESTRICT inbuffer, int outchannels, int inchannels, unsigned int length);
                                    
        FMOD_RESULT     getInput    (DSPI **input);
        FMOD_RESULT     getOutput   (DSPI **output);
        FMOD_RESULT     reset       ();
        FMOD_RESULT     setUnity    ();
        FMOD_RESULT     rampTo      ();
        FMOD_RESULT     checkUnity  (int outchannels, int inchannels);
        FMOD_RESULT     checkMono   (int outchannels, int inchannels);
        FMOD_RESULT     setPan      (float pan, int outchannels, int inchannels, FMOD_SPEAKERMODE speakermode);
        FMOD_RESULT     setMix      (float volume);
        FMOD_RESULT     getMix      (float *volume);
        FMOD_RESULT     setLevels   (float *levels, int numinputlevels);
        FMOD_RESULT     getLevels   (float *levels, int numinputlevels);
        FMOD_RESULT     setLevels   (FMOD_SPEAKER speaker, float *levels, int numlevels);
        FMOD_RESULT     getLevels   (FMOD_SPEAKER speaker, float *levels, int numlevels);

        FMOD_RESULT     copy       (DSPConnectionI *source);

    };

}

#endif  

