#ifndef _FMOD_DSP_CODEC_H
#define _FMOD_DSP_CODEC_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_DSPCODEC

#include "fmod.h"
#include "fmod_dsp_codecpool.h"
#include "fmod_dsp_resampler.h"

#ifdef PLATFORM_PS3
#include "fmod_file_dma.h"
#else
#include "fmod_file_memory.h"
#endif

#ifdef FMOD_SUPPORT_MPEG
    #include "fmod_codec_mpeg.h"
#endif

#ifdef FMOD_SUPPORT_IMAADPCM
    #include "fmod_codec_wav.h"
#endif

#ifdef FMOD_SUPPORT_XMA
    #include "fmod_codec_xma.h"
#endif


#include "fmod_codec_raw.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    class DSPCodecPool;
    class SoundI;

    class DSPCodec : public DSPResampler
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

        friend class DSPCodecPool;
        friend class DSPResampler;

      private:

        FMOD_RESULT     createInternal();
        FMOD_RESULT     release(bool freethis = true);
        FMOD_RESULT     releaseInternal();
        FMOD_RESULT     resetInternal();
        FMOD_RESULT     readInternal(signed short *inbuffer, signed short *outbuffer, unsigned int length, int inchannels, int outchannels);
        FMOD_RESULT     setPositionInternal(unsigned int pos);
        FMOD_RESULT     setParameterInternal(int index, float value);
        FMOD_RESULT     getParameterInternal(int index, float *value, char *valuestr);

      public:

        DSPCodecPool   *mPool;
        MemoryFile      mMemoryFile;
        unsigned int    mPosition;
        unsigned int    mNewPosition;
        int             mPoolIndex;
        FMOD_CODEC_WAVEFORMAT mWaveFormat;
        Codec          *mCodec;

        FMOD_RESULT     freeFromPool() { mPool->mAllocated[mPoolIndex] = false; return FMOD_OK; }

        static FMOD_DSP_DESCRIPTION_EX *getDescriptionEx();

        static FMOD_RESULT F_CALLBACK createCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK releaseCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK resetCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_DSP_STATE *dsp, unsigned int pos);
        static FMOD_RESULT F_CALLBACK setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value);
        static FMOD_RESULT F_CALLBACK getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr);
    }; 
    
#ifdef FMOD_SUPPORT_MPEG
    class DSPCodecMPEG : public DSPCodec
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      public:
        char                  mResampleBufferMemory[((1152 + (FMOD_DSP_RESAMPLER_OVERFLOWLENGTH * 4)) * sizeof(short) * 2 * 2) + 16];    // *2 = stereo max.  *2 = double buffer.
        CodecMPEG             mCodecMemory;
        CodecMPEG_MemoryBlock mCodecMemoryBlock;
    };
#endif

#ifdef FMOD_SUPPORT_IMAADPCM
    class DSPCodecADPCM : public DSPCodec
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      public:
        char                 mResampleBufferMemory[((64 + (FMOD_DSP_RESAMPLER_OVERFLOWLENGTH * 4)) * sizeof(short) * 2 * 2) + 16];    // *2 = stereo max.  *2 = double buffer.
        CodecWav             mCodecMemory;
    };
#endif

#ifdef FMOD_SUPPORT_XMA
    class DSPCodecXMA : public DSPCodec
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      public:
        char                  mResampleBufferMemory[((512 + (FMOD_DSP_RESAMPLER_OVERFLOWLENGTH * 4)) * sizeof(short) * 2 * 2) + 16];      // *2 = stereo max.  *2 = double buffer.
        CodecXMA              mCodecMemory;
        #ifdef FMOD_SUPPORT_XMA_NEWHAL
        CodecXMA_DecoderData  mCodecMemoryBlock;
        #else
        CodecXMA_DecoderHW    mCodecMemoryBlock;
        #endif        
    };
#endif


#ifdef FMOD_SUPPORT_RAW
    class DSPCodecRaw : public DSPCodec
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      public:
        char      mResampleBufferMemory[((256 + (FMOD_DSP_RESAMPLER_OVERFLOWLENGTH * 4)) * sizeof(short) * 8 * 2) + 16];      // *8 = 8 channel max.  *2 = double buffer.
        CodecRaw  mCodecMemory;
    };
#endif

}

#endif

#endif  

