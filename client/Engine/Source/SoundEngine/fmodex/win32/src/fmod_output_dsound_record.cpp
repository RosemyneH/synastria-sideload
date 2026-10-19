#include "fmod_settings.h"

#if defined(FMOD_SUPPORT_DSOUND) && defined(FMOD_SUPPORT_RECORDING)

#include "fmod.hpp"
#include "fmod_output_dsound.h"
#include "fmod_soundi.h"
#include "fmod_codec_wav.h"

#include <dsound.h>

namespace FMOD
{

static const FMOD_GUID _KSDATAFORMAT_SUBTYPE_PCM            = { 0x00000001, 0x0000, 0x0010, { 0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71} };
static const FMOD_GUID _KSDATAFORMAT_SUBTYPE_IEEE_FLOAT     = { 0x00000003, 0x0000, 0x0010, { 0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71} };


/*
[
	[DESCRIPTION]
    Callback to enumerate each found input driver

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]
    The first enumerated driver is skipped as it has no GUID, (it's a duplicate
    anyway), the next driver enumerated is always the default, therefore it will be
    in element 0 of FMODs driver list as required.

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: c26a4e03 manual
// OFFSET: 0x009512C0
BOOL CALLBACK FMOD_Output_DSound_RecordEnumProc(LPGUID lpGUID, LPCSTR lpszDesc, LPCSTR lpszDrvName, LPVOID lpContext)
{
    FMOD::OutputDSound *outputdsound = (FMOD::OutputDSound *)lpContext;

	if (outputdsound->mRecordNumDrivers >= FMOD_OUTPUT_MAXDRIVERS)
	{
        return TRUE;
    }

    if (!lpGUID)
    {
        return TRUE;
    }

    outputdsound->mRecordDriverName[outputdsound->mRecordNumDrivers] = (char *)FMOD_Memory_Calloc(FMOD_strlen(lpszDesc) + 1);
    if (!outputdsound->mRecordDriverName[outputdsound->mRecordNumDrivers])
    {
        return TRUE;
    }

    strncpy(outputdsound->mRecordDriverName[outputdsound->mRecordNumDrivers], lpszDesc, FMOD_strlen(lpszDesc));

    FMOD_memcpy(&outputdsound->mRecordGUID[outputdsound->mRecordNumDrivers], lpGUID, sizeof(GUID));

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "FMOD_Output_DSound_RecordEnumProc", "Enumerating \"%s\"\n", lpszDesc));

    outputdsound->mRecordNumDrivers++;

	return TRUE;
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
// VERIFY: 857dae51 auto
// OFFSET: 0x009513A0
FMOD_RESULT OutputDSound::recordEnumerate()
{
    FMOD_RESULT result;

    result = registerDLL();
    if (result != FMOD_OK)
    {
        return result;
    }

    for (int i = 0; i < mRecordNumDrivers; i++)
    {
         if (mRecordDriverName[i])
         {
             FMOD_Memory_Free(mRecordDriverName[i]);
             mRecordDriverName[i] = NULL;
         }
    }
    mRecordNumDrivers = 0;

	if (mDirectSoundCaptureEnumerate)
    {
		(*mDirectSoundCaptureEnumerate)(FMOD_Output_DSound_RecordEnumProc, (void *)this);
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
// VERIFY: e1ed72fd auto
// OFFSET: 0x00951420
FMOD_RESULT OutputDSound::recordGetNumDrivers(int *numdrivers)
{
    FMOD_RESULT result;

    if (!numdrivers)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = recordEnumerate();
    if (result != FMOD_OK)
    {
        return result;
    }

    *numdrivers = mRecordNumDrivers;

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
// VERIFY: 529a81b2 manual
// OFFSET: 0x00951450
FMOD_RESULT OutputDSound::recordGetDriverInfo(int id, char *name, int namelen)
{
    FMOD_RESULT result;

    if (!name || namelen < 1)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = recordEnumerate();
    if (result != FMOD_OK)
    {
        return result;
    }

    if (id < 0 || id >= mRecordNumDrivers)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    FMOD_strncpy(name, mRecordDriverName[id], namelen - 1);

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
// VERIFY: ab84f2e9 manual
// OFFSET: 0x009514B0
FMOD_RESULT OutputDSound::recordStart(int id, Sound *sound, bool loop)
{
    FMOD_RESULT                 result;
	DSCBUFFERDESC               dscbd;
    WAVE_FORMATEXTENSIBLE       wfx;
    SoundI                     *soundi;
    int                         bits;
    int                         channels;
    int                         rate;
	HRESULT                     hr;

    soundi = (SoundI *)sound;
    if (!soundi)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

	result = registerDLL();
    if (result != FMOD_OK)
    {
        return result;
    }

	if (!mDirectSoundCaptureCreate)
	{
		return FMOD_ERR_OUTPUT_DRIVERCALL;
	}

	hr = (*mDirectSoundCaptureCreate)(&mRecordGUID[id], &mDirectSoundCapture, 0);
	if (hr != DS_OK)
	{
		return FMOD_ERR_OUTPUT_DRIVERCALL;
	}

    mRecordFormat       = soundi->mFormat;
    channels            = soundi->mChannels;
    mRecordBufferLength = soundi->mLength;
    rate                = (int)soundi->mDefaultFrequency;

    SoundI::getBitsFromFormat(mRecordFormat, &bits);

    FMOD_memset(&wfx, 0, sizeof(WAVE_FORMATEXTENSIBLE));

    wfx.Format.wFormatTag            = WAVE_FORMAT_EXTENSIBLE;
    wfx.Format.nChannels             = channels;
    wfx.Format.wBitsPerSample        = bits;
    wfx.Format.nBlockAlign           = wfx.Format.nChannels * wfx.Format.wBitsPerSample / 8;
    wfx.Format.nSamplesPerSec        = rate;
    wfx.Format.nAvgBytesPerSec	     = wfx.Format.nSamplesPerSec * wfx.Format.nBlockAlign;
    wfx.Format.cbSize                = 22;
    wfx.Samples.wValidBitsPerSample  = bits;

    FMOD_memcpy(&wfx.SubFormat, (soundi->mFormat == FMOD_SOUND_FORMAT_PCMFLOAT ? &_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT : &_KSDATAFORMAT_SUBTYPE_PCM), sizeof(GUID));

	dscbd.dwSize		= sizeof(DSCBUFFERDESC);
	dscbd.dwFlags		= 0;
	dscbd.dwBufferBytes = mRecordBufferLength * wfx.Format.nBlockAlign;
	dscbd.dwReserved	= 0;
	dscbd.lpwfxFormat	= (WAVEFORMATEX*)&wfx;
	dscbd.dwFXCount		= 0;
	dscbd.lpDSCFXDesc	= 0;

    mDirectSoundCaptureBuffer = 0;

    hr = mDirectSoundCapture->CreateCaptureBuffer(&dscbd, (LPDIRECTSOUNDCAPTUREBUFFER *)&mDirectSoundCaptureBuffer, 0);
    if (hr != DS_OK)
    {
		return FMOD_ERR_FORMAT;
	}

	hr = mDirectSoundCaptureBuffer->Start(loop);
	if (hr != DS_OK)
    {
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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: ec9d2748 auto
// OFFSET: 0x00951690
FMOD_RESULT OutputDSound::recordStop()
{
	if (mDirectSoundCaptureBuffer)
	{
		mDirectSoundCaptureBuffer->Stop();
		mDirectSoundCaptureBuffer->Release();
		mDirectSoundCaptureBuffer = 0;
	}

    if (mDirectSoundCapture)
    {
        mDirectSoundCapture->Release();
        mDirectSoundCapture = 0;
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
// VERIFY: aa4be989 manual
// OFFSET: 0x009516E0
FMOD_RESULT OutputDSound::recordGetPosition(unsigned int *pcm)
{
    FMOD_RESULT         result;
	HRESULT             hr = DS_OK;
	unsigned int        position = 0, lengthbytes;
    FMOD_SOUND_FORMAT   format;
    int                 bits, channels;

	if (!mDirectSoundCaptureBuffer)
    {
        return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

    hr = mDirectSoundCaptureBuffer->GetCurrentPosition(0, (LPDWORD)&position);
	if (hr != DS_OK)
	{
		return FMOD_ERR_OUTPUT_DRIVERCALL;
	}

    result = mRecordSound->getLength(&lengthbytes, FMOD_TIMEUNIT_PCMBYTES);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mRecordSound->getFormat(0, &format, &channels, &bits);
    if (result != FMOD_OK)
    {
        return result;
    }

	if (position >= lengthbytes)
    {
        *pcm = 0;
		return FMOD_ERR_OUTPUT_DRIVERCALL;
    }

    position *= 8;
    position /= bits;
    position /= channels;

	if (pcm)
    {
        *pcm = position;
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
// VERIFY: f778fecb auto
// OFFSET: 0x00951780
FMOD_RESULT OutputDSound::recordLock(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
    mDirectSoundCaptureBuffer->Lock(offset, length, ptr1, (DWORD *)len1, ptr2, (DWORD *)len2, 0);

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
// VERIFY: 92e952b2 auto
// OFFSET: 0x009517C0
FMOD_RESULT OutputDSound::recordUnlock(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
    mDirectSoundCaptureBuffer->Unlock(ptr1, len1, ptr2, len2);

    return FMOD_OK;
}

}

#endif