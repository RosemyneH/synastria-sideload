#include "fmod_settings.h"

#include "fmod_listener.h"

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
// VERIFY: 87ceaddc auto
// OFFSET: 0x00921C20
Listener::Listener()
{
    mPosition.x = 0;
    mPosition.y = 0;
    mPosition.z = 0;

    mLastPosition.x = 0;
    mLastPosition.y = 0;
    mLastPosition.z = 0;

    mVelocity.x = 0;
    mVelocity.y = 0;
    mVelocity.z = 0;

    mUp.x = 0;
    mUp.y = 1.0f;
    mUp.z = 0;

    mFront.x = 0;
    mFront.y = 0;
    mFront.z = 1.0f;

    mRight.x = 1.0f;
    mRight.y = 0;
    mRight.z = 0;

}


}


