#ifndef ENGINE_SOURCE_STORM_SOURCE_IOUNIT_IOFILEUNIT_H
#define ENGINE_SOURCE_STORM_SOURCE_IOUNIT_IOFILEUNIT_H

#include <cstddef>
#include <cstdint>

namespace Storm {

class IORefcount {
    public:
    /* 0004 */ IORefcount* m_refcount;
    /* 0008 */ int32_t m_count;

    virtual ~IORefcount();
    void Release();
};

struct IOFileParam {
    /* 0000 */ const char* m_name;
    /* 0004 */ uint32_t m_access;
    /* 0008 */ uint32_t m_share;
    /* 000C */ uint32_t m_creation;
    /* 0010 */ uint32_t m_flags;
};

class IOUnit {
    public:
    /* 0004 */ IORefcount* m_refcount;
    /* 0008 */ int32_t m_count;

    IOUnit();
    virtual ~IOUnit();
    virtual bool Read(void* buffer, int64_t offset, uint32_t count, uint32_t* bytesRead) = 0;
    virtual bool Write(const void* buffer, int64_t offset, uint32_t count) = 0;
    virtual bool Flush() = 0;
    virtual bool GetFileSize(int64_t* size) = 0;
    virtual bool SetFileSize(int64_t size) = 0;
    virtual void* Win32Handle() = 0;

    void Release();
};

class IOWin32FileUnit : public IOUnit {
    public:
    static IOWin32FileUnit* Open(const IOFileParam* param);

    /* 000C */ void* m_handle;
    /* 0010 */ int64_t m_position;

    IOWin32FileUnit(void* handle);
    virtual ~IOWin32FileUnit();
    virtual bool Read(void* buffer, int64_t offset, uint32_t count, uint32_t* bytesRead);
    virtual bool Write(const void* buffer, int64_t offset, uint32_t count);
    virtual bool Flush();
    virtual bool GetFileSize(int64_t* size);
    virtual bool SetFileSize(int64_t size);
    virtual void* Win32Handle();

    bool Seek(int64_t offset);
};

class IOUnbufferedFileUnit : public IOUnit {
    public:
    static IOUnbufferedFileUnit* Open(const IOFileParam* param);

    /* 000C */ IOUnit* m_unit;
    /* 0010 */ int64_t m_size;
    /* 0018 */ void* m_buffer;
    /* 001C */ uint32_t m_alignMask;
    /* 0020 */ char m_path[260];

    IOUnbufferedFileUnit(void* handle, const IOFileParam* param);
    virtual ~IOUnbufferedFileUnit();
    virtual bool Read(void* buffer, int64_t offset, uint32_t count, uint32_t* bytesRead);
    virtual bool Write(const void* buffer, int64_t offset, uint32_t count);
    virtual bool Flush();
    virtual bool GetFileSize(int64_t* size);
    virtual bool SetFileSize(int64_t size);
    virtual void* Win32Handle();
};

class IOFileUnit {
    public:
    static IOUnit* Create(const IOFileParam& param, IOUnit* next);
};

}

#endif
