#include "fmod_settings.h"

#include "fmod_dspnet.h"
#include "fmod_dspi.h"
#include "fmod_memory.h"
#include "fmod_os_net.h"
#include "fmod_systemi.h"

namespace FMOD
{

// VERIFY: 07adf318 auto
// OFFSET: 0x008E6750
DSPNet::DSPNet()
{
    mInitialized         = false;
    mTimeSinceLastUpdate = 0;
    mListenSocket        = 0;
    mClientSocket        = 0;
    mStack               = 0;
    mStackPos            = 0;
    mStackSize           = DSPNET_STACKSIZE;
    mPacket              = 0;
    mHeader              = 0;
    mNodes               = 0;
    mNumNodes            = 0;
}


// VERIFY: 3e9661d7 manual
// OFFSET: 0x008E68A0
FMOD_RESULT DSPNet::init()
{
    FMOD_RESULT result;

    if (mInitialized)
    {
        release();
    }

    result = FMOD_OS_Net_Init();
    if (result != FMOD_OK)
    {
        return result;
    }

    result = FMOD_OS_Net_Listen(DSPNET_PORT, &mListenSocket);
    if (result != FMOD_OK)
    {
        return result;
    }

    mStack = (DSPI **)FMOD_Memory_Alloc(mStackSize * sizeof(DSPI *));
    if (!mStack)
    {
        return FMOD_ERR_MEMORY;
    }

    mPacket = (char *)FMOD_Memory_Alloc(DSPNET_PACKETSIZE);
    if (!mPacket)
    {
        return FMOD_ERR_MEMORY;
    }

    FMOD_memset(mPacket, 0, DSPNET_PACKETSIZE);

    mHeader = (DSPNetHeader *)mPacket;
    mNodes  = (DSPNetNode *)(mHeader + 1);

    mInitialized = true;

    return FMOD_OK;
}


// VERIFY: 2c5a388c auto
// OFFSET: 0x008E6810
FMOD_RESULT DSPNet::release()
{
    if (mInitialized)
    {
        FMOD_OS_Net_Close(mListenSocket);
        mListenSocket = 0;

        FMOD_OS_Net_Close(mClientSocket);
        mClientSocket = 0;

        FMOD_OS_Net_Shutdown();

        FMOD_Memory_Free(mStack);
        mStack    = 0;
        mStackPos = 0;

        FMOD_Memory_Free(mPacket);
        mPacket   = 0;
        mHeader   = 0;
        mNodes    = 0;
        mNumNodes = 0;

        mTimeSinceLastUpdate = 0;
        mInitialized         = false;
    }

    return FMOD_OK;
}


// VERIFY: cc6d1c15 auto
// OFFSET: 0x008E6780
FMOD_RESULT DSPNet::flush(bool more)
{
    FMOD_RESULT  result;
    unsigned int byteswritten = 0;

    mHeader->mType = DSPNET_PACKET_NODES;
    mHeader->mSize = mNumNodes * sizeof(DSPNetNode);
    mHeader->mMore = more;

    result = FMOD_OS_Net_Write(mClientSocket, mPacket, mHeader->mSize + sizeof(DSPNetHeader), &byteswritten);
    if (result != FMOD_OK && result != FMOD_ERR_NET_WOULD_BLOCK)
    {
        mClientSocket = 0;
        return result;
    }

    FMOD_memset(mPacket, 0, DSPNET_PACKETSIZE);
    mNumNodes = 0;

    return FMOD_OK;
}


// VERIFY: 38253a31 auto
// OFFSET: 0x008E6950
FMOD_RESULT DSPNet::update(SystemI *system, unsigned int dt)
{
    FMOD_RESULT result;

    mTimeSinceLastUpdate += dt;

    if (!mInitialized || mTimeSinceLastUpdate < 50)
    {
        return FMOD_OK;
    }

    mTimeSinceLastUpdate = 0;

    if (!mClientSocket && FMOD_OS_Net_Accept(mListenSocket, &mClientSocket) != FMOD_OK)
    {
        mClientSocket = 0;
        return FMOD_OK;
    }

    result = system->getDSPHead(&mStack[mStackPos++]);
    if (result != FMOD_OK)
    {
        return result;
    }

    while (mStackPos > 0)
    {
        DSPI       *dsp;
        DSPNetNode *node;
        bool        found;
        int         count;

        if (mNumNodes == DSPNET_MAXNODES)
        {
            result = flush(true);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        dsp  = mStack[--mStackPos];
        node = &mNodes[mNumNodes++];

        node->mDSP = dsp;

        result = dsp->getInfo(node->mName, 0, 0, 0, 0);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = dsp->getNumInputs(&node->mNumInputs);
        if (result != FMOD_OK)
        {
            return result;
        }

        node->mActive    = (dsp->mFlags & FMOD_DSP_FLAG_ACTIVE) ? true : false;
        node->mTreeLevel = dsp->mTreeLevel;

        found = false;
        for (count = 0; count < mNumNodes - 1 && !found; count++)
        {
            if (mNodes[count].mDSP == node->mDSP)
            {
                found = true;
            }
        }

        if (!found)
        {
            for (count = node->mNumInputs - 1; count >= 0; count--)
            {
                if (mStackPos == mStackSize)
                {
                    mStackSize *= 2;
                    mStack = (DSPI **)FMOD_Memory_ReAlloc(mStack, mStackSize * sizeof(DSPI *));
                }

                result = dsp->getInput(count, &mStack[mStackPos++]);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }
        }
    }

    return flush(false);
}

}
