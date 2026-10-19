#include "fmod_settings.h"

#include "fmod_time.h"
#include "fmod_os_misc.h"


namespace FMOD
{

/*
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
*/
// VERIFY: fab32a67 auto
// OFFSET: 0x008E8330
TimeStamp::TimeStamp()
{
    mIn = 0;
    mOut = 0;
    mTotalIn = 0;
    mTotalOut = 0;
    mPaused = false;
    mPausedIn = 0;
    mPausedOut = 0;
    mPausedTotal = 0;
    mPausedRefCount = 0;
    mTiming = false;
}


/*
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
*/
// VERIFY: 5abd71c5 auto
// OFFSET: 0x008E8360
FMOD_RESULT TimeStamp::stampIn()
{
	FMOD_OS_Time_GetNs(&mIn);

    mTiming = true;

    return FMOD_OK;
}


/*
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
*/
// VERIFY: 18885d29 manual
// OFFSET: 0x008E8380
FMOD_RESULT TimeStamp::stampOut(int damppercentage)
{
	unsigned int  val;
	FMOD_UFLOAT   total,smoothedtotal;
    FMOD_UFLOAT   dampratio = FMOD_SCALEUP(damppercentage) / 100;

	FMOD_OS_Time_GetNs(&val);

	mOut      = val;
	mTotalOut = val;

    if (mTotalOut < mTotalIn)
    {
        total = 0;
    }
    else
    {
	    total = (FMOD_UFLOAT)(mTotalOut - mTotalIn);
    }
    
	/*
        Smooth total
    */
    mAvTotal = FMOD_SCALEDOWN(mAvTotal * dampratio);
	mAvTotal += total;											
	smoothedtotal = FMOD_SCALEDOWN(mAvTotal * (FMOD_SCALEUP(1) - dampratio));

	/*
        Smooth cpu usage
    */
	mPercent = FMOD_FMUL(mPercent, dampratio);
    if (mOut > mIn)
    {
        unsigned int delta = mOut - mIn - mPausedTotal;

	    mPercent += ((FMOD_SCALEUP(delta) / smoothedtotal) * (FMOD_UFLOAT)100);
    }

	mCPUUsage = FMOD_FMUL(mPercent, (FMOD_SCALEUP(1) - dampratio));

	mTotalIn = val;
    
    mPausedTotal = 0;
    mPausedRefCount = 0;;
    mTiming = false;

	return FMOD_OK;
}


/*
	[DESCRIPTION]
    Returns the current system time value in milliseconds.

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]
    Calls the nanosecond counter and divides by 1000.
    May wrap regularly on certain platforms, only to be used on very short deltas.

	[SEE_ALSO]
*/
FMOD_RESULT TimeStamp::getCPUUsage(FMOD_UFLOAT *cpuusage)
{
    if (!cpuusage)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *cpuusage = mCPUUsage;

    return FMOD_OK;
}


/*
	[DESCRIPTION]
    Returns the current system time value in milliseconds.

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]
    Calls the nanosecond counter and divides by 1000.
    May wrap regularly on certain platforms, only to be used on very short deltas.

	[SEE_ALSO]
*/
// VERIFY: 4a87c97f auto
// OFFSET: 0x008E8440
FMOD_RESULT TimeStamp::setPaused(bool paused)
{
    if (!mTiming)
    {
        return FMOD_OK;
    }

    if (paused)
    {
        if (!mPausedRefCount)
        {
	        FMOD_OS_Time_GetNs(&mPausedIn);
        }
        mPausedRefCount++;
    }
    else if (!paused)
    {
        mPausedRefCount--;
        if (mPausedRefCount < 0)
        {
            mPausedRefCount = mPausedRefCount;
        }

        if (!mPausedRefCount)
        {
	        FMOD_OS_Time_GetNs(&mPausedOut);

            if (mPausedOut > mPausedIn)
            {
                mPausedTotal += (mPausedOut - mPausedIn);
            }
        }
    }

    mPaused = paused;

    return FMOD_OK;
}

}
