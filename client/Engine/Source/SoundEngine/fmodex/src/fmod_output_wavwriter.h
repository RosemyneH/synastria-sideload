#ifndef _FMOD_OUTPUT_WAVWRITER_H
#define _FMOD_OUTPUT_WAVWRITER_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_WAVWRITER

#include "fmod_output_polled.h"
#include "fmod_codec_wav.h"

#include <stdio.h>

namespace FMOD
{
    class OutputWavWriter : public OutputPolled
    {
      private:
        
        char              mFileName[FMOD_STRING_MAXPATHLEN];
        void             *mBuffer;
        unsigned int      mBufferLengthBytes;
        int               mChannels, mRate, mBits;
        unsigned int      mLengthBytes;
        FMOD_SOUND_FORMAT mFormat;
        FILE             *mFP;

        FMOD_RESULT     writeWavHeader();

      public:
        
        static FMOD_OUTPUT_DESCRIPTION_EX *getDescriptionEx();  

        FMOD_RESULT     getNumDrivers(int *numdrivers);
        FMOD_RESULT     getDriverName(int id, char *name, int namelen);
        FMOD_RESULT     getDriverCaps(int id, FMOD_CAPS *caps);
	    FMOD_RESULT     init(int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata);
	    FMOD_RESULT     close();
        FMOD_RESULT     getHandle(void **handle);
        FMOD_RESULT     getPosition(unsigned int *pcm);
        FMOD_RESULT     lock(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
        FMOD_RESULT     unlock(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);

        static FMOD_RESULT F_CALLBACK getNumDriversCallback(FMOD_OUTPUT_STATE *output, int *numdrivers);
        static FMOD_RESULT F_CALLBACK getDriverNameCallback(FMOD_OUTPUT_STATE *output, int id, char *name, int namelen);
        static FMOD_RESULT F_CALLBACK getDriverCapsCallback(FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps);
        static FMOD_RESULT F_CALLBACK initCallback         (FMOD_OUTPUT_STATE *output, int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata);
        static FMOD_RESULT F_CALLBACK closeCallback        (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK getPositionCallback  (FMOD_OUTPUT_STATE *output, unsigned int *pcm);
        static FMOD_RESULT F_CALLBACK lockCallback         (FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
        static FMOD_RESULT F_CALLBACK unlockCallback       (FMOD_OUTPUT_STATE *output, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);
        static FMOD_RESULT F_CALLBACK getHandleCallback    (FMOD_OUTPUT_STATE *output, void **handle);
    };
}

#endif /* #ifdef FMOD_SUPPORT_WAVWRITER */

#endif

