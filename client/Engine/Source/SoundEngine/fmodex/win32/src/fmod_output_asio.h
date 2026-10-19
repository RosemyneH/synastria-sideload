#ifndef _FMOD_OUTPUT_ASIO_H
#define _FMOD_OUTPUT_ASIO_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_ASIO

#include "fmod_outputi.h"

#include "asio\asiosys.h"
#include "asio\asio.h"
#include "asio\asiodrivers.h"

namespace FMOD
{
    class OutputASIO : public Output
    {
      private:

        AsioDrivers        *mHandle;
        long                mInputChannels;
        long                mOutputChannels;
        long                mBufferMinSize;
        long                mBufferMaxSize;
        long                mBufferPreferredSize;
        long                mBufferGranularity;
        bool                mPostOutput;
        float              *mInterleavedBuffer;

#ifdef FMOD_SUPPORT_RECORDING
        float              *mInterleavedRecordBuffer;
        unsigned int        mInterleavedRecordBufferPos;
        bool                mRecordLoop;
#endif

        ASIOCallbacks       mCallbacks;
        ASIOBufferInfo     *mBufferInfo;
        ASIOChannelInfo    *mChannelInfo;


        static void         bufferSwitch(long index, ASIOBool processNow);
        static ASIOTime *   bufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
        static void         sampleRateDidChange(ASIOSampleRate sRate);
        static long         asioMessage(long selector, long value, void* message, double* opt);
        
#ifdef FMOD_SUPPORT_SOFTWARE
        FMOD_RESULT         updateMixer(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
#endif

      public:
        
        static FMOD_OUTPUT_DESCRIPTION_EX *getDescriptionEx();  

        FMOD_RESULT              enumerate();
        FMOD_RESULT              getNumDrivers(int *numdrivers);
        FMOD_RESULT              getDriverName(int id, char *name, int namelen);
        FMOD_RESULT              getDriverCaps(int id, FMOD_CAPS *caps);
	    FMOD_RESULT              init(int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata);
	    FMOD_RESULT              close();
        FMOD_RESULT              getHandle(void **handle);
        FMOD_RESULT              start();
        FMOD_RESULT              stop();                                       

#ifdef FMOD_SUPPORT_RECORDING
        FMOD_RESULT              recordEnumerate();
        FMOD_RESULT              recordGetNumDrivers(int *numdrivers);
        FMOD_RESULT              recordGetDriverInfo(int id, char *name, int namelen);
        FMOD_RESULT              recordStart        (int id, Sound *sound, bool loop);
        FMOD_RESULT              recordStop         ();
        FMOD_RESULT              recordGetPosition  (unsigned int *pcm);
        FMOD_RESULT              recordLock         (unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
        FMOD_RESULT              recordUnlock       (void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);
#endif

        static FMOD_RESULT F_CALLBACK getNumDriversCallback      (FMOD_OUTPUT_STATE *output, int *numdrivers);
        static FMOD_RESULT F_CALLBACK getDriverNameCallback      (FMOD_OUTPUT_STATE *output, int id, char *name, int namelen);
        static FMOD_RESULT F_CALLBACK getDriverCapsCallback      (FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps);
        static FMOD_RESULT F_CALLBACK initCallback               (FMOD_OUTPUT_STATE *output, int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata);
        static FMOD_RESULT F_CALLBACK closeCallback              (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK startCallback              (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK stopCallback               (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK getHandleCallback          (FMOD_OUTPUT_STATE *output, void **handle);

#ifdef FMOD_SUPPORT_RECORDING
        static FMOD_RESULT F_CALLBACK recordGetNumDriversCallback(FMOD_OUTPUT_STATE *output, int *numdrivers);
        static FMOD_RESULT F_CALLBACK recordGetDriverInfoCallback(FMOD_OUTPUT_STATE *output, int id, char *name, int namelen);
        static FMOD_RESULT F_CALLBACK recordStartCallback        (FMOD_OUTPUT_STATE *output, int id, FMOD_SOUND *sound, int loop);
        static FMOD_RESULT F_CALLBACK recordStopCallback         (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK recordGetPositionCallback  (FMOD_OUTPUT_STATE *output, unsigned int *pcm);
        static FMOD_RESULT F_CALLBACK recordLockCallback         (FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
        static FMOD_RESULT F_CALLBACK recordUnlockCallback       (FMOD_OUTPUT_STATE *output, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);
#endif
    };
}

#endif  /* #ifdef FMOD_SUPPORT_ASIO */

#endif