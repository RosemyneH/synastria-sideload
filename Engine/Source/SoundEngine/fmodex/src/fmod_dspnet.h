#ifndef _FMOD_DSPNET_H
#define _FMOD_DSPNET_H

#include "fmod_settings.h"

#include "fmod.h"

namespace FMOD
{
    class DSPI;
    class SystemI;

    #define DSPNET_PORT         9264
    #define DSPNET_MAXNODES     50
    #define DSPNET_STACKSIZE    50
    #define DSPNET_PACKET_NODES 3

    struct DSPNetHeader
    {
        int             mType;
        int             mSize;
        int             mMore;
    };

    #pragma pack(push, 1)
    struct DSPNetNode
    {
        DSPI           *mDSP;
        char            mName[32];
        int             mNumInputs;
        bool            mActive;
        int             mTreeLevel;
    };
    #pragma pack(pop)

    #define DSPNET_PACKETSIZE   (sizeof(DSPNetHeader) + (DSPNET_MAXNODES * sizeof(DSPNetNode)))

    class DSPNet
    {
      private:

        bool            mInitialized;
        int             mTimeSinceLastUpdate;
        void           *mListenSocket;
        void           *mClientSocket;
        DSPI          **mStack;
        int             mStackPos;
        int             mStackSize;
        char           *mPacket;
        DSPNetHeader   *mHeader;
        DSPNetNode     *mNodes;
        int             mNumNodes;

        FMOD_RESULT     flush(bool more);

      public:

        DSPNet();

        FMOD_RESULT     init();
        FMOD_RESULT     release();
        FMOD_RESULT     update(SystemI *system, unsigned int dt);
    };
}

#endif
