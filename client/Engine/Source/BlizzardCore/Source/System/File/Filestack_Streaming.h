#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_FILE_FILESTACK_STREAMING_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_FILE_FILESTACK_STREAMING_H

#include <cstdint>
#include <map>
#include <string>
#include "Engine/Source/BlizzardCore/Source/System/File/Filestack.h"
#include "Engine/Source/BlizzardCore/Source/System/File/Types.h"
#include "Engine/Source/Storm/H/Types.h"

namespace Blizzard {
namespace Streaming {
namespace Manifests {
class FileItem;
}
}
}

class FileStack_Streaming {
    public:
    /* 0000 */ std::string m_key;
    /* 001C */ Blizzard::Streaming::Manifests::FileItem* m_file;
    /* 0020 */ Blizzard::Streaming::StorageEntry* m_entry;

    Blizzard::File::StreamRecord* InitRecord(Blizzard::File::StreamRecord* record);
    Blizzard::File::StreamRecord* StreamingOpen(Blizzard::File::Filesystem* fs, System_File::Stacked::FileParms* parms);
};

class StreamDirWalk {
    public:
    static bool Callback(const Blizzard::File::ProcessDirParms& parms);

    /* 0000 */ std::map<std::string, bool> m_files;

    bool Walk(Blizzard::File::Filesystem* fs, System_File::Stacked::FileParms* parms);
};

namespace System_File {
namespace Stacked {

extern Blizzard::File::Filesystem s_streamstack;

Blizzard::File::StreamRecord* STORMAPI AllocStreamingRecord(const char* path);
bool STORMAPI CloseAt(Blizzard::File::Filesystem* fs, Blizzard::File::StreamRecord* file);
bool STORMAPI CreatePartFile(Blizzard::File::Filesystem* fs, FileParms* parms);
int32_t STORMAPI ExistsAt(Blizzard::File::Filesystem* fs, const char* path);
Blizzard::File::StreamRecord* STORMAPI OpenAt(Blizzard::File::Filesystem* fs, Blizzard::File::StreamRecord* file, const char* path, int32_t mode);
void PushStreaming();
bool stream_close(Blizzard::File::Filesystem* fs, FileParms* parms);
bool stream_dirwalk(Blizzard::File::Filesystem* fs, FileParms* parms);
bool stream_exists(Blizzard::File::Filesystem* fs, FileParms* parms);
bool stream_flush(Blizzard::File::Filesystem* fs, FileParms* parms);
bool stream_getfileinfo(Blizzard::File::Filesystem* fs, FileParms* parms);
bool stream_open(Blizzard::File::Filesystem* fs, FileParms* parms);
bool stream_readp(Blizzard::File::Filesystem* fs, FileParms* parms);
bool stream_shutdown(Blizzard::File::Filesystem* fs, FileParms* parms);

}
}

#endif
