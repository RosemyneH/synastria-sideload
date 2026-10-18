#pragma once

#include <cstddef>
#include <cstdint>

struct SynCache {
    bool bin_ok;
    bool bin2_ok;
    bool item_ok;
    bool obj_ok;
    bool lua_ok;
    bool dll_present;
    uint32_t bin2_version;
    uint32_t item_version;
    uint32_t obj_version;
    uint32_t obj_count;
    size_t bin_bytes;
    size_t bin2_bytes;
    size_t item_bytes;
    size_t obj_bytes;
    size_t lua_bytes;
    size_t dll_bytes;
    char* lua;
};

// ʕ •ᴥ•ʔ✿ Read the Synastria cache files that WoWExt.dll would have loaded beside Wow.exe. ✿ ʕ •ᴥ•ʔ
bool syn_load_cache(const char* root, SynCache* cache);
void syn_free_cache(SynCache* cache);
