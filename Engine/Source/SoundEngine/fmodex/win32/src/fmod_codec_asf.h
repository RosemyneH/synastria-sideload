#ifndef _FMOD_CODEC_ASF_H
#define _FMOD_CODEC_ASF_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_ASF

#include "fmod_codeci.h"

#include <objidl.h>
#include "../lib/wmsdk/include/wmsdk.h"

namespace FMOD
{
    class CROStream : public IStream
    {
    public:

        CROStream();

        //
        // IUnknown methods
        //
        HRESULT STDMETHODCALLTYPE QueryInterface( /* [in] */  REFIID riid,
                                                  /* [out] */ void **ppvObject );
        ULONG STDMETHODCALLTYPE AddRef();
        ULONG STDMETHODCALLTYPE Release();

        //
        // Methods of IStream
        //
        HRESULT STDMETHODCALLTYPE Read( void *pv, ULONG cb, ULONG *pcbRead );
        HRESULT STDMETHODCALLTYPE Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition );
        HRESULT STDMETHODCALLTYPE Stat( STATSTG *pstatstg, DWORD grfStatFlag );

        //
        // Non-implemented methods of IStream
        //
// VERIFY: 4917731d auto
        // OFFSET: 0x008F7EF0
        HRESULT STDMETHODCALLTYPE Write( void const *pv, ULONG cb, ULONG *pcbWritten )
        {
            return( E_NOTIMPL );
        }
// VERIFY: 7226f9ef auto
        // OFFSET: 0x008F7F00
        HRESULT STDMETHODCALLTYPE SetSize( ULARGE_INTEGER libNewSize )
        {
            return( E_NOTIMPL );
        }
// VERIFY: f719ab7e auto
        // OFFSET: 0x008F7F10
        HRESULT STDMETHODCALLTYPE CopyTo( IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten )
        {
            return( E_NOTIMPL );
        }
        HRESULT STDMETHODCALLTYPE Commit( DWORD grfCommitFlags )
        {
            return( E_NOTIMPL );
        }
// VERIFY: a6ccca7b auto
        // OFFSET: 0x008F7F30
        HRESULT STDMETHODCALLTYPE Revert()
        {
            return( E_NOTIMPL );
        }
        HRESULT STDMETHODCALLTYPE LockRegion( ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType )
        {
            return( E_NOTIMPL );
        }
        HRESULT STDMETHODCALLTYPE UnlockRegion( ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType )
        {
            return( E_NOTIMPL );
        }
        HRESULT STDMETHODCALLTYPE Clone( IStream **ppstm )
        {
            return( E_NOTIMPL );
        }

        //
        // CROStream method
        //
        HRESULT Open( /* [in] */ LPCTSTR ptszURL );

    protected:

        ~CROStream();

        File   *mFile;
        LONG    m_cRefs;
    };

    typedef HRESULT (STDMETHODCALLTYPE *PFN_WMCREATESYNCREADER)( IUnknown* pUnkCert, DWORD dwRights, IWMSyncReader **ppSyncReader );

    class SyncPoint;

    static const unsigned int FMOD_ASF_MAXIMUM_SEEK_POS_MS = 2000;

    class CodecASF : public Codec
    {
      private:
        
    	IWMSyncReader  *mReader;              // IWMReader pointer
        CROStream      *mStream;
        WORD            mAudioStreamNum;
        WORD            mVideoStreamNum;
        QWORD           mSampleTime;
        bool            mSeekable;
        int             mNumSyncPoints;
        SyncPoint *mSyncPoint;

        static PFN_WMCREATESYNCREADER gWMCreateSyncReader;
        static HMODULE                gDLLHandle;
        
        FMOD_RESULT getStreamNumbers(IWMProfile* pProfile);

        FMOD_RESULT openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        FMOD_RESULT closeInternal();
        FMOD_RESULT readInternal(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype);
        FMOD_RESULT soundCreateInternal(int subsound, FMOD_SOUND *sound);

      public:

        static FMOD_RESULT F_CALLBACK openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        static FMOD_RESULT F_CALLBACK closeCallback(FMOD_CODEC_STATE *codec);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
        static FMOD_RESULT F_CALLBACK soundCreateCallback(FMOD_CODEC_STATE *codec, int subsound, FMOD_SOUND *sound);

        static FMOD_CODEC_DESCRIPTION_EX *getDescriptionEx();        
    };
}

#endif  /* FMOD_SUPPORT_ASF */

#endif