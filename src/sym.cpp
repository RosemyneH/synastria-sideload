#include "sym.h"

#include <cstdio>
#include <cstring>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

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

}  // namespace

bool syn_resolve(const char* name, void** out) {
    *out = nullptr;

    int fd = open("/proc/self/exe", O_RDONLY);
    if (fd < 0) {
        return false;
    }

    struct stat st;
    if (fstat(fd, &st) != 0 || st.st_size < static_cast<off_t>(sizeof(Elf64_Ehdr))) {
        close(fd);
        return false;
    }

    void* map = mmap(nullptr, static_cast<size_t>(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (map == MAP_FAILED) {
        return false;
    }

    auto* ehdr = static_cast<Elf64_Ehdr*>(map);
    bool ok = false;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) == 0 && ehdr->e_ident[EI_CLASS] == ELFCLASS64) {
        auto* shdrs = reinterpret_cast<Elf64_Shdr*>(static_cast<char*>(map) + ehdr->e_shoff);
        const char* shstr = static_cast<char*>(map) + shdrs[ehdr->e_shstrndx].sh_offset;
        (void)shstr;

        uint64_t bias = load_bias();
        uint64_t first_vaddr = 0;
        auto* phdrs = reinterpret_cast<Elf64_Phdr*>(static_cast<char*>(map) + ehdr->e_phoff);
        for (int i = 0; i < ehdr->e_phnum; ++i) {
            if (phdrs[i].p_type == PT_LOAD) {
                first_vaddr = phdrs[i].p_vaddr;
                break;
            }
        }

        for (int i = 0; i < ehdr->e_shnum && !ok; ++i) {
            if (shdrs[i].sh_type != SHT_SYMTAB) {
                continue;
            }
            auto* syms = reinterpret_cast<Elf64_Sym*>(static_cast<char*>(map) + shdrs[i].sh_offset);
            const char* str = static_cast<char*>(map) + shdrs[shdrs[i].sh_link].sh_offset;
            size_t count = shdrs[i].sh_size / sizeof(Elf64_Sym);
            for (size_t s = 0; s < count; ++s) {
                if (syms[s].st_shndx == SHN_UNDEF || syms[s].st_name == 0) {
                    continue;
                }
                if (strcmp(str + syms[s].st_name, name) != 0) {
                    continue;
                }
                *out = reinterpret_cast<void*>(bias - first_vaddr + syms[s].st_value);
                ok = true;
                break;
            }
        }
    }

    munmap(map, static_cast<size_t>(st.st_size));
    return ok;
}
