#ifndef WOW_SOURCE_WOWSERVICES_PATCHFILES_H
#define WOW_SOURCE_WOWSERVICES_PATCHFILES_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Os/OsFile.h"
#include "Engine/Source/Storm/H/stpl.h"

struct PATCHARCHIVESPEC {
    /* 0000 */ int32_t m_pass;
    /* 0004 */ int32_t m_flag;
    /* 0008 */ bool m_useInstallPath;
    /* 000C */ const char* m_directory;
    /* 0010 */ const char* m_pattern;
};

class PatchFiles {
    public:
    /* 0000 */ const char* m_directory;
    /* 0004 */ TSGrowableArray<char*> m_files;

    static int32_t EnumPatchArchives(OS_FILE_DATA& data, void* param);

    PatchFiles(const char* locale, bool flag);
    ~PatchFiles();
};

#endif
