#include "fmod_settings.h"

#include "fmod_channel_emulated.h"
#include "fmod_channelpool.h"
#include "fmod_memory.h"
#include "fmod_output_emulated.h"
#include "fmod_soundi.h"
#include "fmod_string.h"

namespace FMOD
{

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: e6166020 auto
// OFFSET: 0x00922210
OutputEmulated::OutputEmulated()
{
    FMOD_memset(&mDescription, 0, sizeof(FMOD_OUTPUT_DESCRIPTION_EX));

    mChannel = 0;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 1ade2770 manual
// OFFSET: 0x00922250
FMOD_RESULT OutputEmulated::init(int maxchannels)
{
    FMOD_RESULT  result;

    if (!mSystem)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

    /*
        Create Emulated channels
    */
    if (maxchannels)
    {
        int count;

        mChannelPool = mChannelPool3D = FMOD_Object_Alloc(ChannelPool);
        if (!mChannelPool)
        {
            return FMOD_ERR_MEMORY;
        }

        result = mChannelPool->init(mSystem, this, maxchannels);
        if (result != FMOD_OK)
        {
            return result;
        }

        mChannel = (ChannelEmulated *)FMOD_Memory_Calloc(sizeof(ChannelEmulated) * maxchannels);
        if (!mChannel)
        {
            return FMOD_ERR_MEMORY;
        }

        for (count = 0; count < maxchannels; count++)
        {
            new (&mChannel[count]) ChannelEmulated;
            CHECK_RESULT(mChannelPool->setChannel(count, &mChannel[count]));
        }
    }
    
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 0b4ab641 auto
// OFFSET: 0x00922330
FMOD_RESULT OutputEmulated::release()
{
    if (mChannelPool)
    {
        mChannelPool->release();
        mChannelPool = 0;
    }
    if (mChannel)
    {    
        FMOD_Memory_Free(mChannel);
        mChannel = 0;
    }

    return Output::release();
}

	  
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputEmulated::update()
{
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
