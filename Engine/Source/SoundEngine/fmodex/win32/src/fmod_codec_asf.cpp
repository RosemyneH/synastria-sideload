#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_ASF

#include "fmod.h"
#include "fmod_codec_asf.h"
#include "fmod_debug.h"
#include "fmod_file.h"
#include "fmod_soundi.h"
#include "fmod_string.h"
#include "fmod_syncpoint.h"

#include <windows.h>
#include <memory.h>
#include <stdio.h>

namespace FMOD
{


FMOD_CODEC_DESCRIPTION_EX asfcodec;

static const FMOD_GUID ASF_Header_Object = { 0x75B22630,0x668E,0x11CF,{0xA6,0xD9,0x00,0xAA,0x00,0x62,0xCE,0x6C}};

PFN_WMCREATESYNCREADER CodecASF::gWMCreateSyncReader = 0;
HMODULE                CodecASF::gDLLHandle = 0;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetCodecDescriptionEx is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_CODEC_DESCRIPTION_EX * F_API FMODGetCodecDescriptionEx()
    {
        return CodecASF::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: ce70856d manual
// OFFSET: 0x008F8B60
FMOD_CODEC_DESCRIPTION_EX *CodecASF::getDescriptionEx()
{
    FMOD_memset(&asfcodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    asfcodec.name        = "FMOD ASF Codec";
    asfcodec.version     = 0x00010100;
    asfcodec.timeunits   = FMOD_TIMEUNIT_MS;
    asfcodec.open        = &CodecASF::openCallback;
    asfcodec.close       = &CodecASF::closeCallback;
    asfcodec.read        = &CodecASF::readCallback;
    asfcodec.setposition = &CodecASF::setPositionCallback;
    asfcodec.soundcreate = &CodecASF::soundCreateCallback;

    asfcodec.mType       = FMOD_SOUND_TYPE_ASF;
    asfcodec.mSize       = sizeof(CodecASF);

    return &asfcodec;
}



/*
[
	[DESCRIPTION]
    Retrieve video and audio stream numbers from profile

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: c9259e30 manual
// OFFSET: 0x008F7C40
FMOD_RESULT CodecASF::getStreamNumbers(IWMProfile* pProfile)
{
    HRESULT          hr = S_OK;
    IWMStreamConfig *pStream = NULL;
    DWORD            dwStreams = 0;
    GUID             pguidStreamType;

    if ( NULL == pProfile )
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    hr = pProfile->GetStreamCount( &dwStreams );
    if ( FAILED( hr ) )
    {
        return FMOD_ERR_FILE_BAD;
    }

    mAudioStreamNum = 0;
    mVideoStreamNum = 0;

    for ( DWORD i = 0; i < dwStreams; i++ )
    {
        hr = pProfile->GetStream( i, &pStream );
        if ( FAILED( hr ) )
        {
            return FMOD_ERR_FILE_BAD;
        }

        WORD wStreamNumber = 0 ;

        //
        //  Get the stream number of the current stream
        //

        hr = pStream->GetStreamNumber( &wStreamNumber );
        if ( FAILED( hr ) )
        {
            return FMOD_ERR_FILE_BAD;
        }

        hr = pStream->GetStreamType( &pguidStreamType );
        if ( FAILED( hr ) )
        {
            return FMOD_ERR_FILE_BAD;
        }
        
        if( WMMEDIATYPE_Audio == pguidStreamType )
        {
            mAudioStreamNum = wStreamNumber;
        }
        else if( WMMEDIATYPE_Video == pguidStreamType )
        {
            mVideoStreamNum = wStreamNumber;
        }

        if (pStream)
        {
            pStream->Release();
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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: b472eeb0 manual
// OFFSET: 0x008F8260
FMOD_RESULT CodecASF::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    FMOD_RESULT             result = FMOD_OK;
    HRESULT                 hr;
    WMT_STREAM_SELECTION	wmtSS = WMT_ON;
    IWMProfile             *pProfile = NULL;
    int                     count, numoutputs;
    char                    header[16];

    init(FMOD_SOUND_TYPE_ASF);

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "CodecASF::openInternal", "attempting to open as ASF..\n"));

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

	result = mFile->read(&header, 1, 16, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (memcmp(header, &ASF_Header_Object, 16))       /* ASF */
    {
        return FMOD_ERR_FORMAT;
    }

    mSeekable      = true;
    mSyncPoint     = 0;
    mNumSyncPoints = 0;

    if (!gWMCreateSyncReader)
    {
	    HMODULE	asfmodule = NULL;

	    if (!gDLLHandle)
        {
		    gDLLHandle = LoadLibrary("wmvcore.dll");
            if (!gDLLHandle)
            {
                return FMOD_ERR_PLUGIN_RESOURCE;
            }
        }
	    
	    gWMCreateSyncReader = (PFN_WMCREATESYNCREADER)GetProcAddress(gDLLHandle,"WMCreateSyncReader");
        if (!gWMCreateSyncReader)
        {
            return FMOD_ERR_PLUGIN_RESOURCE;
        }

    }

    mWaveFormatMemory = (FMOD_CODEC_WAVEFORMAT *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_WAVEFORMAT));
    if (!mWaveFormatMemory)
    {
        return FMOD_ERR_MEMORY;
    }

    waveformat = mWaveFormatMemory;

	/*
        Get size of file in bytes
    */
	result = mFile->getSize(&mWaveFormatMemory->lengthbytes);
    if (result != FMOD_OK)
    {
        return result;
    }

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    hr = CoInitialize( NULL );
    if (FAILED( hr ))
    {
        return FMOD_ERR_COM;
    }


    hr = gWMCreateSyncReader(  NULL, 0, &mReader );
	if( FAILED( hr ) )
	{
        if (hr == E_OUTOFMEMORY)
        {
            return FMOD_ERR_MEMORY;
        }

        return FMOD_ERR_FORMAT;
	}

	mStream = FMOD_Object_Calloc(CROStream);
	if( NULL == mStream )
	{
        return FMOD_ERR_MEMORY;
	}

    hr = mStream->Open( (const char *)mFile );
    if ( FAILED( hr ) )
    {
        return FMOD_ERR_FORMAT;
    }

    hr = mReader->OpenStream( mStream );
    if (FAILED( hr ) )
    {
        return FMOD_ERR_FORMAT;
    }
    
    /*
        Get the profile interface
    */
    hr = mReader->QueryInterface( IID_IWMProfile, ( VOID ** )&pProfile );
    if ( FAILED( hr ) ) 
    {
		return FMOD_ERR_COM;
    }

    /*
        Find out stream numbers for video and audio using the profile
    */
    result = getStreamNumbers( pProfile );
    if (result != FMOD_OK)
    {
        return result;
    }

    pProfile->Release();

    hr = mReader->SetRange( 0, 0 );
    if ( FAILED( hr ) )
    {
        return FMOD_ERR_FORMAT;
    }

    hr = mReader->SetStreamsSelected( 1, &mAudioStreamNum, &wmtSS );
    if ( FAILED( hr ) )
    {
        return FMOD_ERR_FORMAT;
    }

    hr = mReader->SetReadStreamSamples( mAudioStreamNum, 0 );
    if ( FAILED( hr ) )
    {
        return FMOD_ERR_FORMAT;
    }

    /*
        Get the format information
    */
    hr = mReader->GetOutputCount( (DWORD *)&numoutputs );

    mPCMBufferLengthBytes = 0;
    for (count=0; count < numoutputs; count++)
    {
        IWMOutputMediaProps *props;
        WM_MEDIA_TYPE       *mediatype;
        DWORD                mediatypesize;
        WAVEFORMATEX        *waveformat;
        int                  bits;

        hr = mReader->GetOutputFormat(count, 0, &props);
        if ( FAILED( hr ) )
        {
            return FMOD_ERR_FORMAT;
        }

        hr = props->GetMediaType(NULL, &mediatypesize);
        if ( FAILED( hr ) )
        {
            props->Release();
            return FMOD_ERR_FORMAT;
        }

        mediatype = (WM_MEDIA_TYPE *)FMOD_Memory_Calloc(mediatypesize);
        if (!mediatype)
        {
            props->Release();
            return FMOD_ERR_MEMORY;
        }

        hr = props->GetMediaType(mediatype, &mediatypesize);
        if ( FAILED( hr ) )
        {
            props->Release();
            return FMOD_ERR_FORMAT;
        }

        if (mediatype->lSampleSize > mPCMBufferLengthBytes)
        {
            mPCMBufferLengthBytes = mediatype->lSampleSize;
        }
        if (!memcmp(&mediatype->formattype, &WMFORMAT_WaveFormatEx, sizeof(GUID)))
        {
            waveformat = (WAVEFORMATEX *)mediatype->pbFormat;

            bits = waveformat->wBitsPerSample;
            result = SoundI::getFormatFromBits(bits, &mWaveFormatMemory->format);
            if (result != FMOD_OK)
            {
                props->Release();
                FMOD_Memory_Free(mediatype);
                return FMOD_ERR_FORMAT;
            }
            mWaveFormatMemory->channels   = waveformat->nChannels;
            mWaveFormatMemory->blockalign = waveformat->nBlockAlign;
            mWaveFormatMemory->frequency  = waveformat->nSamplesPerSec;
        }

        props->Release();

        FMOD_Memory_Free(mediatype);
    }

    /*
        Get the ASF header information
    */
    {
        IWMHeaderInfo     *headerinfo;
        WMT_ATTR_DATATYPE  wmtType;
        BYTE              *pbValue;
        WORD               cbLength;
        QWORD              duration;
        WORD               streamnum;
        WORD               markercount;

        hr = mReader->QueryInterface( IID_IWMHeaderInfo, ( VOID ** )&headerinfo );
        if ( FAILED( hr ) )
        {
            return FMOD_ERR_FORMAT;
        }

        streamnum = 0;

	    hr = headerinfo->GetAttributeByName( &streamnum, L"Duration", &wmtType, NULL, &cbLength );
	    if( FAILED( hr ) && ( ASF_E_NOTFOUND != hr ) )
	    {
            return FMOD_ERR_FORMAT;
	    }
	    if( ASF_E_NOTFOUND == hr )
	    {
		    return FMOD_ERR_FORMAT;
	    }

	    pbValue = (BYTE *)FMOD_Memory_Calloc(cbLength);
	    if (!pbValue)
	    {
		    return FMOD_ERR_MEMORY;
	    }

	    hr = headerinfo->GetAttributeByName( &streamnum, L"Duration", &wmtType, pbValue, &cbLength );
        if ( FAILED( hr ) )
        {
            FMOD_Memory_Free(pbValue);
            return FMOD_ERR_FORMAT;
        }

	    duration = *((QWORD *)pbValue);       
        mWaveFormatMemory->lengthpcm = (unsigned int)(duration * mWaveFormatMemory->frequency / 10000 / 1000);

        FMOD_Memory_Free(pbValue);

        /*
            Get seekable flag
        */
	    hr = headerinfo->GetAttributeByName( &streamnum, L"Seekable", &wmtType, NULL, &cbLength );
	    if( FAILED( hr ) && ( ASF_E_NOTFOUND != hr ) )
	    {
            return FMOD_ERR_FORMAT;
	    }
	    if( ASF_E_NOTFOUND == hr )
	    {
		    return FMOD_ERR_FORMAT;
	    }

	    pbValue = (BYTE *)FMOD_Memory_Calloc(cbLength);
	    if (!pbValue)
	    {
		    return FMOD_ERR_MEMORY;
	    }

	    hr = headerinfo->GetAttributeByName( &streamnum, L"Seekable", &wmtType, pbValue, &cbLength );
        if ( FAILED( hr ) )
        {
            FMOD_Memory_Free(pbValue);
            return FMOD_ERR_FORMAT;
        }
        
	    mSeekable = *((BOOL *)pbValue) ? true : false;

        FMOD_Memory_Free(pbValue);

        /*
            Check for syncpoints.
        */
        hr = headerinfo->GetMarkerCount(&markercount);
        if ( FAILED( hr ) )
        {
            return FMOD_ERR_FORMAT;
        }

        if (markercount)
        {
            WORD count;

            mNumSyncPoints = markercount;

            mSyncPoint = (SyncPoint *)FMOD_Memory_Alloc(mNumSyncPoints * sizeof(SyncPoint));
            if (!mSyncPoint)
            {
                return FMOD_ERR_MEMORY;
            }

            for (count=0; count < mNumSyncPoints; count++)
            { 
                SyncPoint *point = &mSyncPoint[count];
                WCHAR name[256];
                WORD  len = 256;
                QWORD offset;


                headerinfo->GetMarker(count, name, &len, &offset); 

                wcstombs(point->mName, name, len);

                point->mOffset = (unsigned int)(offset * mWaveFormatMemory->frequency / 10000000);
            }
        }
    }

    if (!mPCMBufferLengthBytes)
    {
        mPCMBufferLengthBytes = 64 * 1024;
    }
    if (mPCMBufferLengthBytes)
    {
        mPCMBuffer = (unsigned char *)FMOD_Memory_Calloc(mPCMBufferLengthBytes);
        if (!mPCMBuffer)
        {
            return FMOD_ERR_MEMORY;
        }
    }

    mSampleTime     = 0;
	mSrcDataOffset  = 0;

    /*
        Fill out base class members, also pointing to or allocating storage for them.
    */
    numsubsounds = 0;

    return result;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 03a9396f manual
// OFFSET: 0x008F7D50
FMOD_RESULT CodecASF::closeInternal()
{   
    if (mReader)
    {        
        mReader->Close();
        mReader->Release();
        mReader = 0;
    }

    if (mStream)
    {
        mStream->Release();
        mStream = 0;
    }

    if (mPCMBuffer)
    {
        FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "CodecWAV::release", "Free PCM Buffer\n"));

        FMOD_Memory_Free(mPCMBuffer);
    }
    mPCMBufferLengthBytes = 0;


    
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 118f5ba9 auto
// OFFSET: 0x008F7DD0
FMOD_RESULT CodecASF::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    HRESULT       hr           = S_OK;
    INSSBuffer   *pSample      = NULL;
    QWORD         cnsDuration  = 0;
    DWORD         dwFlags      = 0;
    DWORD         dwOutputNum  = 0;
    WORD          wStreamNum   = 0;
    BYTE         *pdwBuffer    = NULL;
    DWORD         dwLength     = 0;

    *bytesread = 0;

    hr = mReader->GetNextSample(mAudioStreamNum, &pSample, &mSampleTime, &cnsDuration, &dwFlags, &dwOutputNum, &wStreamNum);
    if (FAILED(hr))
    {
        if (hr == NS_E_NO_MORE_SAMPLES)
        {
            return FMOD_ERR_FILE_EOF;
        }
        return FMOD_ERR_FILE_BAD;
    }

    pSample->GetBufferAndLength(&pdwBuffer, &dwLength);

    FMOD_memcpy(buffer, pdwBuffer, dwLength);

    *bytesread = dwLength;

    pSample->Release();

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: d2fecdfa manual
// OFFSET: 0x008F7E80
FMOD_RESULT CodecASF::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    HRESULT hr;
    QWORD offset;
    int retries;

    if (!mSeekable)
    {
        return FMOD_ERR_FILE_COULDNOTSEEK;
    }

    /*
        I'm sure why but setting it twice seemed to fix a problem with it returning an error "ASF_E_BADDATAUNIT" in some cases.
    */
    retries = 0;
    do
    {
        offset = (QWORD)position * 10000;

        hr = mReader->SetRange( offset, 0 );
        if ( FAILED( hr ) )
        {
            FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "CodecASF::setPositionInternal", "ERROR %08x subsound %d position %d postype %d.\n", hr, subsound, position, postype));
            retries ++;
            position+=100;
        }
    } while (FAILED( hr ) && retries < 100);

    if (FAILED( hr ))
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

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
CROStream::CROStream() :
    m_cRefs( 1 ),
    mFile( 0 )
{
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
CROStream::~CROStream()
{
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
HRESULT CROStream::Open( LPCTSTR ptszURL )
{
    mFile = (File *)ptszURL;

    return S_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 4d833d13 auto
// OFFSET: 0x008F7F40
HRESULT CROStream::Read( void *pv, ULONG cb, ULONG *pcbRead )
{
    FMOD_RESULT result;
    
    result = mFile->read(pv, 1, cb, (unsigned int *)pcbRead);
    if (result != FMOD_OK)
    {
        return !S_OK;
    }

    return S_OK;
}



/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 328299c9 auto
// OFFSET: 0x008F7F70
HRESULT CROStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    FMOD_RESULT result;
    int dwMoveMethod;

    switch( dwOrigin )
    {
        case STREAM_SEEK_SET:
            dwMoveMethod = SEEK_SET;
            break;

        case STREAM_SEEK_CUR:
            dwMoveMethod = SEEK_CUR;
            break;

        case STREAM_SEEK_END:
            dwMoveMethod = SEEK_END;
            break;

        default:
            return( E_INVALIDARG );
    };

    result = mFile->seek(dlibMove.LowPart, dwMoveMethod);
    if (result != FMOD_OK)
    {
        return !S_OK;
    }

    if( NULL != plibNewPosition )
    {
        DWORD dwPos;

        mFile->tell((unsigned int *)&dwPos);

        plibNewPosition->LowPart = dwPos;
        plibNewPosition->HighPart = 0;
    }

    return S_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 7848f93b auto
// OFFSET: 0x008F7FF0
HRESULT CROStream::Stat( STATSTG *pstatstg, DWORD grfStatFlag )
{
    FMOD_RESULT result;

    if( ( NULL == pstatstg ) || ( STATFLAG_NONAME != grfStatFlag ) )
    {
        return( E_INVALIDARG );
    }

    DWORD dwFileSize;

    result = mFile->getSize((unsigned int *)&dwFileSize);
    if (result != FMOD_OK)
    {
        return !S_OK;
    }

    FMOD_memset( pstatstg, 0, sizeof( STATSTG ) );

    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.LowPart = dwFileSize;

    return( S_OK );
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 1a68ff47 manual
// OFFSET: 0x008F8050
HRESULT CROStream::QueryInterface( REFIID riid, void **ppv )
{
    if( ( IID_IUnknown == riid ) || ( IID_IStream == riid ) )
    {
        *ppv = this;
        AddRef();

        return( S_OK );
    }
        
    return( E_NOINTERFACE );
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 92d206aa manual
// OFFSET: 0x008F80A0
ULONG CROStream::AddRef()
{
    return( InterlockedIncrement( &m_cRefs ) );
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 323ab5c1 manual
// OFFSET: 0x008F80C0
ULONG CROStream::Release()
{
    if( 0 == InterlockedDecrement( &m_cRefs ) )
    {
        FMOD_Memory_Free(this);
        return( 0 );
    }
    
    return( 0xbad );
}



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
// VERIFY: 61a55a61 auto
// OFFSET: 0x008F8100
FMOD_RESULT CodecASF::soundCreateInternal(int subsound, FMOD_SOUND *sound)
{
    SoundI *s = (SoundI *)sound;

    if (mNumSyncPoints && mSyncPoint)
    {
        int count;

        for (count = 0; count < mNumSyncPoints; count++)
        {
            s->addSyncPoint(mSyncPoint[count].mOffset, FMOD_TIMEUNIT_PCM, mSyncPoint[count].mName, 0);
        }

        FMOD_Memory_Free(mSyncPoint);
        mSyncPoint = 0;
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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 220cf014 manual
// OFFSET: 0x008F8B20
FMOD_RESULT F_CALLBACK CodecASF::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecASF *asf = (CodecASF *)codec;

    return asf->openInternal(usermode, userexinfo);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 14b7c71e manual
// OFFSET: 0x008F8190
FMOD_RESULT F_CALLBACK CodecASF::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecASF *asf = (CodecASF *)codec;

    return asf->closeInternal();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 5d18b7f2 auto
// OFFSET: 0x008F81C0
FMOD_RESULT F_CALLBACK CodecASF::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecASF *asf = (CodecASF *)codec;

    return asf->readInternal(buffer, sizebytes, bytesread);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 872f0dbf auto
// OFFSET: 0x008F81F0
FMOD_RESULT F_CALLBACK CodecASF::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecASF *asf = (CodecASF *)codec;

    return asf->setPositionInternal(subsound, position, postype);
}


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
// VERIFY: b9be3f16 manual
// OFFSET: 0x008F8220
FMOD_RESULT F_CALLBACK CodecASF::soundCreateCallback(FMOD_CODEC_STATE *codec, int subsound, FMOD_SOUND *sound)
{
    CodecASF *asf = (CodecASF *)codec;    

    return asf->soundCreateInternal(subsound, sound);
}

}

#endif