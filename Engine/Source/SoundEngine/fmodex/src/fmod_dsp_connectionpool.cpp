#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SOFTWARE

#include "fmod_dsp_connectionpool.h"
#include "fmod_localcriticalsection.h"
#include "fmod_memory.h"
#include "fmod_systemi.h"

namespace FMOD
{



/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/
// VERIFY: 7720d276 manual
// OFFSET: 0x00922780
FMOD_RESULT DSPConnectionPool::init(SystemI *system, int numconnections, int numoutputlevels, int numinputlevels)
{
    DSP_LEVEL_TYPE *leveldata;
    int count;
    unsigned int alignment = 0;

    if (numconnections < 0)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    for (int i=0; i < DSP_MAX_CONNECTION_BLOCKS; i++)
    {
        mConnection[i] = 0;
        mLevelData[i]  = 0;
    }

    numconnections += 128;
    numconnections /= 128;
    numconnections *= 128;

    mNumConnections = numconnections; 
    mConnectionMemory[0] = (DSPConnectionI *)FMOD_Memory_Calloc(mNumConnections * sizeof(DSPConnectionI) + 16);
    if (!mConnectionMemory[0])
    {
        return FMOD_ERR_MEMORY;
    }
    mConnection[0] = (DSPConnectionI *)FMOD_ALIGNPOINTER(mConnectionMemory[0], 16);

    mNodeMemory[0] = (LinkedListNode *)FMOD_Memory_Calloc(mNumConnections * sizeof(LinkedListNode));
    if (!mNodeMemory[0])
    {
        return FMOD_ERR_MEMORY;
    }  

    mNumOutputLevels = numoutputlevels;
    mNumInputLevels = numinputlevels;
    
    #ifdef PLATFORM_PS3
    alignment = 128;
    #endif
    mLevelDataMemory[0] = (DSP_LEVEL_TYPE *)FMOD_Memory_Calloc(mNumConnections                                          /* How many connections are we going to allow maximum. */
                                            * (mNumOutputLevels < 2 ? 2 : mNumOutputLevels)                             /* Number of rows.  This should be at least 2 for panning. */
                                            * (mNumInputLevels < mNumOutputLevels ? mNumOutputLevels : mNumInputLevels) /* Because pan matricies can be [outputlevels][outputlevels] we need to have at least this */
                                            * sizeof(DSP_LEVEL_TYPE)                                                    /* Float values of course.  Maybe lower? */
                                            * 3                                                                         /* *3 = mLevel, mLevelCurrent, mLevelDelta */
                                            + alignment                                                                 /* PS3 - 128 byte aligned for DMA */
                                            );
    if (!mLevelDataMemory[0])
    {
        return FMOD_ERR_MEMORY;
    }
    #ifdef PLATFORM_PS3
    mLevelData[0] = (DSP_LEVEL_TYPE *)FMOD_ALIGNPOINTER(mLevelDataMemory[0], 128);
    #else
    mLevelData[0] = mLevelDataMemory[0];
    #endif
    leveldata = mLevelData[0];

    mFreeListHead.initNode();
    for (count = 0; count < mNumConnections; count++)
    {
        DSPConnectionI *conn = &mConnection[0][count];

        new (conn) DSPConnectionI;

        conn->init(leveldata, numoutputlevels, numinputlevels);
        conn->mNode = &mNodeMemory[0][count];
        conn->mNode->setData(conn);
        conn->mNode->addAfter(&mFreeListHead);
    }

    mSystem = system;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/
// VERIFY: ab1649c1 auto
// OFFSET: 0x00922380
FMOD_RESULT DSPConnectionPool::close()
{
    int i;

    for (i = 0; i < DSP_MAX_CONNECTION_BLOCKS; i++)
    {
        if (mConnectionMemory[i])
        {
            FMOD_Memory_Free(mConnectionMemory[i]);
            mConnectionMemory[i] = 0;
        }
        mConnection[i] = 0;

        if (mLevelDataMemory[i])
        {
            FMOD_Memory_Free(mLevelDataMemory[i]);
            mLevelDataMemory[i] = 0;
        }
        mLevelData[i] = 0;

        if (mNodeMemory[i])
        {
            FMOD_Memory_Free(mNodeMemory[i]);
            mNodeMemory[i] = 0;
        }
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/
// VERIFY: 2b56631d manual
// OFFSET: 0x00922420
FMOD_RESULT DSPConnectionPool::alloc(DSPConnectionI **connection)
{
    DSPConnectionI *newconnection;
    unsigned int alignment = 0;

    if (!mSystem)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

    LocalCriticalSection crit(mSystem->mDSPConnectionCrit);

    if (!connection)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    crit.enter();

    if (mFreeListHead.isEmpty())
    {
        int             newblock, count;
        DSP_LEVEL_TYPE *leveldata = 0;

        for (newblock = 0; newblock < DSP_MAX_CONNECTION_BLOCKS; newblock++)
        {
            if (!mConnectionMemory[newblock])
            {
                break;
            }
        }
        if (newblock >= DSP_MAX_CONNECTION_BLOCKS)
        {
            FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "DSPConnectionPool::alloc", "DSP_MAX_CONNECTION_BLOCKS exceeded! Memory will leak!\n"));
            return FMOD_ERR_INTERNAL;
        }

        mConnectionMemory[newblock] = (DSPConnectionI *)FMOD_Memory_Calloc(mNumConnections * sizeof(DSPConnectionI) + 16);
        if (!mConnectionMemory[newblock])
        {
            return FMOD_ERR_MEMORY;
        }
        mConnection[newblock] = (DSPConnectionI *)FMOD_ALIGNPOINTER(mConnectionMemory[newblock], 16);

        #ifdef PLATFORM_PS3
        alignment = 128;
        #endif
        mNodeMemory[newblock] = (LinkedListNode *)FMOD_Memory_Calloc(mNumConnections * sizeof(LinkedListNode));
        if (!mNodeMemory[newblock])
        {
            return FMOD_ERR_MEMORY;
        }

        mLevelDataMemory[newblock] = (DSP_LEVEL_TYPE *)FMOD_Memory_Calloc(mNumConnections                                      /* How many connections are we going to allow maximum. */
                                                   * (mNumOutputLevels < 2 ? 2 : mNumOutputLevels)                             /* Number of rows.  This should be at least 2 for panning. */
                                                   * (mNumInputLevels < mNumOutputLevels ? mNumOutputLevels : mNumInputLevels) /* Because pan matricies can be [outputlevels][outputlevels] we need to have at least this */
                                                   * sizeof(DSP_LEVEL_TYPE)                                                    /* Float values of course.  Maybe lower? */
                                                   * 3                                                                         /* *3 = mLevel, mLevelCurrent, mLevelDelta */
                                                   + alignment                                                                 /* PS3 - 128 byte aligned for DMA */
                                                   );
        if (!mLevelDataMemory[newblock])
        {
            return FMOD_ERR_MEMORY;
        }
        #ifdef PLATFORM_PS3
        mLevelData[newblock] = (DSP_LEVEL_TYPE *)FMOD_ALIGNPOINTER(mLevelDataMemory[newblock], 128);
        #else
        mLevelData[newblock] = mLevelDataMemory[newblock];
        #endif
        
        leveldata = mLevelData[newblock];

        for (count = 0; count < mNumConnections; count++)
        {
            DSPConnectionI *conn = &mConnection[newblock][count];

            new (conn) DSPConnectionI;

            conn->init(leveldata, mNumOutputLevels, mNumInputLevels);
            conn->mNode = &mNodeMemory[newblock][count];
            conn->mNode->setData(conn);
            conn->mNode->addAfter(&mFreeListHead);
        }
    }

    newconnection = (DSPConnectionI *)mFreeListHead.getNext()->getData();

    newconnection->mInputNode.setData(newconnection);
    newconnection->mOutputNode.setData(newconnection);

    newconnection->mNode->removeNode();
    newconnection->mNode->addAfter(&mUsedListHead);

    crit.leave();

    *connection = newconnection;

    return FMOD_OK;
}



/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/
// VERIFY: 76921e06 manual
// OFFSET: 0x009226B0
FMOD_RESULT DSPConnectionPool::free(DSPConnectionI *connection)
{
    LocalCriticalSection crit(mSystem->mDSPConnectionCrit);

    if (!connection)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    crit.enter();

    connection->mInputNode.removeNode();
    connection->mOutputNode.removeNode();

    connection->mInputUnit = 0;
    connection->mOutputUnit = 0;

    connection->mNode->removeNode();
    connection->mNode->addAfter(&mFreeListHead);
    connection->mNode->setData(connection);

    crit.leave();

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/


}

#endif
