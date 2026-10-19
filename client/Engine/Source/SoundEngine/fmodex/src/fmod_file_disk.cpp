#include "fmod_settings.h"

#include "fmod_debug.h"
#include "fmod_file_disk.h"
#include "fmod_os_misc.h"
#include "fmod_stringw.h"
#include "fmod_systemi.h"

#include <stdio.h>
#include <stdlib.h>

namespace FMOD
{


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 6a04534c auto
// OFFSET: 0x008E80A0
FMOD_RESULT DiskFile::reallyOpen(const char *name_or_data, unsigned int *filesize)
{
    FMOD_RESULT result;
    char path[2048];

	if (mUnicode)
	{
		if (!FMOD_strlenW((short *)name_or_data))
		{
			return FMOD_ERR_FILE_NOTFOUND;
		}
	}
	else
	{
    	if (!FMOD_strlen(name_or_data))
    	{
        	return FMOD_ERR_FILE_NOTFOUND;
    	}
	}

    /*
        Expand relative path to absolute path so we can work out what drive this file is on later on
    */

#ifdef PLATFORM_WINDOWS    //!! _fullpath() not supported on xbox
    if (!_fullpath(path, name_or_data, 2047))
    {
        return FMOD_ERR_INVALID_PARAM;
    }
#else
    FMOD_strcpy(path, name_or_data);
#endif
   
    result = setName(path);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = FMOD_OS_File_Open((const char *)name_or_data, (char *)"rb", mUnicode, filesize, &mHandle);

	if (result != FMOD_OK)
	{
        FLOG((FMOD_DEBUG_TYPE_FILE, __FILE__, __LINE__, "DiskFile::reallyOpen", "Call to open failed\n"));		
		return result;
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
// VERIFY: 20c61e17 auto
// OFFSET: 0x008E8140
FMOD_RESULT DiskFile::reallyClose()
{
    FMOD_RESULT result = FMOD_OK;

    if (mHandle)
    {
	    result = FMOD_OS_File_Close(mHandle);
        mHandle = 0;
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
// VERIFY: e26cab21 auto
// OFFSET: 0x008E8170
FMOD_RESULT DiskFile::reallyRead(void *buffer, unsigned int size, unsigned int *rd)
{
    FMOD_RESULT result;
    unsigned int read;
    FMOD_UINT_NATIVE threadid = 0;
       
    FMOD_OS_Thread_GetCurrentID(&threadid);

    if (mSystem && threadid != mSystem->mMainThreadID) /* Only stall fmod reads in other threads. */
    {
        FMOD_File_SetDiskBusy(true);
    }

    result = FMOD_OS_File_Read(mHandle, buffer, size, &read);

    if (mSystem && threadid != mSystem->mMainThreadID)
    {
        FMOD_File_SetDiskBusy(false);
    }

    if (rd)
    {
        *rd = read;        
    }

    if (result == FMOD_OK && size != read)
    {
        return FMOD_ERR_FILE_EOF;
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
// VERIFY: 94911e06 auto
// OFFSET: 0x008E8200
FMOD_RESULT DiskFile::reallySeek(unsigned int pos)
{   
	return FMOD_OS_File_Seek(mHandle, pos);
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
// VERIFY: eb95faf5 auto
// OFFSET: 0x008E8220
FMOD_RESULT DiskFile::reallyCancel()
{
    return FMOD_OS_File_Cancel(mHandle);
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


