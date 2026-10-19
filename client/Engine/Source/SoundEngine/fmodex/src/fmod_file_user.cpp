#include "fmod_settings.h"

#include "fmod_debug.h"
#include "fmod_file_user.h"
#include "fmod_systemi.h"

#include <stdio.h>

namespace FMOD
{

FMOD_FILE_OPENCALLBACK  UserFile::gOpenCallback  = 0;
FMOD_FILE_CLOSECALLBACK UserFile::gCloseCallback = 0;
FMOD_FILE_READCALLBACK  UserFile::gReadCallback  = 0;
FMOD_FILE_SEEKCALLBACK  UserFile::gSeekCallback  = 0;

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 0b4c6935 manual
// OFFSET: 0x008E8230
FMOD_RESULT UserFile::reallyOpen(const char *name_or_data, unsigned int *filesize)
{
    FMOD_RESULT result = FMOD_OK;

 	if (mOpenCallback)
    {
		result = mOpenCallback(name_or_data, mUnicode, filesize, &mHandle, &mUserData);
    }
 	else if (gOpenCallback)
    {
		result = gOpenCallback(name_or_data, mUnicode, filesize, &mHandle, &mUserData);
    }

    	
    if (!mHandle)
	{
        FLOG((FMOD_DEBUG_TYPE_FILE, __FILE__, __LINE__, "UserFile::reallyOpen", "FAILED\n"));
	}

	return result;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 9cb8f498 auto
// OFFSET: 0x008E8270
FMOD_RESULT UserFile::reallyClose()
{
	if (mCloseCallback)
    {
		mCloseCallback(mHandle, mUserData);
    }
	else if (gCloseCallback)
    {
		gCloseCallback(mHandle, mUserData);
    }
    else
    {
        FLOG((FMOD_DEBUG_TYPE_FILE, __FILE__, __LINE__, "UserFile::reallyClose", "FAILED\n"));
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 0868b550 manual
// OFFSET: 0x008E82A0
FMOD_RESULT UserFile::reallyRead(void *buffer, unsigned int size, unsigned int *rd)
{
    FMOD_RESULT result = FMOD_OK;

	if (mReadCallback)
    {
		result = mReadCallback(mHandle, buffer, size, rd, mUserData);
    }
	else if (gReadCallback)
    {
		result = gReadCallback(mHandle, buffer, size, rd, mUserData);
    }
    else
    {
        FLOG((FMOD_DEBUG_TYPE_FILE, __FILE__, __LINE__, "UserFile::reallyRead", "FAILED\n"));
    }

    return result;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: eb6901d0 manual
// OFFSET: 0x008E82E0
FMOD_RESULT UserFile::reallySeek(unsigned int pos)
{
	if (mSeekCallback)
    {
        FMOD_RESULT result;

		result = mSeekCallback(mHandle, pos, mUserData);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
	else if (gSeekCallback)
    {
        FMOD_RESULT result;

		result = gSeekCallback(mHandle, pos, mUserData);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    else
    {
        FLOG((FMOD_DEBUG_TYPE_FILE, __FILE__, __LINE__, "UserFile::reallyRead", "FAILED\n"));
    }

    return FMOD_OK;
}



/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/

}

