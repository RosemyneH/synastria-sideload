#ifndef _FMOD_DSP_FFT
#define _FMOD_DSP_FFT

#include "fmod_settings.h"

#include "fmod.h"
#include "fmod_types.h"

namespace FMOD
{        
    const int DSPFFT_COSTABBITS = 13;
    const int DSPFFT_COSTABSIZE = (1 << DSPFFT_COSTABBITS);
    const int DSPFFT_TABLERANGE = (DSPFFT_COSTABSIZE * 4);
    const int DSPFFT_TABLEMASK  = (DSPFFT_TABLERANGE - 1);

    class DSPFFT
    {
      private:

        FMOD_COMPLEX    mFFTBuffer[16 * 1024];

#define USECOSTAB

#ifdef USECOSTAB
        float          mCosTab[DSPFFT_COSTABSIZE];

        FMOD_INLINE const float    cosine(float x);
        FMOD_INLINE const float    sine(float x);
#else
        FMOD_INLINE const float    cosine(float x) { return FMOD_COS(x * FMOD_PI * 2.0f); }
        FMOD_INLINE const float    sine(float x)   { return FMOD_SIN(x * FMOD_PI * 2.0f); }
#endif
        FMOD_INLINE const unsigned int  reverse(unsigned int val, int bits);

        FMOD_RESULT process(int bits);

      public:
        
        DSPFFT();

        FMOD_RESULT getSpectrum(float *pcmbuffer, unsigned int pcmposition, unsigned int pcmlength,  float *spectrum, int length, int channel, int numchannels, FMOD_DSP_FFT_WINDOW windowtype);

    };
}


#endif /* _FMOD_DSP_FFT */

