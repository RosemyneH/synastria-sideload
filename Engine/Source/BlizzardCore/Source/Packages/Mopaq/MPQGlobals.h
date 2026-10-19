#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_MOPAQ_MPQGLOBALS_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_MOPAQ_MPQGLOBALS_H

#include <cstdint>
#include <functional>
#include <set>
#include <vector>
#include "Engine/Source/BlizzardCore/Source/Packages/Mopaq/SFileArchives.h"
#include "Engine/Source/BlizzardCore/Source/System/Lock/Lock.h"
#include "Engine/Source/BlizzardCore/Source/System/Memory/AllocSTL.h"
#include "Engine/Source/BlizzardCore/Source/System/Thread/Thread.h"
#include "Engine/Source/BlizzardCore/Source/System/Using/Using.h"

namespace System_MPQGlobals {

struct ArchiveLess {
    bool operator()(const System_SFile::ARCHIVEREC* a, const System_SFile::ARCHIVEREC* b) const;
};

typedef std::set<System_SFile::ARCHIVEREC*, ArchiveLess, Blizzard::Memory::AllocSTL_raw<System_SFile::ARCHIVEREC*>> ArchiveList;

struct MopaqGlobals {
    /* 0000 */ uint32_t m_directAccess;
    /* 0004 */ char m_basePath[260];
    /* 0108 */ void (*m_openCallback)(const char* name, void* param);
    /* 010C */ void* m_openCallbackParam;
    /* 0110 */ uint32_t m_md5Mode;
    /* 0114 */ bool (*m_md5Callback)(void* context, const uint8_t* md5);
};

class Worker {
    public:
    static uint32_t ThreadProc(void* param);

    /* 0000 */ Blizzard::Thread::ThreadRecord* m_thread;
    /* 0004 */ Blizzard::Lock::EventObject m_start;
    /* 0008 */ Blizzard::Lock::EventObject m_done;
    /* 000C */ void (*m_proc)(void* param);
    /* 0010 */ void* m_param;

    Worker();
    ~Worker();
};

class WorkerPool {
    public:
    /* 0000 */ Blizzard::Lock::RMutex m_mutex;
    /* 0018 */ std::vector<Worker*> m_workers;

    ~WorkerPool();
    void Acquire(uint32_t count, Worker** workers);
    void Release(uint32_t count, Worker** workers);
};

class Globals {
    public:
    /* 0000 */ ArchiveList m_archives;
    /* 000C */ Blizzard::Lock::RMutex m_archiveLock;
    /* 0024 */ MopaqGlobals m_mopaq;
    /* 013C */ Blizzard::Lock::RMutex m_mopaqLock;
    /* 0154 */ WorkerPool m_workerPool;

    Globals();
    ~Globals();
};

class RegisterGlobalStorage : public Blizzard::Using::Use {
    public:
    static void Shutdown();

    RegisterGlobalStorage();
};

extern Blizzard::Lock::ConstructOnce<Globals> s_globals;

System_SFile::ARCHIVEREC* FindArchiveByName(const char* name);
ArchiveList* GetArchiveList();
Blizzard::Lock::RMutex* GetArchiveLock();
MopaqGlobals* GetMopaqGlobals();
Blizzard::Lock::RMutex* GetMopaqGlobalsLock();
WorkerPool* GetWorkerPool();

}

#endif
