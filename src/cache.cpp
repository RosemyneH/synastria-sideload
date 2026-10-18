#include "cache.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

constexpr uint32_t kSpellMagic = 0x2838C53Au;
constexpr uint32_t kItemVersion = 10;

bool read_file(const char* path, uint8_t** out, size_t* size) {
    *out = nullptr;
    *size = 0;
    FILE* file = fopen(path, "rb");
    if (!file) {
        return false;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }
    long len = ftell(file);
    if (len < 0) {
        fclose(file);
        return false;
    }
    rewind(file);
    auto* buf = static_cast<uint8_t*>(malloc(static_cast<size_t>(len) + 1));
    if (!buf) {
        fclose(file);
        return false;
    }
    size_t got = fread(buf, 1, static_cast<size_t>(len), file);
    fclose(file);
    buf[got] = 0;
    *out = buf;
    *size = got;
    return got == static_cast<size_t>(len);
}

uint32_t read_u32(const uint8_t* data, size_t size, size_t off) {
    if (off + 4 > size) {
        return 0;
    }
    uint32_t value;
    memcpy(&value, data + off, 4);
    return value;
}

void join(char* dest, size_t dest_size, const char* root, const char* name) {
    snprintf(dest, dest_size, "%s/%s", root, name);
}

}  // namespace

bool syn_load_cache(const char* root, SynCache* cache) {
    memset(cache, 0, sizeof(*cache));
    char path[1024];

    uint8_t* bytes = nullptr;
    size_t size = 0;

    join(path, sizeof(path), root, "WoWExt.bin");
    if (read_file(path, &bytes, &size)) {
        cache->bin_bytes = size;
        cache->bin_ok = size >= 4 && read_u32(bytes, size, 0) == kSpellMagic;
        free(bytes);
    }

    join(path, sizeof(path), root, "WoWExt2.bin");
    if (read_file(path, &bytes, &size)) {
        cache->bin2_bytes = size;
        cache->bin2_version = read_u32(bytes, size, 0);
        cache->bin2_ok = cache->bin2_version == kItemVersion;
        free(bytes);
    }

    join(path, sizeof(path), root, "WoWExt3.bin");
    if (read_file(path, &bytes, &size) && size > 0) {
        cache->lua = reinterpret_cast<char*>(bytes);
        cache->lua_bytes = size;
        cache->lua_ok = true;
    }

    join(path, sizeof(path), root, "WoWExtItemLoc.bin");
    if (read_file(path, &bytes, &size)) {
        cache->item_bytes = size;
        cache->item_version = read_u32(bytes, size, 0);
        cache->item_ok = size >= 8 && cache->item_version != 0;
        free(bytes);
    }

    join(path, sizeof(path), root, "WoWExtObjLoc.bin");
    if (read_file(path, &bytes, &size)) {
        cache->obj_bytes = size;
        cache->obj_version = read_u32(bytes, size, 0);
        cache->obj_count = read_u32(bytes, size, 4);
        cache->obj_ok = size >= 8 && cache->obj_version != 0;
        free(bytes);
    }

    join(path, sizeof(path), root, "WoWExt.dll");
    if (read_file(path, &bytes, &size)) {
        cache->dll_bytes = size;
        cache->dll_present = size > 2 && bytes[0] == 'M' && bytes[1] == 'Z';
        free(bytes);
    }

    return cache->bin_ok && cache->bin2_ok && cache->lua_ok;
}

void syn_free_cache(SynCache* cache) {
    free(cache->lua);
    cache->lua = nullptr;
}
