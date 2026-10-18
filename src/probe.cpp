#include "cache.h"

#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    const char* root = argc > 1 ? argv[1] : "/home/emma/Games/Synastria";
    SynCache cache{};
    bool ok = syn_load_cache(root, &cache);

    printf("root %s\n", root);
    printf("WoWExt.bin        %s  %zu bytes  magic %s\n", cache.bin_ok ? "ok" : "BAD", cache.bin_bytes, cache.bin_ok ? "2838c53a" : "mismatch");
    printf("WoWExt2.bin       %s  %zu bytes  version %u\n", cache.bin2_ok ? "ok" : "BAD", cache.bin2_bytes, cache.bin2_version);
    printf("WoWExt3.bin       %s  %zu bytes  lua\n", cache.lua_ok ? "ok" : "BAD", cache.lua_bytes);
    printf("WoWExtItemLoc.bin %s  %zu bytes  version %u\n", cache.item_ok ? "ok" : "BAD", cache.item_bytes, cache.item_version);
    printf("WoWExtObjLoc.bin  %s  %zu bytes  version %u  count %u\n", cache.obj_ok ? "ok" : "BAD", cache.obj_bytes, cache.obj_version, cache.obj_count);
    printf("WoWExt.dll        %s  %zu bytes\n", cache.dll_present ? "pe32" : "missing", cache.dll_bytes);
    printf("opcode            0x4b8  CMSG_UNUSED5\n");
    printf("dll mapping       skipped (32-bit Wow.exe addresses)\n");

    syn_free_cache(&cache);
    return ok ? 0 : 1;
}
