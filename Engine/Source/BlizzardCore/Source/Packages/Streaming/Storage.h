#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_STREAMING_STORAGE_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_STREAMING_STORAGE_H

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <string>

namespace Blizzard {
namespace File {
typedef int32_t INODE;
struct Filesystem;
struct StreamRecord;
}
}

namespace Blizzard {
namespace Streaming {
struct LoadRange;
struct StorageEntry;
struct WriteRequest;
}
}

namespace Blizzard {
namespace Storage {

class StorageEntryRef {
    public:
    /* 0000 */ Blizzard::Streaming::StorageEntry* m_entry;

    StorageEntryRef(Blizzard::Streaming::StorageEntry* entry);
};

class PartialFile {
    public:
    static Blizzard::Streaming::StorageEntry* CreateShared(const std::string& key, const char* path, int64_t size, const char* version, int32_t flags);
    static bool ParseFile(Blizzard::File::Filesystem* fs, Blizzard::File::StreamRecord* file);
    static bool RebuildHeader(Blizzard::File::Filesystem* fs, Blizzard::File::StreamRecord* file);
};

extern int32_t s_variantIndex;
extern char s_variantName[260];
extern std::map<std::string, Blizzard::Streaming::StorageEntry>* s_storage;
extern Blizzard::File::StreamRecord* s_storageFile;
extern bool s_storageReady;
extern bool s_storageDirty;
extern volatile bool s_storageStop;
extern volatile bool s_storageStopped;
extern bool s_usePartFiles;
extern int32_t s_blockSize;
extern std::string s_storagePath;
extern std::deque<Blizzard::Streaming::WriteRequest> s_writeQueue;

Blizzard::Streaming::StorageEntry* Acquire(Blizzard::File::Filesystem* fs, const std::string& key, const char* path, int64_t size, const char* version, int32_t flags);
void BuildVariantFileName(char* dest, uint32_t size, const char* name, const char* extension);
int32_t BuildRanges(Blizzard::Streaming::StorageEntry* entry, int64_t offset, int32_t size, Blizzard::Streaming::LoadRange* ranges);
int32_t BuildRangesFromStore(Blizzard::Streaming::StorageEntry* entry, int64_t offset, int32_t size, Blizzard::Streaming::LoadRange* ranges);
int32_t BuildRangesWithHeader(Blizzard::Streaming::StorageEntry* entry, int64_t offset, int32_t size, Blizzard::Streaming::LoadRange* ranges);
bool Close(Blizzard::File::Filesystem* fs, Blizzard::File::StreamRecord* file, Blizzard::Streaming::StorageEntry* entry);
int32_t CountRanges(Blizzard::Streaming::StorageEntry* entry, int64_t offset, int64_t size);
bool FlushFile(Blizzard::File::Filesystem* fs, Blizzard::File::StreamRecord* file);
int32_t GetBlockLength(Blizzard::Streaming::StorageEntry* entry, int32_t block);
int64_t GetNextHeaderOffset();
Blizzard::Streaming::LoadRange* GetRange(Blizzard::Streaming::StorageEntry* entry, int64_t offset, int32_t size, int32_t* count, bool lock);
bool Initialize();
bool Initialize(const char* path, bool required);
bool IsComplete(Blizzard::Streaming::StorageEntry* entry);
bool QueueWrite(Blizzard::File::Filesystem* fs, Blizzard::File::StreamRecord* file, uint8_t* data, int64_t offset, int32_t size, bool owned);
bool ReadAt(Blizzard::File::Filesystem* fs, Blizzard::File::StreamRecord* file, void* buffer, int64_t offset, int32_t size);
bool ReadLocal(Blizzard::File::Filesystem* fs, Blizzard::File::StreamRecord* file, Blizzard::Streaming::StorageEntry* entry, uint8_t* buffer, int64_t offset, int32_t size);
bool ReadStorageFile(const uint8_t* data, int32_t size);
bool Remove(const std::string& path);
bool RequestRange(Blizzard::Streaming::StorageEntry* entry, int64_t offset, int32_t size, int32_t state);
void Shutdown();
void SetRangeAs(Blizzard::Streaming::StorageEntry* entry, int64_t offset, int32_t size, int32_t state);
uint32_t StorageThread(void* param);
bool WriteAt(Blizzard::File::StreamRecord* file, const void* buffer, int64_t offset, int32_t size);
bool WriteBlock(Blizzard::Streaming::WriteRequest* request);
bool WriteIndex();
bool WriteIndexEntry(Blizzard::Streaming::StorageEntry* entry, int32_t block, bool header);
bool WritePartBlockInfo(Blizzard::Streaming::StorageEntry* entry, Blizzard::File::StreamRecord* file, int32_t block);

}
}

#endif
