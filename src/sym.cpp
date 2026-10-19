#include "sym.h"

#include <cstdio>
#include <cstring>

#if !defined(_WIN32)
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace {

struct Bind {
    const char* name;
    void* addr;
};

Bind g_binds[192];
int g_bind_count = 0;

bool bind_find(const char* name, void** out) {
    for (int i = 0; i < g_bind_count; ++i) {
        if (strcmp(g_binds[i].name, name) == 0 && g_binds[i].addr) {
            *out = g_binds[i].addr;
            return true;
        }
    }
    return false;
}

#if !defined(_WIN32)
uint64_t load_bias() {
    char exe[4096];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (n <= 0) {
        return 0;
    }
    exe[n] = 0;

    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) {
        return 0;
    }

    char line[1024];
    uint64_t base = 0;
    while (fgets(line, sizeof(line), maps)) {
        unsigned long start = 0;
        unsigned long offset = 0;
        char perms[8] = {};
        char path[512] = {};
        if (sscanf(line, "%lx-%*x %7s %lx %*s %*s %511[^\n]", &start, perms, &offset, path) < 3) {
            continue;
        }
        if (offset == 0 && path[0] && strcmp(path, exe) == 0) {
            base = start;
            break;
        }
    }
    fclose(maps);
    return base;
}
#endif

}  // namespace

void syn_bind(const char* name, void* addr) {
    if (!name || !addr || g_bind_count >= static_cast<int>(sizeof(g_binds) / sizeof(g_binds[0]))) {
        return;
    }
    g_binds[g_bind_count].name = name;
    g_binds[g_bind_count].addr = addr;
    ++g_bind_count;
}

int syn_resolve_each(const char* name, void** out, int cap) {
#if defined(_WIN32)
    (void)name;
    (void)out;
    (void)cap;
    return 0;
#else
    if (!out || cap <= 0) {
        return 0;
    }

    int fd = open("/proc/self/exe", O_RDONLY);
    if (fd < 0) {
        return 0;
    }

    struct stat st;
    if (fstat(fd, &st) != 0 || st.st_size < static_cast<off_t>(sizeof(Elf64_Ehdr))) {
        close(fd);
        return 0;
    }

    void* map = mmap(nullptr, static_cast<size_t>(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (map == MAP_FAILED) {
        return 0;
    }

    int found = 0;
    auto* ehdr = static_cast<Elf64_Ehdr*>(map);
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) == 0 && ehdr->e_ident[EI_CLASS] == ELFCLASS64) {
        auto* shdrs = reinterpret_cast<Elf64_Shdr*>(static_cast<char*>(map) + ehdr->e_shoff);
        uint64_t bias = load_bias();
        uint64_t first_vaddr = 0;
        auto* phdrs = reinterpret_cast<Elf64_Phdr*>(static_cast<char*>(map) + ehdr->e_phoff);
        for (int i = 0; i < ehdr->e_phnum; ++i) {
            if (phdrs[i].p_type == PT_LOAD) {
                first_vaddr = phdrs[i].p_vaddr;
                break;
            }
        }

        for (int i = 0; i < ehdr->e_shnum && found < cap; ++i) {
            if (shdrs[i].sh_type != SHT_SYMTAB) {
                continue;
            }
            auto* syms = reinterpret_cast<Elf64_Sym*>(static_cast<char*>(map) + shdrs[i].sh_offset);
            const char* str = static_cast<char*>(map) + shdrs[shdrs[i].sh_link].sh_offset;
            size_t count = shdrs[i].sh_size / sizeof(Elf64_Sym);
            for (size_t s = 0; s < count && found < cap; ++s) {
                if (syms[s].st_shndx == SHN_UNDEF || syms[s].st_name == 0) {
                    continue;
                }
                if (strcmp(str + syms[s].st_name, name) != 0) {
                    continue;
                }
                out[found++] = reinterpret_cast<void*>(bias - first_vaddr + syms[s].st_value);
            }
        }
    }

    munmap(map, static_cast<size_t>(st.st_size));
    return found;
#endif
}

bool syn_resolve(const char* name, void** out) {
    *out = nullptr;
#if defined(__linux__)
    if (syn_resolve_each(name, out, 1) == 1) {
        return true;
    }
#endif
    return bind_find(name, out);
}
