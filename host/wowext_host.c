#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// ʕ •ᴥ•ʔ✿ WoWExt.dll is 32-bit and calls Wow.exe at fixed addresses. Wine already owns that range, so those sites are rewritten to shims in this process. ✿ ʕ •ᴥ•ʔ

typedef struct {
    uint32_t original;
    uint8_t* stub;
} Shim;

enum { kMaxShims = 4096 };

Shim g_shims[kMaxShims];
int g_shim_count = 0;
uint8_t* g_stub_mem = NULL;
int g_stub_used = 0;
void* g_image = NULL;

static uint8_t* shim_for(uint32_t original, int call) {
    for (int i = 0; i < g_shim_count; ++i) {
        if (g_shims[i].original == original) {
            return g_shims[i].stub;
        }
    }
    if (g_shim_count >= kMaxShims || !g_stub_mem) {
        return NULL;
    }
    uint8_t* stub = g_stub_mem + g_stub_used;
    g_stub_used += 32;
    // mov eax, original; ret  — cdecl stand-in until the bridge knows the signature
    stub[0] = 0xB8;
    memcpy(stub + 1, &original, 4);
    stub[5] = 0xC3;
    (void)call;
    g_shims[g_shim_count].original = original;
    g_shims[g_shim_count].stub = stub;
    ++g_shim_count;
    return stub;
}

static int in_wow(uint32_t addr) {
    return addr >= 0x00400000u && addr < 0x01000000u;
}

static void patch_text(uint8_t* text, uint32_t size, uint32_t va) {
    for (uint32_t i = 0; i + 5 < size; ++i) {
        uint32_t site = va + i;
        if (text[i] == 0xE8 || text[i] == 0xE9) {
            int32_t rel = 0;
            memcpy(&rel, text + i + 1, 4);
            uint32_t tgt = site + 5 + (uint32_t)rel;
            if (!in_wow(tgt)) {
                continue;
            }
            uint8_t* stub = shim_for(tgt, 1);
            if (!stub) {
                continue;
            }
            int32_t neu = (int32_t)((uint32_t)(uintptr_t)stub - (site + 5));
            memcpy(text + i + 1, &neu, 4);
            i += 4;
            continue;
        }
        if (text[i] == 0x68 || (text[i] >= 0xB8 && text[i] <= 0xBF)) {
            uint32_t imm = 0;
            memcpy(&imm, text + i + 1, 4);
            if (!in_wow(imm)) {
                continue;
            }
            uint8_t* stub = shim_for(imm, 0);
            if (!stub) {
                continue;
            }
            uint32_t neu = (uint32_t)(uintptr_t)stub;
            memcpy(text + i + 1, &neu, 4);
            i += 4;
        }
    }
}

static int map_dll(const char* path) {
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        printf("open failed %lu\n", GetLastError());
        return 0;
    }
    DWORD bytes = GetFileSize(file, NULL);
    uint8_t* raw = (uint8_t*)HeapAlloc(GetProcessHeap(), 0, bytes);
    DWORD got = 0;
    ReadFile(file, raw, bytes, &got, NULL);
    CloseHandle(file);
    if (got < 0x200 || raw[0] != 'M' || raw[1] != 'Z') {
        printf("not a pe\n");
        return 0;
    }
    uint32_t e_lfanew = 0;
    memcpy(&e_lfanew, raw + 0x3c, 4);
    uint8_t* nt = raw + e_lfanew;
    uint32_t image_base = 0;
    uint32_t image_size = 0;
    uint32_t header_size = 0;
    memcpy(&image_base, nt + 24 + 28, 4);
    memcpy(&image_size, nt + 24 + 56, 4);
    memcpy(&header_size, nt + 24 + 60, 4);
    uint16_t sections = 0;
    uint16_t opt_size = 0;
    memcpy(&sections, nt + 6, 2);
    memcpy(&opt_size, nt + 20, 2);
    g_image = VirtualAlloc((void*)(uintptr_t)image_base, image_size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    printf("image preferred %08x got %p size %x\n", image_base, g_image, image_size);
    if (!g_image) {
        return 0;
    }
    memset(g_image, 0, image_size);
    memcpy(g_image, raw, header_size);
    uint8_t* sec = nt + 24 + opt_size;
    for (uint16_t i = 0; i < sections; ++i) {
        uint32_t va = 0, raw_ptr = 0, raw_size = 0;
        memcpy(&va, sec + 12, 4);
        memcpy(&raw_size, sec + 16, 4);
        memcpy(&raw_ptr, sec + 20, 4);
        if (raw_size && raw_ptr + raw_size <= bytes) {
            memcpy((uint8_t*)g_image + va, raw + raw_ptr, raw_size);
        }
        sec += 40;
    }
    ptrdiff_t delta = (uint8_t*)g_image - (uint8_t*)(uintptr_t)image_base;
    if (delta) {
        uint32_t reloc_rva = 0, reloc_size = 0;
        memcpy(&reloc_rva, nt + 24 + 96 + 5 * 8, 4);
        memcpy(&reloc_size, nt + 24 + 96 + 5 * 8 + 4, 4);
        uint8_t* rel = (uint8_t*)g_image + reloc_rva;
        uint8_t* end = rel + reloc_size;
        while (rel + 8 <= end) {
            uint32_t page = 0, block = 0;
            memcpy(&page, rel, 4);
            memcpy(&block, rel + 4, 4);
            if (block < 8) {
                break;
            }
            uint32_t count = (block - 8) / 2;
            for (uint32_t i = 0; i < count; ++i) {
                uint16_t ent = 0;
                memcpy(&ent, rel + 8 + i * 2, 2);
                if ((ent >> 12) == 3) {
                    uint32_t* slot = (uint32_t*)((uint8_t*)g_image + page + (ent & 0xfff));
                    *slot += (uint32_t)delta;
                }
            }
            rel += block;
        }
    }
    uint32_t import_rva = 0;
    memcpy(&import_rva, nt + 24 + 96 + 1 * 8, 4);
    if (import_rva) {
        uint8_t* imp = (uint8_t*)g_image + import_rva;
        for (;;) {
            uint32_t orig = 0, name_rva = 0, thunk = 0;
            memcpy(&orig, imp, 4);
            memcpy(&name_rva, imp + 12, 4);
            memcpy(&thunk, imp + 16, 4);
            if (!name_rva || !thunk) {
                break;
            }
            const char* dll = (const char*)g_image + name_rva;
            HMODULE mod = LoadLibraryA(dll);
            printf("import %s %p\n", dll, mod);
            uint32_t* iat = (uint32_t*)((uint8_t*)g_image + thunk);
            uint32_t* hint = (uint32_t*)((uint8_t*)g_image + (orig ? orig : thunk));
            for (; *hint; ++hint, ++iat) {
                FARPROC proc = NULL;
                if (*hint & 0x80000000u) {
                    proc = GetProcAddress(mod, (LPCSTR)(uintptr_t)(*hint & 0xffff));
                } else {
                    const char* fname = (const char*)g_image + *hint + 2;
                    proc = GetProcAddress(mod, fname);
                }
                *iat = (uint32_t)(uintptr_t)proc;
            }
            imp += 20;
        }
    }
    g_stub_mem = (uint8_t*)VirtualAlloc(NULL, 0x200000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    printf("shims %p\n", g_stub_mem);
    uint8_t* mapped_nt = (uint8_t*)g_image + e_lfanew;
    uint16_t mapped_sections = 0, mapped_opt = 0;
    memcpy(&mapped_sections, mapped_nt + 6, 2);
    memcpy(&mapped_opt, mapped_nt + 20, 2);
    uint8_t* mapped_sec = mapped_nt + 24 + mapped_opt;
    for (uint16_t i = 0; i < mapped_sections; ++i) {
        char name[9] = {};
        memcpy(name, mapped_sec, 8);
        uint32_t va = 0, vsize = 0;
        memcpy(&vsize, mapped_sec + 8, 4);
        memcpy(&va, mapped_sec + 12, 4);
        if (name[1] == 't') {
            patch_text((uint8_t*)g_image + va, vsize, (uint32_t)(uintptr_t)g_image + va);
        }
        mapped_sec += 40;
    }
    printf("rewrote %d wow sites\n", g_shim_count);
    uint32_t entry = 0;
    memcpy(&entry, nt + 24 + 16, 4);
    BOOL(WINAPI * DllMain)(HINSTANCE, DWORD, LPVOID) =
        (BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID))((uint8_t*)g_image + entry);
    printf("dllmain %p\n", (void*)DllMain);
    BOOL ok = DllMain((HINSTANCE)g_image, DLL_PROCESS_ATTACH, NULL);
    printf("dllmain returned %d\n", ok);
    HeapFree(GetProcessHeap(), 0, raw);
    return ok ? 1 : 0;
}

static LONG WINAPI on_fault(EXCEPTION_POINTERS* info) {
    printf("init stopped at %p code %08lx\n",
           info->ExceptionRecord->ExceptionAddress,
           info->ExceptionRecord->ExceptionCode);
    fflush(stdout);
    ExitProcess(2);
    return EXCEPTION_EXECUTE_HANDLER;
}

int main(void) {
    SetUnhandledExceptionFilter(on_fault);
    const char* path = "Z:\\home\\emma\\Games\\Synastria\\WoWExt.dll";
    if (!map_dll(path)) {
        return 1;
    }
    printf("WoWExt.dll is resident\n");
    return 0;
}
