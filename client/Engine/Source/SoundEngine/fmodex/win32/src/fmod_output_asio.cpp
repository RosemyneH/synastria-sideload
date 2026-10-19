#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_ASIO

#include "fmod.h"
#include "fmod_downmix.h"
#include "fmod_dspi.h"
#include "fmod_output_asio.h"
#include "fmod_soundi.h"
#include "fmod_systemi.h"

namespace FMOD
{

FMOD_OUTPUT_DESCRIPTION_EX asiooutput;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetOutputDescription is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_OUTPUT_DESCRIPTION_EX * F_API FMODGetOutputDescriptionEx()
    {
        return OutputASIO::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


OutputASIO *gASIOObject = 0;


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 6aa60dd5 auto
// OFFSET: 0x00937280
FMOD_OUTPUT_DESCRIPTION_EX *OutputASIO::getDescriptionEx()
{
    FMOD_memset(&asiooutput, 0, sizeof(FMOD_OUTPUT_DESCRIPTION_EX));

    asiooutput.name          = "FMOD ASIO Output";
    asiooutput.version       = 0x00010100;
    asiooutput.polling       = false;
    asiooutput.getnumdrivers = &OutputASIO::getNumDriversCallback;
    asiooutput.getdrivername = &OutputASIO::getDriverNameCallback;
    asiooutput.getdrivercaps = &OutputASIO::getDriverCapsCallback;
    asiooutput.init          = &OutputASIO::initCallback;
    asiooutput.close         = &OutputASIO::closeCallback;
    asiooutput.start         = &OutputASIO::startCallback;
    asiooutput.stop          = &OutputASIO::stopCallback;
    asiooutput.gethandle     = &OutputASIO::getHandleCallback;

    /*
        Private members
    */
#ifdef FMOD_SUPPORT_RECORDING
    asiooutput.record_getnumdrivers   = &OutputASIO::recordGetNumDriversCallback;
    asiooutput.record_getdrivername   = &OutputASIO::recordGetDriverInfoCallback;
    asiooutput.record_start           = &OutputASIO::recordStartCallback;
    asiooutput.record_stop            = &OutputASIO::recordStopCallback;
    asiooutput.record_getposition     = &OutputASIO::recordGetPositionCallback;
    asiooutput.record_lock            = &OutputASIO::recordLockCallback;
    asiooutput.record_unlock          = &OutputASIO::recordUnlockCallback;
#endif
    asiooutput.mType                  = FMOD_OUTPUTTYPE_ASIO;
    asiooutput.mSize                  = sizeof(OutputASIO);

    return &asiooutput;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: 85848840 auto
// OFFSET: 0x00936910
void OutputASIO::bufferSwitch(long index, ASIOBool processNow)
{
	ASIOTime  timeInfo;
	FMOD_memset (&timeInfo, 0, sizeof (timeInfo));

	if (ASIOGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
    {
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;
    }

	bufferSwitchTimeInfo(&timeInfo, index, processNow);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
void OutputASIO::sampleRateDidChange(ASIOSampleRate sRate)
{
    /* Gizza */
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: f2f14e3d auto
// OFFSET: 0x00935E20
long OutputASIO::asioMessage(long selector, long value, void* message, double* opt)
{
	// currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;
	switch(selector)
	{
		case kAsioSelectorSupported:
			if(value == kAsioResetRequest
			|| value == kAsioEngineVersion
			|| value == kAsioResyncRequest
			|| value == kAsioLatenciesChanged
			// the following three were added for ASIO 2.0, you don't necessarily have to support them
			|| value == kAsioSupportsTimeInfo
			|| value == kAsioSupportsTimeCode
			|| value == kAsioSupportsInputMonitor)
				ret = 1L;
			break;
		case kAsioResetRequest:
			// defer the task and perform the reset of the driver during the next "safe" situation
			// You cannot reset the driver right now, as this code is called from the driver.
			// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
			// Afterwards you initialize the driver again.
			ret = 1L;
			break;
		case kAsioResyncRequest:
			// This informs the application, that the driver encountered some non fatal data loss.
			// It is used for synchronization purposes of different media.
			// Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
			// Windows Multimedia system, which could loose data because the Mutex was hold too long
			// by another thread.
			// However a driver can issue it in other situations, too.
			ret = 1L;
			break;
		case kAsioLatenciesChanged:
			// This will inform the host application that the drivers were latencies changed.
			// Beware, it this does not mean that the buffer sizes have changed!
			// You might need to update internal delay data.
			ret = 1L;
			break;
		case kAsioEngineVersion:
			// return the supported ASIO version of the host application
			// If a host applications does not implement this selector, ASIO 1.0 is assumed
			// by the driver
			ret = 2L;
			break;
		case kAsioSupportsTimeInfo:
			// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
			// is supported.
			// For compatibility with ASIO 1.0 drivers the host application should always support
			// the "old" bufferSwitch method, too.
			ret = 1;
			break;
		case kAsioSupportsTimeCode:
			// informs the driver wether application is interested in time code info.
			// If an application does not need to know about time code, the driver has less work
			// to do.
			ret = 0;
			break;
	}
	return ret;
}


#ifdef FMOD_SUPPORT_SOFTWARE
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: 6dd60461 manual
// OFFSET: 0x00935E90
FMOD_RESULT OutputASIO::updateMixer(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess)
{
    FMOD_RESULT result;
    int count, channels;
  
    result = mSystem->getSoftwareFormat(0, 0, &channels, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return FMOD_OK;
    }

#ifdef FMOD_SUPPORT_RECORDING

    if (mRecording && mRecordSound)
    {
        /*
            Interleave ASIO record buffers to the FMOD style (normal) interleaved buffer,
        */
        for (count = 0; count < mInputChannels + mOutputChannels; count++)
	    {
		    if (mBufferInfo[count].isInput && mBufferInfo[count].channelNum < mRecordSound->mChannels)
		    {
                int     numsamples = mBufferPreferredSize;
                float  *dest       = mInterleavedRecordBuffer + (mInterleavedRecordBufferPos * mRecordSound->mChannels) + mBufferInfo[count].channelNum;
                int     count2;          

			    /*
                    Do processing for the outputs only.
                    ASIO works with individual mono channels, (ie 1 for left and 1 for right), so it has to be
                    de-interleaved from our interleaved stereo stream.
                */
			    switch (mChannelInfo[count].type)
			    {
			        case ASIOSTInt16LSB:
                    {
                        DSPI::convert(dest, mBufferInfo[count].buffers[doubleBufferIndex], mRecordFormat, FMOD_SOUND_FORMAT_PCM16, numsamples, mRecordSound->mChannels, 1, 1.0f);
				        break;
                    }
			        case ASIOSTInt24LSB:		// used for 20 bits as well
                    {
                        DSPI::convert(dest, mBufferInfo[count].buffers[doubleBufferIndex], mRecordFormat, FMOD_SOUND_FORMAT_PCM24, numsamples, mRecordSound->mChannels, 1, 1.0f);
				        break;
                    }
			        case ASIOSTInt32LSB:
                    {
                        DSPI::convert(dest, mBufferInfo[count].buffers[doubleBufferIndex], mRecordFormat, FMOD_SOUND_FORMAT_PCM32, numsamples, mRecordSound->mChannels, 1, 1.0f);
				        break;
                    }
			        case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
                    {
                        DSPI::convert(dest, mBufferInfo[count].buffers[doubleBufferIndex], mRecordFormat, FMOD_SOUND_FORMAT_PCMFLOAT, numsamples, mRecordSound->mChannels, 1, 1.0f);
                        break;
                    }
			        case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
                    {
                        double *src = (double *)mBufferInfo[count].buffers[doubleBufferIndex];

                        for (count2=0; count2<numsamples; count2++)
                        {
                            dest[count2] = (float)src[count2 * mRecordSound->mChannels];
                        }
				        break;
				        // these are used for 32 bit data buffer, with different alignment of the data inside
				        // 32 bit PCI bus systems can more easily used with these
                    }
			        case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
			        case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
			        case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
			        case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
                    {
                        void *src = mBufferInfo[count].buffers[doubleBufferIndex];

				        FMOD_memset(dest, 0, numsamples * 4);
				        break;
                    }


                    /*
                        MSB
                    */

			        case ASIOSTInt16MSB:
                    {
                        DSPI::convert(dest, mBufferInfo[count].buffers[doubleBufferIndex], mRecordFormat, FMOD_SOUND_FORMAT_PCM16, numsamples, mRecordSound->mChannels, 1, 1.0f);
				        break;
                    }
			        case ASIOSTInt24MSB:		// used for 20 bits as well
                    {
                        DSPI::convert(dest, mBufferInfo[count].buffers[doubleBufferIndex], mRecordFormat, FMOD_SOUND_FORMAT_PCM24, numsamples, mRecordSound->mChannels, 1, 1.0f);
				        break;
                    }
			        case ASIOSTInt32MSB:
                    {
                        DSPI::convert(dest, mBufferInfo[count].buffers[doubleBufferIndex], mRecordFormat, FMOD_SOUND_FORMAT_PCM32, numsamples, mRecordSound->mChannels, 1, 1.0f);
				        break;
                    }
			        case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
                    {
                        DSPI::convert(dest, mBufferInfo[count].buffers[doubleBufferIndex], mRecordFormat, FMOD_SOUND_FORMAT_PCMFLOAT, numsamples, mRecordSound->mChannels, 1, 1.0f);
                        break;
                    }
			        case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
                    {
                        double *src = (double *)mBufferInfo[count].buffers[doubleBufferIndex];

                        for (count2=0; count2<numsamples; count2++)
                        {
                            dest[count2] = (float)src[count2 * mRecordSound->mChannels];
                        }
				        break;
				        // these are used for 32 bit data buffer, with different alignment of the data inside
				        // 32 bit PCI bus systems can more easily used with these
                    }
			        case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
			        case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
			        case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
			        case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
                    {
                        void *src = mBufferInfo[count].buffers[doubleBufferIndex];

				        FMOD_memset(dest, 0, numsamples * 4);
				        break;
                    }
			    }
		    }
	    }

        mInterleavedRecordBufferPos += mBufferPreferredSize;
        if (mInterleavedRecordBufferPos >= (unsigned int)mRecordBufferLength)
        {
            if (mRecordLoop)
            {
                mInterleavedRecordBufferPos -= (unsigned int)mRecordBufferLength;
            }
            else
            {
                mRecording = false;
            }
        }

    }
#endif

    /*
        Update the mixer to the interleaved buffer.
    */
    result = mix(mInterleavedBuffer, mBufferPreferredSize);
    if (result != FMOD_OK)
    {
        return FMOD_OK;
    }

    /*
        Now de-interleave the mixed result to the ASIO buffers.
    */
    int speaker = 0;
    for (count = 0; count < mInputChannels + mOutputChannels; count++)
	{
		if (!mBufferInfo[count].isInput && mBufferInfo[count].channelNum < channels)
		{
            int     numsamples  = mBufferPreferredSize;
            float  *src = 0;
            int     count2;          

            if (mSystem->mASIOSpeakerList[speaker] != FMOD_SPEAKER_NULL)
            {
                src = mInterleavedBuffer + (int)mSystem->mASIOSpeakerList[speaker];
            }
            speaker++;

			/*
                Do processing for the outputs only.
                ASIO works with individual mono channels, (ie 1 for left and 1 for right), so it has to be
                de-interleaved from our interleaved stereo stream.
            */
			switch (mChannelInfo[count].type)
			{
			    case ASIOSTInt16LSB:
                {
                    if (src)
                    {
                        DSPI::convert(mBufferInfo[count].buffers[doubleBufferIndex], src, FMOD_SOUND_FORMAT_PCM16, FMOD_SOUND_FORMAT_PCMFLOAT, numsamples, 1, channels, 1.0f);
                    }
                    else
                    {
                        void *dest = mBufferInfo[count].buffers[doubleBufferIndex];
				        FMOD_memset(dest, 0, numsamples * 2);
                    }
				    break;
                }
			    case ASIOSTInt24LSB:		// used for 20 bits as well
                {
                    if (src)
                    {
                        DSPI::convert(mBufferInfo[count].buffers[doubleBufferIndex], src, FMOD_SOUND_FORMAT_PCM24, FMOD_SOUND_FORMAT_PCMFLOAT, numsamples, 1, channels, 1.0f);
                    }
                    else
                    {
                        void *dest = mBufferInfo[count].buffers[doubleBufferIndex];
				        FMOD_memset(dest, 0, numsamples * 3);
                    }
				    break;
                }
			    case ASIOSTInt32LSB:
                {
                    if (src)
                    {
                        DSPI::convert(mBufferInfo[count].buffers[doubleBufferIndex], src, FMOD_SOUND_FORMAT_PCM32, FMOD_SOUND_FORMAT_PCMFLOAT, numsamples, 1, channels, 1.0f);
                    }
                    else
                    {
                        void *dest = mBufferInfo[count].buffers[doubleBufferIndex];
				        FMOD_memset(dest, 0, numsamples * 4);
                    }
				    break;
                }
			    case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
                {
                    if (src)
                    {
                        DSPI::convert(mBufferInfo[count].buffers[doubleBufferIndex], src, FMOD_SOUND_FORMAT_PCMFLOAT, FMOD_SOUND_FORMAT_PCMFLOAT, numsamples, 1, channels, 1.0f);
                    }
                    else
                    {
                        void *dest = mBufferInfo[count].buffers[doubleBufferIndex];
				        FMOD_memset(dest, 0, numsamples * 4);
                    }
                    break;
                }
			    case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
                {
                    if (src)
                    {
                        double *dest = (double *)mBufferInfo[count].buffers[doubleBufferIndex];

                        for (count2=0; count2<numsamples; count2++)
                        {
                            dest[count2] = (double)src[count2 * channels];
                        }
                    }
                    else
                    {
                        void *dest = mBufferInfo[count].buffers[doubleBufferIndex];
				        FMOD_memset(dest, 0, numsamples * 8);
                    }
				    break;
				    // these are used for 32 bit data buffer, with different alignment of the data inside
				    // 32 bit PCI bus systems can more easily used with these
                }
			    case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
			    case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
			    case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
			    case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
                {
                    void *dest = mBufferInfo[count].buffers[doubleBufferIndex];
				    FMOD_memset(dest, 0, numsamples * 4);
				    break;
                }


                /*
                    MSB
                */

			    case ASIOSTInt16MSB:
                {
                    if (src)
                    {
                        DSPI::convert(mBufferInfo[count].buffers[doubleBufferIndex], src, FMOD_SOUND_FORMAT_PCM16, FMOD_SOUND_FORMAT_PCMFLOAT, numsamples, 1, channels, 1.0f);
                    }
                    else
                    {
                        void *dest = mBufferInfo[count].buffers[doubleBufferIndex];
				        FMOD_memset(dest, 0, numsamples * 2);
                    }
				    break;
                }
			    case ASIOSTInt24MSB:		// used for 20 bits as well
                {
                    if (src)
                    {
                        DSPI::convert(mBufferInfo[count].buffers[doubleBufferIndex], src, FMOD_SOUND_FORMAT_PCM24, FMOD_SOUND_FORMAT_PCMFLOAT, numsamples, 1, channels, 1.0f);
                    }
                    else
                    {
                        void *dest = mBufferInfo[count].buffers[doubleBufferIndex];
				        FMOD_memset(dest, 0, numsamples * 3);
                    }
				    break;
                }
			    case ASIOSTInt32MSB:
                {
                    if (src)
                    {
                        DSPI::convert(mBufferInfo[count].buffers[doubleBufferIndex], src, FMOD_SOUND_FORMAT_PCM32, FMOD_SOUND_FORMAT_PCMFLOAT, numsamples, 1, channels, 1.0f);
                    }
                    else
                    {
                        void *dest = mBufferInfo[count].buffers[doubleBufferIndex];
				        FMOD_memset(dest, 0, numsamples * 4);
                    }
				    break;
                }
			    case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
                {
                    if (src)
                    {
                        DSPI::convert(mBufferInfo[count].buffers[doubleBufferIndex], src, FMOD_SOUND_FORMAT_PCMFLOAT, FMOD_SOUND_FORMAT_PCMFLOAT, numsamples, 1, channels, 1.0f);
                    }
                    else
                    {
                        void *dest = mBufferInfo[count].buffers[doubleBufferIndex];
				        FMOD_memset(dest, 0, numsamples * 4);
                    }
                    break;
                }
			    case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
                {
                    if (src)
                    {
                        double *dest = (double *)mBufferInfo[count].buffers[doubleBufferIndex];

                        for (count2=0; count2<numsamples; count2++)
                        {
                            dest[count2] = (double)src[count2 * channels];
                        }
                    }
                    else
                    {
                        void *dest = mBufferInfo[count].buffers[doubleBufferIndex];
				        FMOD_memset(dest, 0, numsamples * 8);
                    }
				    break;
				    // these are used for 32 bit data buffer, with different alignment of the data inside
				    // 32 bit PCI bus systems can more easily used with these
                }
			    case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
			    case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
			    case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
			    case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
                {
                    void *dest = mBufferInfo[count].buffers[doubleBufferIndex];

				    FMOD_memset(dest, 0, numsamples * 4);
				    break;
                }
			}
		}
	}

	/*
        Finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
    */
	if (mPostOutput)
    {
        ASIOOutputReady();
    }

    return FMOD_OK;
}
#endif


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: 8399e672 auto
// OFFSET: 0x009363C0
ASIOTime * OutputASIO::bufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess)
{
#ifdef FMOD_SUPPORT_SOFTWARE
    gASIOObject->updateMixer(params, doubleBufferIndex, directProcess);
#endif

    return 0;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: d572cc76 manual
// OFFSET: 0x009363E0
FMOD_RESULT OutputASIO::getNumDrivers(int *numdrivers)
{
    /*
        Set up gGlobal - for debug / file / memory access by this plugin.
    */
    Plugin::init();

    if (!mHandle)
    {
        mHandle = FMOD_Object_Calloc(AsioDrivers);
        if (!mHandle)
        {
            return FMOD_ERR_MEMORY;
        }
    }

    *numdrivers = mHandle->asioGetNumDev();

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
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: 7a9c1173 manual
// OFFSET: 0x00936450
FMOD_RESULT OutputASIO::getDriverName(int driver, char *name, int namelen)
{
    Plugin::init(); /* Set up gGlobal - for debug / file / memory access by this plugin. */

    if (!mHandle)
    {
        mHandle = FMOD_Object_Calloc(AsioDrivers);
        if (!mHandle)
        {
            return FMOD_ERR_MEMORY;
        }
    }

    mHandle->asioGetDriverName(driver, name, namelen);

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
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: 884883f8 manual
// OFFSET: 0x009364C0
FMOD_RESULT OutputASIO::getDriverCaps(int id, FMOD_CAPS *caps)
{
    Plugin::init(); /* Set up gGlobal - for debug / file / memory access by this plugin. */

    if (!mHandle)
    {
        mHandle = FMOD_Object_Calloc(AsioDrivers);
        if (!mHandle)
        {
            return FMOD_ERR_MEMORY;
        }
    }

    if (mHandle->asioGetNumDev() <= 0)
    {
		FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "OutputASIO::getDriverCaps", "Error - No sound devices!\n"));
		return FMOD_ERR_PLUGIN_RESOURCE;
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
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: b7a86198 manual
// OFFSET: 0x00936970
FMOD_RESULT OutputASIO::init(int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata)
{
    ASIODriverInfo      asioinf;
    ASIOError           err;
    ASIOSampleRate      mixratef;
    ASIOBufferInfo     *info;
    char                name[256];
    int                 offset = 0;
    int                 count;

    Plugin::init(); /* Set up gGlobal - for debug / file / memory access by this plugin. */

	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputASIO::init", "Initializing.\n"));

    if (!mHandle)
    {
        mHandle = FMOD_Object_Calloc(AsioDrivers);
        if (!mHandle)
        {
            return FMOD_ERR_MEMORY;
        }
    }

    if (mHandle->asioGetNumDev() <= 0)
    {
		FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputASIO::init", "Error - No sound devices!\n"));
		return FMOD_ERR_OUTPUT_INIT;
	}

    if (selecteddriver < 0)
    {
        selecteddriver = 0;
    }

    mHandle->asioGetDriverName(selecteddriver, name, 256);

    FMOD_memset(&asioinf, 0, sizeof(ASIODriverInfo));
    asioinf.sysRef = (void *)extradriverdata;

    err = mHandle->loadDriver(name);
    if (!err)
    {
    	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputASIO::init", "loadDriver failed using %s as the driver name (%d).\n", name, err));
        return FMOD_ERR_OUTPUT_INIT;
    }


    *outputformat = FMOD_SOUND_FORMAT_PCMFLOAT;  /* Change ASIO mix output to float to handle all ASIO types */

    /*
        Initialize ASIO.
    */
    err = ASIOInit(&asioinf);
    if (err != ASE_OK)
    {
    	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputASIO::init", "ASIOInit failed (%d).\n", err));
        return FMOD_ERR_OUTPUT_INIT;
    }

   
	err = ASIOGetBufferSize(&mBufferMinSize, &mBufferMaxSize, &mBufferPreferredSize, &mBufferGranularity);
    if (err != ASE_OK)
    {
    	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputASIO::init", "ASIOGetBufferSize failed (%d).\n", err));
        return FMOD_ERR_OUTPUT_INIT;
    }

	/*
        Get the sample rate from the control panel.
    */
	err = ASIOGetSampleRate(&mixratef);
    if (err != ASE_OK)
    {
    	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputASIO::init", "ASIOGetSampleRate failed (%d).\n", err));
        return FMOD_ERR_OUTPUT_INIT;
    }

    *outputrate = (int)mixratef;

	if (ASIOOutputReady() == ASE_OK)
    {
		mPostOutput = true;
    }
	else
    {
		mPostOutput = false;
    }

	err = ASIOGetChannels(&mInputChannels, &mOutputChannels);
    if (err != ASE_OK)
    {
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputASIO::init", "ASIOGetChannels failed (%d).\n", err));
        return FMOD_ERR_OUTPUT_INIT;
    }

    /*
        Allocate the asio buffer and channel infos for all inputs and outputs.
    */
    mBufferInfo = (ASIOBufferInfo *)FMOD_Memory_Calloc(sizeof(ASIOBufferInfo) * (mInputChannels + mOutputChannels));
    if (!mBufferInfo)
    {
        return FMOD_ERR_MEMORY;
    }

    mChannelInfo = (ASIOChannelInfo *)FMOD_Memory_Calloc(sizeof(ASIOChannelInfo) * (mInputChannels + mOutputChannels));
    if (!mChannelInfo)
    {
        FMOD_Memory_Free(mBufferInfo);
        mBufferInfo = NULL;
        return FMOD_ERR_MEMORY;
    }

    /*
        Allocate an interleaved output buffer for the mixer, that we can deinterleave later in the asio callback.
    */
    {
        unsigned int lenbytes;
        
        SoundI::getBytesFromSamples(mBufferPreferredSize, &lenbytes, outputchannels, *outputformat);

        mInterleavedBuffer = (float *)FMOD_Memory_Calloc(lenbytes);
        if (!mInterleavedBuffer)
        {
            FMOD_Memory_Free(mBufferInfo);
            FMOD_Memory_Free(mChannelInfo);

            mBufferInfo = NULL;
            mChannelInfo = NULL;

            return FMOD_ERR_MEMORY;
        }
    }


    /*
        Prepare output channels
    */
    info = mBufferInfo;

	for (count = 0; count < mInputChannels; count++, info++)
	{
		info->isInput    = ASIOTrue;
		info->channelNum = count;
		info->buffers[0] = info->buffers[1] = 0;
	}
	for (count = 0; count < mOutputChannels; count++, info++)
	{
		info->isInput    = ASIOFalse;
		info->channelNum = count;
		info->buffers[0] = info->buffers[1] = 0;
	}  

	mCallbacks.bufferSwitch          = bufferSwitch;
	mCallbacks.sampleRateDidChange   = sampleRateDidChange;
	mCallbacks.asioMessage           = asioMessage;
	mCallbacks.bufferSwitchTimeInfo  = bufferSwitchTimeInfo;

    gASIOObject = this;

    err = ASIOCreateBuffers(mBufferInfo, (mInputChannels + mOutputChannels), mBufferPreferredSize, &mCallbacks);
    if (err != ASE_OK)
    {
        FMOD_Memory_Free(mBufferInfo);
        FMOD_Memory_Free(mChannelInfo);
        FMOD_Memory_Free(mInterleavedBuffer);

        mBufferInfo = NULL;
        mChannelInfo = NULL;
        mInterleavedBuffer = NULL;

        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputASIO::init", "ASIOCreateBuffers failed (%d).\n", err));

        return FMOD_ERR_OUTPUT_CREATEBUFFER;
    }

    mSystem->mAdvancedSettings.ASIOChannelList = (char **)FMOD_Memory_Calloc(sizeof(char *) * (mInputChannels + mOutputChannels));
    if (!mSystem->mAdvancedSettings.ASIOChannelList)
    {
        return FMOD_ERR_MEMORY;
    }

    mSystem->mAdvancedSettings.ASIONumChannels = 0;
    
    // now get all the buffer details, sample word length, name, word clock group and activation
	for (count = 0; count < (mInputChannels + mOutputChannels); count++)
	{
		mChannelInfo[count].channel = mBufferInfo[count].channelNum;
		mChannelInfo[count].isInput = mBufferInfo[count].isInput;
		err = ASIOGetChannelInfo(&mChannelInfo[count]);
        if (err != ASE_OK)
        {
            break;
        }
     
        if (!mChannelInfo[count].isInput)
        {
            mSystem->mAdvancedSettings.ASIOChannelList[mSystem->mAdvancedSettings.ASIONumChannels++] = mChannelInfo[count].name;
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
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: 0a4390a4 auto
// OFFSET: 0x00936530
FMOD_RESULT OutputASIO::close()
{
    if (mBufferInfo)
    {
        ASIODisposeBuffers();
        FMOD_Memory_Free(mBufferInfo);
    	mBufferInfo = NULL;
    }

    if (mChannelInfo)
    {
        FMOD_Memory_Free(mChannelInfo);
        mChannelInfo = 0;
    }

    ASIOExit();

    if (mInterleavedBuffer)
    {
        FMOD_Memory_Free(mInterleavedBuffer);
        mInterleavedBuffer = 0;
    }

    mHandle->close();

    FMOD_Memory_Free(mHandle);
    mHandle = 0;

    if(mSystem->mAdvancedSettings.ASIOChannelList)
    {
        FMOD_Memory_Free(mSystem->mAdvancedSettings.ASIOChannelList);
        mSystem->mAdvancedSettings.ASIOChannelList = 0;
        mSystem->mAdvancedSettings.ASIONumChannels = 0;
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
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputASIO::getHandle(void **handle)
{
    *handle = mHandle;
    
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
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputASIO::start()
{
    ASIOError err;
    
    err = ASIOStart();

    if (err != ASE_OK)
    {
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputASIO::start", "ASIOStart failed (%d).\n", err));

        return FMOD_ERR_OUTPUT_DRIVERCALL;
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
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputASIO::stop()
{
    ASIOError err = ASE_OK;
    
    err = ASIOStop();

    if (err != ASE_OK)
    {
        /*
            FIXME: Errorcode?
        */
        return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

    return FMOD_OK;
}


#ifdef FMOD_SUPPORT_RECORDING
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 02391b59 auto
// OFFSET: 0x00936630
FMOD_RESULT OutputASIO::recordGetNumDrivers(int *numdrivers)
{
    if (!mHandle)
    {
        mHandle = FMOD_Object_Calloc(AsioDrivers);
        if (!mHandle)
        {
            return FMOD_ERR_MEMORY;
        }
    }

    if (!numdrivers)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *numdrivers = 1;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]
    ASIO does not provide GUIDs

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: bdfdcbad manual
// OFFSET: 0x009366A0
FMOD_RESULT OutputASIO::recordGetDriverInfo(int id, char *name, int namelen)
{
    if (!name || namelen < 1)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (id)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!mHandle)
    {
        mHandle = FMOD_Object_Calloc(AsioDrivers);
        if (!mHandle)
        {
            return FMOD_ERR_MEMORY;
        }
    }

    mHandle->asioGetDriverName(id, name, namelen);

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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 113e6fa4 auto
// OFFSET: 0x00936F50
FMOD_RESULT OutputASIO::recordStart(int id, Sound *sound, bool loop)
{
    SoundI *soundi;
    unsigned int lenbytes;

    soundi = (SoundI *)sound;
    if (!soundi)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    mRecordBufferLength = mBufferPreferredSize * 2;
    mRecordFormat       = FMOD_SOUND_FORMAT_PCMFLOAT;
    mRecordLoop         = loop;

    SoundI::getBytesFromSamples(mRecordBufferLength, &lenbytes, soundi->mChannels, FMOD_SOUND_FORMAT_PCMFLOAT);

    mInterleavedRecordBuffer = (float *)FMOD_Memory_Calloc(lenbytes);
    if (!mInterleavedRecordBuffer)
    {
        return FMOD_ERR_MEMORY;
    }

    mInterleavedRecordBufferPos = 0;

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
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputASIO::recordStop()
{
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
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputASIO::recordGetPosition(unsigned int *pcm)
{
    if (!pcm)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *pcm = mInterleavedRecordBufferPos;

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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 76836e71 auto
// OFFSET: 0x00936FD0
FMOD_RESULT OutputASIO::recordLock(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
    FMOD_RESULT result;
    char *src = 0;
    unsigned int lenbytes;

    if (!mRecordSound)
    {
        return FMOD_ERR_INTERNAL;
    }

    result = SoundI::getBytesFromSamples(mRecordBufferLength, &lenbytes, mRecordSound->mChannels, mRecordFormat);
    if (result != FMOD_OK)
    {
        return result;
    }
        
    src = (char *)mInterleavedRecordBuffer;
        
    if (offset >= lenbytes || offset < 0 || length < 0) 
	{
		*ptr1 = 0;
		*ptr2 = 0;
		*len1 = 0;
		*len2 = 0;
		return FMOD_ERR_INVALID_PARAM;
	}

	if (offset + length <= lenbytes)
	{
        *ptr1 = src + offset;
		*len1 = length;
		*ptr2 = 0;
		*len2 = 0;
	}
	/*
        Otherwise return wrapped pointers in pt1 and ptr2
    */
	else
	{
        *ptr1 = src + offset;
		*len1 = lenbytes - offset;
		*ptr2 = src;
		*len2 = length - (lenbytes - offset);
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
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputASIO::recordUnlock(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
    return FMOD_OK;
}
#endif

/*
    ==============================================================================================================

    CALLBACK INTERFACE

    ==============================================================================================================
*/


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: 224e02da manual
// OFFSET: 0x00936740
FMOD_RESULT F_CALLBACK OutputASIO::getNumDriversCallback(FMOD_OUTPUT_STATE *output, int *numdrivers)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->getNumDrivers(numdrivers);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: c877aaeb auto
// OFFSET: 0x00936770
FMOD_RESULT F_CALLBACK OutputASIO::getDriverNameCallback(FMOD_OUTPUT_STATE *output, int id, char *name, int namelen)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->getDriverName(id, name, namelen);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: 99e55a2a manual
// OFFSET: 0x009367A0
FMOD_RESULT OutputASIO::getDriverCapsCallback(FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->getDriverCaps(id, caps);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: 72068220 auto
// OFFSET: 0x009371D0
FMOD_RESULT F_CALLBACK OutputASIO::initCallback(FMOD_OUTPUT_STATE *output, int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->init(selecteddriver, flags, outputrate, outputchannels, outputformat, dspbufferlength, dspnumbuffers, extradriverdata);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: 9a2fcacd manual
// OFFSET: 0x009367E0
FMOD_RESULT F_CALLBACK OutputASIO::closeCallback(FMOD_OUTPUT_STATE *output)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->close();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: dc631099 auto
// OFFSET: 0x00936810
FMOD_RESULT F_CALLBACK OutputASIO::startCallback(FMOD_OUTPUT_STATE *output)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->start();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: da703dd6 auto
// OFFSET: 0x00936820
FMOD_RESULT F_CALLBACK OutputASIO::stopCallback(FMOD_OUTPUT_STATE *output)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->stop();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
// VERIFY: 7c6abd62 manual
// OFFSET: 0x00936830
FMOD_RESULT F_CALLBACK OutputASIO::getHandleCallback(FMOD_OUTPUT_STATE *output, void **handle)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->getHandle(handle);
}


#ifdef FMOD_SUPPORT_RECORDING
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: dd08bc52 manual
// OFFSET: 0x00936870
FMOD_RESULT F_CALLBACK OutputASIO::recordGetNumDriversCallback(FMOD_OUTPUT_STATE *output, int *numdrivers)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->recordGetNumDrivers(numdrivers);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 085f0c4a auto
// OFFSET: 0x009368A0
FMOD_RESULT F_CALLBACK OutputASIO::recordGetDriverInfoCallback(FMOD_OUTPUT_STATE *output, int id, char *name, int namelen)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->recordGetDriverInfo(id, name, namelen);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 7054a8bd auto
// OFFSET: 0x00937210
FMOD_RESULT F_CALLBACK OutputASIO::recordStartCallback(FMOD_OUTPUT_STATE *output, int id, FMOD_SOUND *sound, int loop)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->recordStart(id, (Sound *)sound, loop ? true : false);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK OutputASIO::recordStopCallback(FMOD_OUTPUT_STATE *output)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->recordStop();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: e4008fc3 manual
// OFFSET: 0x009368D0
FMOD_RESULT F_CALLBACK OutputASIO::recordGetPositionCallback(FMOD_OUTPUT_STATE *output, unsigned int *pcm)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->recordGetPosition(pcm);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 667bf9fe auto
// OFFSET: 0x00937240
FMOD_RESULT F_CALLBACK OutputASIO::recordLockCallback(FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->recordLock(offset, length, ptr1, ptr2, len1, len2);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK OutputASIO::recordUnlockCallback(FMOD_OUTPUT_STATE *output, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
    OutputASIO *asio = (OutputASIO *)output;

    return asio->recordUnlock(ptr1, ptr2, len1, len2);
}
#endif

}

#endif
