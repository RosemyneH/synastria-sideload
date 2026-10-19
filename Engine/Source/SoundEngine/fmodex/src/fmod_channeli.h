#ifndef _FMOD_CHANNELI_H
#define _FMOD_CHANNELI_H

#include "fmod_settings.h"

#include "fmod.hpp"
#include "fmod_linkedlist.h"
#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{

    const unsigned int SYSTEMID_BITS   = 4;
    const unsigned int CHANINDEX_BITS  = 12;
    const unsigned int REFCOUNT_BITS   = 16;
    const unsigned int SYSTEMID_SHIFT  = REFCOUNT_BITS + CHANINDEX_BITS;
    const unsigned int CHANINDEX_SHIFT = REFCOUNT_BITS;
    const unsigned int REFCOUNT_SHIFT  = 0;
    const unsigned int SYSTEMID_MASK   = (unsigned int)0xffffffff >> (32 - SYSTEMID_BITS);
    const unsigned int CHANINDEX_MASK  = (unsigned int)0xffffffff >> (32 - CHANINDEX_BITS);
    const unsigned int REFCOUNT_MASK   = (unsigned int)0xffffffff >> (32 - REFCOUNT_BITS);

    const unsigned int FMOD_CHANNEL_DEFAULTPRIORITY = 128;

    const int          FMOD_CHANNEL_MAXINPUTCHANNELS = 16;

    const int          FMOD_CHANNEL_MAXREALSUBCHANNELS = FMOD_CHANNEL_MAXINPUTCHANNELS;

    typedef enum
    {
        FMOD_CHANNEL_PANMODE_PAN,
        FMOD_CHANNEL_PANMODE_SPEAKERMIX,
        FMOD_CHANNEL_PANMODE_SPEAKERLEVELS,
    } FMOD_CHANNEL_PANMODE;

    class ChannelGroupI;
    class ChannelReal;
	class DSPI;
    class DSPConnectionI;
    class ReverbI;
    class SoundI;
    class Stream;
    class SyncPoint;
    class SystemI;

    typedef unsigned int FMOD_CHANNEL_FLAG;

    #define CHANNELI_FLAG_PAUSED                0x00000001
    #define CHANNELI_FLAG_MUTED                 0x00000002
    #define CHANNELI_FLAG_JUSTWENTVIRTUAL       0x00000004
    #define CHANNELI_FLAG_REALMUTE              0x00000008
    #define CHANNELI_FLAG_MOVED                 0x00000010
    #define CHANNELI_FLAG_USEDADDDSP            0x00000020
    #define CHANNELI_FLAG_FORCEVIRTUAL          0x00000040
    #define CHANNELI_FLAG_USEDINPUTMIX          0x00000080

    typedef struct
    {
        float                          *mLevels;
        unsigned int                    mPCM,  mLoopStart,  mLoopEnd;
        ChannelReal                    *mRealChannel;
        SoundI                         *mSound;
        DSPI                           *mDSP;
        int                             mLoopCount;
        bool                            mMute, mPaused;
        unsigned int                    mDelayStart, mDelayEnd;
        FMOD_REVERB_CHANNELPROPERTIES   mReverbProperties;
        DSPI                           *mAddDSP;
        FMOD_MODE                       mMode;
    } FMOD_CHANNEL_INFO;

    class ChannelI : public LinkedListNode      /* This linked list node entry is for System::mChannelFreeListHead / mChannelUsedListHead */
    {                                                                                                           /*  16 bytes */
        DECLARE_MEMORYTRACKER

        friend class Channel;

      public:

        SortedLinkedListNode    mSortedListNode;
        int                     mIndex;
        void                   *mUserData;
        unsigned int            mHandleOriginal;    /* Handle as it was when this channel was first played */
        SystemI                *mSystem;
        int                     mNumRealChannels;
        ChannelReal            *mRealChannel[FMOD_CHANNEL_MAXREALSUBCHANNELS];
        unsigned int            mHandleCurrent;     /* Handle as it is now */
        FMOD_CHANNEL_FLAG       mFlags;
        FMOD_CHANNEL_PANMODE    mLastPanMode;
        int                     mPriority;
        unsigned int            mListPosition;
        SyncPoint              *mSyncPointCurrent;
        ChannelGroupI          *mChannelGroup;
        LinkedListNode          mChannelGroupNode;
        float                   mFadeVolume;
        float                   mFadeTarget;
        float                   mVolume;
        float                   mFrequency;
        float                   mPan;
        float                   mSpeakerFL, mSpeakerFR, mSpeakerC, mSpeakerLFE;
        float                   mSpeakerBL, mSpeakerBR, mSpeakerSL, mSpeakerSR;
        float                   mInputMix[FMOD_CHANNEL_MAXINPUTCHANNELS];
        float                  *mLevels;

        /*
            3D
        */
        float                   mDirectOcclusionVolume;
        float                   mReverbDryVolume;
        float                   mVolume3D;   /* current relative 3d volume.    */
        float                   mPitch3D;    /* current relative 3d frequency. */
        FMOD_VECTOR             mPosition3D;
        FMOD_VECTOR             mVelocity3D;
        float                   mDistance;
        float                   mMinDistance;
        float                   mMaxDistance;
        float                   mConeVolume3D;
        float                   mConeInsideAngle;
        float                   mConeOutsideAngle;
        float                   mConeOutsideVolume;
        FMOD_VECTOR             mConeOrientation;
        float                   mDirectOcclusion;
        float                   mReverbOcclusion;
        float                   mDirectOcclusionTarget;
        float                   mReverbOcclusionTarget;
        FMOD_VECTOR            *mRolloffPoint;
        int                     mNumRolloffPoints;
        float                   mSpread;
        float                   m3DPanLevel;
        float                   m3DDopplerLevel;

        FMOD_CHANNEL_CALLBACK   mCallback[FMOD_CHANNEL_CALLBACKTYPE_MAX];
        int                     mCallbackCommand[FMOD_CHANNEL_CALLBACKTYPE_MAX];
                                                                                                                /* --------- */
        static FMOD_RESULT      validate(Channel *channel, ChannelI **channeli);

        FMOD_RESULT             returnToFreeList();
        FMOD_RESULT             setDefaults();
        FMOD_RESULT             referenceStamp(bool newstamp = false);
		FMOD_RESULT             updatePosition();
        FMOD_RESULT             forceVirtual(bool force);
        FMOD_RESULT             getChannelInfo(FMOD_CHANNEL_INFO *info);
        FMOD_RESULT             setChannelInfo(FMOD_CHANNEL_INFO *info);
        FMOD_RESULT             getRealChannel(ChannelReal **realchan, int *subchannels);
        FMOD_RESULT             calcVolumeAndPitchFor3D(int delta);
        FMOD_RESULT             setChannelGroupInternal(ChannelGroupI *channelgroup, bool resetattributes);
		FMOD_RESULT	            set3DOcclusionInternal(float directOcclusion, float reverbOcclusion, bool resettarget=true);

      public:

        ChannelI();
        ChannelI(int index, SystemI *system);
        FMOD_RESULT  init();


        FMOD_RESULT  getSystemObject      (System **system);

        FMOD_RESULT  play                 (SoundI *sound, bool paused, bool reset, bool startmuted);
        FMOD_RESULT  play                 (DSPI *dsp, bool paused, bool reset, bool startmuted);
        FMOD_RESULT  alloc                (SoundI *sound, bool reset);
        FMOD_RESULT  alloc                (DSPI *dsp, bool reset);
        FMOD_RESULT  start                ();
        FMOD_RESULT  updateSyncPoints     (bool seeking);
        FMOD_RESULT  update               (int delta);

        FMOD_RESULT  stop                 ();
        FMOD_RESULT  stopEx               (bool refstamp, bool updatelist, bool resetcallbacks, bool resetstate, bool callendcallback, bool resetchannelgroup, bool force);
        FMOD_RESULT  setPaused            (bool paused);
        FMOD_RESULT  getPaused            (bool *paused);
        FMOD_RESULT  setVolume            (float volume, bool forceupdatepos = false);
        FMOD_RESULT  getVolume            (float *volume);
        FMOD_RESULT  setFrequency         (float frequency);
        FMOD_RESULT  getFrequency         (float *frequency);
        FMOD_RESULT  setPan               (float pan, bool calldriver = true);
        FMOD_RESULT  getPan               (float *pan);
        FMOD_RESULT  setDelay             (unsigned int startdelay, unsigned int enddelay);
        FMOD_RESULT  getDelay             (unsigned int *startdelay, unsigned int *enddelay);
        FMOD_RESULT  setSpeakerMix        (float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright, bool calldriver = true);
        FMOD_RESULT  getSpeakerMix        (float *frontleft, float *frontright, float *center, float *lfe, float *backleft, float *backright, float *sideleft, float *sideright);
        FMOD_RESULT  setSpeakerLevels     (FMOD_SPEAKER speaker, float *levels, int numlevels, bool calldriver = true);
        FMOD_RESULT  getSpeakerLevels     (FMOD_SPEAKER speaker, float *levels, int numlevels);
        FMOD_RESULT  setInputChannelMix   (float *levels, int numlevels);
        FMOD_RESULT  getInputChannelMix   (float *levels, int numlevels);
        FMOD_RESULT  setMute              (bool mute);
        FMOD_RESULT  getMute              (bool *mute);
        FMOD_RESULT  setPriority          (int priority);
        FMOD_RESULT  getPriority          (int *priority);
        FMOD_RESULT  setPosition          (unsigned int position, FMOD_TIMEUNIT postype);
        FMOD_RESULT  getPosition          (unsigned int *position, FMOD_TIMEUNIT postype);
        FMOD_RESULT  setReverbProperties  (const FMOD_REVERB_CHANNELPROPERTIES *prop);
        FMOD_RESULT  getReverbProperties  (FMOD_REVERB_CHANNELPROPERTIES *prop);
        FMOD_RESULT  setChannelGroup      (ChannelGroupI *channelgroup);
        FMOD_RESULT  getChannelGroup      (ChannelGroupI **channelgroup);
        FMOD_RESULT  setCallback          (FMOD_CHANNEL_CALLBACKTYPE type, FMOD_CHANNEL_CALLBACK callback, int command);
        FMOD_RESULT  calculate3DReverbGain(ReverbI *reverb, FMOD_VECTOR *channelpos, float *gain);

        // 3D functionality.
        FMOD_RESULT  set3DAttributes      (const FMOD_VECTOR *pos, const FMOD_VECTOR *vel);
        FMOD_RESULT  get3DAttributes      (FMOD_VECTOR *pos, FMOD_VECTOR *vel);
        FMOD_RESULT  set3DMinMaxDistance  (float mindistance, float maxdistance);
        FMOD_RESULT  get3DMinMaxDistance  (float *mindistance, float *maxdistance);
        FMOD_RESULT  set3DConeSettings    (float insideconeangle, float outsideconeangle, float outsidevolume);
        FMOD_RESULT  get3DConeSettings    (float *insideconeangle, float *outsideconeangle, float *outsidevolume);
        FMOD_RESULT  set3DConeOrientation (FMOD_VECTOR *orientation);
        FMOD_RESULT  get3DConeOrientation (FMOD_VECTOR *orientation);
        FMOD_RESULT  set3DCustomRolloff   (FMOD_VECTOR *points, int numpoints);
        FMOD_RESULT  get3DCustomRolloff   (FMOD_VECTOR **points, int *numpoints);
		FMOD_RESULT	 set3DOcclusion       (float directOcclusion, float reverbOcclusion);
		FMOD_RESULT  get3DOcclusion       (float *directOcclusion, float *reverbOcclusion);
        FMOD_RESULT  set3DSpread          (float angle);
        FMOD_RESULT  get3DSpread          (float *angle);
        FMOD_RESULT  set3DPanLevel        (float level);
        FMOD_RESULT  get3DPanLevel        (float *level);
        FMOD_RESULT  set3DDopplerLevel    (float level);
        FMOD_RESULT  get3DDopplerLevel    (float *level);

        // DSP functionality only for channels playing sounds created with FMOD_SOFTWARE.
        FMOD_RESULT  getDSPHead           (DSPI **dsp);
        FMOD_RESULT  addDSP               (DSPI *dsp);
        FMOD_RESULT  addDSPChain          (DSPI *dsp);

        // Information only functions.
        FMOD_RESULT  isPlaying            (bool *isplaying);
        FMOD_RESULT  isVirtual            (bool *isvirtual);
        FMOD_RESULT  getAudibility        (float *audibility);
        FMOD_RESULT  getFadeVolume        (float *fadevolume);
        FMOD_RESULT  getCurrentSound      (SoundI **sound);
        FMOD_RESULT  getCurrentDSP        (DSPI  **dsp);
        FMOD_RESULT  getSpectrum          (float *spectrumarray, int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype);
        FMOD_RESULT  getWaveData          (float *wavearray, int numvalues, int channeloffset);
        FMOD_RESULT  getIndex             (int *index);

        // Functions also found in Sound class but here they can be set per channel.
        FMOD_RESULT  setMode              (FMOD_MODE mode);
        FMOD_RESULT  getMode              (FMOD_MODE *mode);
        FMOD_RESULT  setLoopCount         (int loopcount);
        FMOD_RESULT  getLoopCount         (int *loopcount);
        FMOD_RESULT  setLoopPoints        (unsigned int loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int loopend, FMOD_TIMEUNIT loopendtype);
        FMOD_RESULT  getLoopPoints        (unsigned int *loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int *loopend, FMOD_TIMEUNIT loopendtype);

        // Userdata set/get.
        FMOD_RESULT  setUserData          (void *userdata);
        FMOD_RESULT  getUserData          (void **userdata);
    };
}

#endif
