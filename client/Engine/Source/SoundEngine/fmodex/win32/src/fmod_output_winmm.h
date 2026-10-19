#ifndef _FMOD_OUTPUT_WINMM_H
#define _FMOD_OUTPUT_WINMM_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_WINMM

#include "fmod_output_polled.h"

#include <windows.h>
#include <mmsystem.h>

namespace FMOD
{
    const int PLAY_MAXBLOCKS    = 1;    /* number of buffers   */
    const int RECORD_MAXBLOCKS  = 100;  /* number of buffers   */
    const int RECORD_BLOCKLENMS = 5;    /* size of each buffer - 100 * 5 = 500ms with 5ms granularity */

    typedef struct 
    {
	    WAVEHDR		wavehdr;
	    char	   *data;
    } SoundBlock;

    class OutputWinMM : public OutputPolled
    {
      private:

        bool               mCoInitialized;

        /*
            Playback information.
        */
        HWAVEOUT           mHandle;
        bool               mRunning;
        char              *mBuffer;
        SoundBlock         mBlock[PLAY_MAXBLOCKS];
        int                mNumBlocks;
        unsigned int       mBlockLengthBytes;
                     
        /*
            Record information.
        */
#ifdef FMOD_SUPPORT_RECORDING
        HWAVEIN            mRecordHandle;
        Thread             mRecordThread;
        bool               mRecording;
        int                mRecordCurrentBlock;
        int                mRecordNextBlock;
        SoundBlock         mRecordBlock[RECORD_MAXBLOCKS];
        char              *mRecordData;
        unsigned int       mRecordBlockLenBytes;
        int                mRecordBlockAlign;

        static void CALLBACK recordCallback(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        static void          recordThreadCallback(void *userdata);
        FMOD_RESULT          recordThread();
#endif

        FMOD_RESULT          testFormat(int id, FMOD_SOUND_FORMAT format, int channels);

      public:

        static FMOD_OUTPUT_DESCRIPTION_EX *getDescriptionEx();  
        
        FMOD_RESULT              enumerate();
        FMOD_RESULT              getNumDrivers(int *numdrivers);
        FMOD_RESULT              getDriverName(int id, char *name, int namelen);
        FMOD_RESULT              getDriverCaps(int id, FMOD_CAPS *caps);
	    FMOD_RESULT              init(int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbuffersize, int dspnumbuffers, void *extradriverdata);
	    FMOD_RESULT              close();
        FMOD_RESULT              getHandle(void **handle);
        FMOD_RESULT              start();
        FMOD_RESULT              stop();                                       
	    FMOD_RESULT              getPosition(unsigned int *pcm);
	    FMOD_RESULT              lock(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
	    FMOD_RESULT              unlock(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);

#ifdef FMOD_SUPPORT_RECORDING
        FMOD_RESULT              recordGetNumDrivers(int *numdrivers);
        FMOD_RESULT              recordGetDriverInfo(int id, char *name, int namelen);
        FMOD_RESULT              recordStart        (int id, Sound *sound, bool loop);
        FMOD_RESULT              recordStop         ();
        FMOD_RESULT              recordGetPosition  (unsigned int *pcm);
        FMOD_RESULT              recordLock         (unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
#endif

        static FMOD_RESULT F_CALLBACK getNumDriversCallback         (FMOD_OUTPUT_STATE *output, int *numdrivers);
        static FMOD_RESULT F_CALLBACK getDriverNameCallback         (FMOD_OUTPUT_STATE *output, int id, char *name, int namelen);
        static FMOD_RESULT F_CALLBACK getDriverCapsCallback         (FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps);
        static FMOD_RESULT F_CALLBACK initCallback                  (FMOD_OUTPUT_STATE *output, int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata);
        static FMOD_RESULT F_CALLBACK closeCallback                 (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK startCallback                 (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK stopCallback                  (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK getHandleCallback             (FMOD_OUTPUT_STATE *output, void **handle);
        static FMOD_RESULT F_CALLBACK getPositionCallback           (FMOD_OUTPUT_STATE *output, unsigned int *pcm);
        static FMOD_RESULT F_CALLBACK lockCallback                  (FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
        static FMOD_RESULT F_CALLBACK unlockCallback                (FMOD_OUTPUT_STATE *output, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);
#ifdef FMOD_SUPPORT_RECORDING
        static FMOD_RESULT F_CALLBACK recordGetNumDriversCallback   (FMOD_OUTPUT_STATE *output, int *numdrivers);
        static FMOD_RESULT F_CALLBACK recordGetDriverInfoCallback   (FMOD_OUTPUT_STATE *output, int id, char *name, int namelen);
        static FMOD_RESULT F_CALLBACK recordStartCallback           (FMOD_OUTPUT_STATE *output, int id, FMOD_SOUND *sound, int loop);
        static FMOD_RESULT F_CALLBACK recordStopCallback            (FMOD_OUTPUT_STATE *output);
        static FMOD_RESULT F_CALLBACK recordGetPositionCallback     (FMOD_OUTPUT_STATE *output, unsigned int *pcm);
        static FMOD_RESULT F_CALLBACK recordLockCallback            (FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
#endif
    };
}

#endif  /* #ifdef FMOD_SUPPORT_WINMM */

#endif