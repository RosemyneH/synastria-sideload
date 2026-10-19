#ifndef _FMOD_SOUNDI_H
#define _FMOD_SOUNDI_H

#include "fmod_settings.h"

#include "fmod.hpp"
#include "fmod_channeli.h"
#include "fmod_linkedlist.h"
#include "fmod_memory.h"
#include "fmod_string.h"
#include "fmod_syncpoint.h"
#include "fmod_types.h"
#include "fmod_codeci.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    const int DEFAULT_FREQUENCY = 44100;

    class Sample;
    class DSPI;
    class SystemI;
    class SoundGroupI;
    struct AsyncData;

    const unsigned int SOUND_READCHUNKSIZE = (16*1024);

    typedef unsigned int FMOD_SOUND_FLAG;

    #define FMOD_SOUND_FLAG_DONOTRELEASE    0x00000001
    #define FMOD_SOUND_FLAG_READDATA        0x00000002
    #define FMOD_SOUND_FLAG_PLAYED          0x00000004
    #define FMOD_SOUND_FLAG_FINISHED        0x00000008
    #define FMOD_SOUND_FLAG_WANTSTOFLUSH    0x00000010

    #define FMOD_PS3_SUBSOUNDLISTMAXITEMS   512             // 4k max for PS3


    class SoundI : public LinkedListNode
    {
        DECLARE_MEMORYTRACKER

      public:

        FMOD_SOUND_TYPE         mType;
        FMOD_SOUND_FORMAT       mFormat;
        FMOD_MODE               mMode;
        char                   *mName;
        unsigned int            mPosition;
        unsigned int            mLength;
        unsigned int            mLengthBytes;
        unsigned int            mLoopStart;
        unsigned int            mLoopLength;
        int                     mLoopCount;
        FMOD_SOUND_FLAG         mFlags;
                               
        Codec                  *mCodec;
        int                     mChannels;
        float                   mDefaultVolume;
        float                   mDefaultFrequency;
        float                   mDefaultPan;
        int                     mDefaultPriority;
        unsigned int            mDefaultChannelMask;
        float                   mFrequencyVariation;
        float                   mVolumeVariation;
        float                   mPanVariation;

        /*
            3D Stuff
        */
        float                   mMinDistance;
        float                   mMaxDistance;
        float                   mConeInsideAngle;
        float                   mConeOutsideAngle;
        float                   mConeOutsideVolume;
        FMOD_VECTOR            *mRolloffPoint;
        int                     mNumRolloffPoints;

        /* 
            SubSound stuff
        */
        SoundI                **mSubSound;              /* Array of pointers to subsounds. */
        SoundI                 *mSubSoundShared;
        int                     mNumSubSounds;
        int                     mNumActiveSubSounds;
        SoundI                 *mSubSoundParent;
        int                     mSubSoundIndex;
#ifdef FMOD_SUPPORT_SENTENCING
        int                    *mSubSoundList;
        int                     mSubSoundListNum;
        int                     mSubSoundListCurrent;
#endif
        SoundI                 *mSubSampleParent;       /* Pointer to parent subsample if multisample sound. */
        void                   *mUserData;
        SystemI                *mSystem;
        unsigned int            mMemoryUsed;
        int                     mNumSyncPoints;
        SyncPoint              *mSyncPointHead;
        AsyncData              *mAsyncData;             /* Data for async loading. */
        FMOD_OPENSTATE          mOpenState;
        SoundGroupI            *mSoundGroup;
        LinkedListNode          mSoundGroupNode;
        int                     mNumAudible;
        FMOD_SOUND_PCMREADCALLBACK     mPostReadCallback;
        FMOD_SOUND_PCMSETPOSCALLBACK   mPostSetPositionCallback;
        FMOD_SOUND                    *mPostCallbackSound;

        FMOD_RESULT             loadSubSound(int index, FMOD_MODE mode);
        FMOD_RESULT             read(unsigned int offset, unsigned int numsamples, unsigned int *read);
        FMOD_RESULT             seek(int subsound, unsigned int position);
        FMOD_RESULT             clear(unsigned int offset, unsigned int numsamples);
        FMOD_RESULT             setPositionInternal(unsigned int pcm);
        FMOD_RESULT             updateSubSound(int index, bool updateinfoonly);


        static FMOD_RESULT  validate(Sound *sound, SoundI **soundi);

        FMOD_INLINE FMOD_RESULT getBytesFromSamples(unsigned int samples, unsigned int *bytes, bool roundup = true)
        {
            return getBytesFromSamples(samples, bytes, mChannels, mFormat, roundup);
        }
        static FMOD_INLINE FMOD_RESULT getBytesFromSamples(unsigned int samples, unsigned int *bytes, int channels, FMOD_SOUND_FORMAT format, bool roundup = true)
        {
            int bits = 0;

            getBitsFromFormat(format, &bits);

            if (bits)
            {
                *bytes = samples * bits / 8;
            }
            else
            {
                switch (format)
                {
                    case FMOD_SOUND_FORMAT_GCADPCM  : { *bytes = ((samples + (roundup ? 13 : 0)) / 14) * 14 *  8 / 14; break; }
                    case FMOD_SOUND_FORMAT_IMAADPCM : { *bytes = ((samples + (roundup ? 63 : 0)) / 64) * 64 * 36 / 64; break; }
                    case FMOD_SOUND_FORMAT_VAG      : { *bytes = ((samples + (roundup ? 27 : 0)) / 28) * 28 * 16 / 28; break; }
                    case FMOD_SOUND_FORMAT_XMA      : { *bytes = samples; return FMOD_OK; }
                    case FMOD_SOUND_FORMAT_MPEG     : { *bytes = samples; return FMOD_OK; }
					case FMOD_SOUND_FORMAT_NONE		: { *bytes = 0; break; }
                    default : return FMOD_ERR_FORMAT;
                }
            }

            *bytes *= channels;

            return FMOD_OK;
        }

        FMOD_INLINE FMOD_RESULT getSamplesFromBytes(unsigned int bytes, unsigned int *samples)
        {
            return getSamplesFromBytes(bytes, samples, mChannels, mFormat);
        }
        static FMOD_INLINE FMOD_RESULT getSamplesFromBytes(unsigned int bytes, unsigned int *samples, int channels, FMOD_SOUND_FORMAT format)
        {
            int bits = 0;
            
            if (!channels)
            {
                return FMOD_ERR_INVALID_PARAM;
            }

            getBitsFromFormat(format, &bits);

            if (bits)
            {
                *samples = (unsigned int)((FMOD_UINT64)bytes * (FMOD_UINT64)8 / (FMOD_UINT64)bits);
            }
            else
            {
                switch (format)
                {
                    case FMOD_SOUND_FORMAT_GCADPCM  : { *samples = bytes * 14 / 8; break; }
                    case FMOD_SOUND_FORMAT_IMAADPCM : { *samples = bytes * 64 / 36; break; }
                    case FMOD_SOUND_FORMAT_VAG      : { *samples = bytes * 28 / 16; break; }
                    case FMOD_SOUND_FORMAT_XMA      : { *samples = bytes; return FMOD_OK; }
                    case FMOD_SOUND_FORMAT_MPEG     : { *samples = bytes; return FMOD_OK; }
					case FMOD_SOUND_FORMAT_NONE		: { *samples = 0; break; }
                    default : return FMOD_ERR_FORMAT;
                }
            }

            *samples /= channels;

            return FMOD_OK;
        }

        FMOD_INLINE FMOD_RESULT getBitsFromFormat(int *bits)
        {
            return getBitsFromFormat(mFormat, bits);
        }
        static FMOD_INLINE FMOD_RESULT getBitsFromFormat(FMOD_SOUND_FORMAT format, int *bits)
        {
            switch (format)
            {
                case FMOD_SOUND_FORMAT_PCM8     : { *bits = 8; break; }
                case FMOD_SOUND_FORMAT_PCM16    : { *bits = 16; break; }
                case FMOD_SOUND_FORMAT_PCM24    : { *bits = 24; break; }
                case FMOD_SOUND_FORMAT_PCM32    : { *bits = 32; break; }
                case FMOD_SOUND_FORMAT_PCMFLOAT : { *bits = 32; break; }
                case FMOD_SOUND_FORMAT_GCADPCM  : { *bits = 0; break; }
                case FMOD_SOUND_FORMAT_IMAADPCM : { *bits = 0; break; }
                case FMOD_SOUND_FORMAT_XMA      : { *bits = 0; break; }
                case FMOD_SOUND_FORMAT_VAG      : { *bits = 0; break; }
                case FMOD_SOUND_FORMAT_MPEG     : { *bits = 0; break; }
				case FMOD_SOUND_FORMAT_NONE		: {	*bits = 0; break; }
                default : return FMOD_ERR_FORMAT;
            }
            return FMOD_OK;
        }
        static FMOD_INLINE FMOD_RESULT getFormatFromBits(int bits, FMOD_SOUND_FORMAT *format)
        {
            switch (bits)
            {
                case 8  : { *format = FMOD_SOUND_FORMAT_PCM8; break; }
                case 16 : { *format = FMOD_SOUND_FORMAT_PCM16; break; }
                case 24 : { *format = FMOD_SOUND_FORMAT_PCM24; break; }
                case 32 : { *format = FMOD_SOUND_FORMAT_PCM32; break; }
                default : return FMOD_ERR_FORMAT;
            }
            return FMOD_OK;
        }
        virtual bool isStream() { return false; }

      public:

        SoundI();

        bool                 readyForRelease();
        virtual FMOD_RESULT  release              (bool freethis = true);
        virtual FMOD_RESULT  getSystemObject      (System **system);
        virtual FMOD_RESULT  lock                 (unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
        virtual FMOD_RESULT  unlock               (void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);
        virtual FMOD_RESULT  setDefaults          (float frequency, float volume, float pan, int priority);
        virtual FMOD_RESULT  getDefaults          (float *frequency, float *volume, float *pan, int *priority);
        virtual FMOD_RESULT  setVariations        (float frequencyvar, float volumevar, float panvar);
        virtual FMOD_RESULT  getVariations        (float *frequencyvar, float *volumevar, float *panvar);
        virtual FMOD_RESULT  set3DMinMaxDistance  (float min, float max);
        virtual FMOD_RESULT  get3DMinMaxDistance  (float *min, float *max);
        virtual FMOD_RESULT  set3DConeSettings    (float insideconeangle, float outsideconeangle, float outsidevolume);
        virtual FMOD_RESULT  get3DConeSettings    (float *insideconeangle, float *outsideconeangle, float *outsidevolume);
        virtual FMOD_RESULT  set3DCustomRolloff   (FMOD_VECTOR *points, int numpoints);
        virtual FMOD_RESULT  get3DCustomRolloff   (FMOD_VECTOR **points, int *numpoints);
        virtual FMOD_RESULT  setSubSound          (int index, SoundI  *subsound);
        virtual FMOD_RESULT  getSubSound          (int index, SoundI **subsound);
        virtual FMOD_RESULT  setSubSoundSentence  (int *subsoundlist, int numsubsounds);

        virtual FMOD_RESULT  getName              (char *name, int namelen);
        virtual FMOD_RESULT  getLength            (unsigned int *length, FMOD_TIMEUNIT lengthtype);
        virtual FMOD_RESULT  getFormat            (FMOD_SOUND_TYPE *type, FMOD_SOUND_FORMAT *format, int *channels, int *bits);
        virtual FMOD_RESULT  getNumSubSounds      (int *numsubsounds);
        virtual FMOD_RESULT  getNumTags           (int *numtags, int *numtagsupdated);
        virtual FMOD_RESULT  getTag               (const char *name, int index, FMOD_TAG *tag);
        virtual FMOD_RESULT  getOpenState         (FMOD_OPENSTATE *openstate, unsigned int *percentbuffered, bool *starving);
        virtual FMOD_RESULT  readData             (void *buffer, unsigned int lenbytes, unsigned int *read);
        virtual FMOD_RESULT  seekData             (unsigned int pcm);

        virtual FMOD_RESULT  setSoundGroup        (SoundGroupI *soundgroup);
        virtual FMOD_RESULT  getSoundGroup        (SoundGroupI **soundgroup);

        virtual FMOD_RESULT  getNumSyncPoints     (int *numsyncpoints);
        virtual FMOD_RESULT  getSyncPoint         (int index, FMOD_SYNCPOINT **point);
        virtual FMOD_RESULT  getSyncPointInfo     (FMOD_SYNCPOINT *point, char *name, int namelen, unsigned int *offset, FMOD_TIMEUNIT offsettype);
        virtual FMOD_RESULT  addSyncPoint         (unsigned int offset, FMOD_TIMEUNIT offsettype, const char *name, FMOD_SYNCPOINT **point);
        virtual FMOD_RESULT  deleteSyncPoint      (FMOD_SYNCPOINT *point);

        virtual FMOD_RESULT  setMode              (FMOD_MODE mode);
        virtual FMOD_RESULT  getMode              (FMOD_MODE *mode);
        virtual FMOD_RESULT  setLoopCount         (int loopcount);
        virtual FMOD_RESULT  getLoopCount         (int *loopcount);
        virtual FMOD_RESULT  setLoopPoints        (unsigned int loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int loopend, FMOD_TIMEUNIT loopendtype);
        virtual FMOD_RESULT  getLoopPoints        (unsigned int *loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int *loopend, FMOD_TIMEUNIT loopendtype);
        virtual FMOD_RESULT  setPosition          (unsigned int pcm);
        virtual FMOD_RESULT  getPosition          (unsigned int *pcm);

        virtual FMOD_RESULT  setUserData          (void *userdata);
        virtual FMOD_RESULT  getUserData          (void **userdata);

    };
}

#endif


