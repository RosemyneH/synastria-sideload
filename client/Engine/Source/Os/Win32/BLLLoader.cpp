#if defined(WOWDECOMP_ARCH_32)

#include <windows.h>
#include <cstdio>
#include <cstring>
#include "Engine/Source/Os/BLLLoader.h"
#include "Engine/Source/Storm/H/storm.h"

// VERIFY: f3eb8a26 auto
// OFFSET: 0x008722C0
void BLLLoader::Unload() {
    if (!this->m_image) {
        return;
    }

    const uint32_t* imports = reinterpret_cast<const uint32_t*>(this->m_image + reinterpret_cast<const uint32_t*>(this->m_image)[7]);

    while (this->m_loadedDlls > 0) {
        this->m_loadedDlls--;
        FreeLibrary(GetModuleHandleA(reinterpret_cast<const char*>(this->m_image + imports[this->m_loadedDlls * 2])));
    }

    VirtualFree(this->m_image, 0, MEM_RELEASE);
    this->m_image = nullptr;
    this->m_imageSize = 0;
}

// VERIFY: 0c34f1cb manual
// OFFSET: 0x00872350
bool BLLLoader::Load(const void* data, uint32_t size) {
    bool failed = true;

    __try {
        const uint32_t* source = static_cast<const uint32_t*>(data);
        uint32_t imageSize = source[0];
        this->m_imageSize = imageSize;

        if (imageSize >= 0x80000000) {
            char buffer[1024] = "Improper header received: [ ";
            char* end = &buffer[strlen(buffer)];

            for (uint32_t i = 0; i < 32; i++) {
                sprintf(end, "%02X ", static_cast<const uint8_t*>(data)[i]);
                end += strlen(end);
            }

            SErrDisplayAppFatal(buffer);
        }

        this->m_image = static_cast<uint8_t*>(VirtualAlloc(nullptr, imageSize, MEM_COMMIT, PAGE_READWRITE));

        if (this->m_image) {
            memcpy(this->m_image, data, 40);

            const uint32_t* header = reinterpret_cast<const uint32_t*>(this->m_image);
            const uint8_t* chunk = reinterpret_cast<const uint8_t*>(&source[3 * header[9] + 10]);
            uint8_t* dest = this->m_image + source[10];
            uint8_t* end = this->m_image + header[0];

            for (bool copy = true; dest < end; copy = !copy) {
                uint32_t length = *reinterpret_cast<const uint16_t*>(chunk);
                chunk += 2;

                if (copy) {
                    memcpy(dest, chunk, length);
                    chunk += length;
                }

                dest += length;
            }

            const uint8_t* relocation = this->m_image + header[2];
            uint32_t offset = 0;

            for (uint32_t i = 0; i < header[3]; i++) {
                if (relocation[0] & 0x80) {
                    offset = relocation[3] + ((relocation[2] + ((relocation[1] + ((relocation[0] & 0x7F) << 8)) << 8)) << 8);
                    relocation += 4;
                } else {
                    offset += relocation[1] + (relocation[0] << 8);
                    relocation += 2;
                }

                *reinterpret_cast<uint32_t*>(this->m_image + offset) += reinterpret_cast<uint32_t>(this->m_image);
            }

            const uint32_t* imports = reinterpret_cast<const uint32_t*>(this->m_image + header[7]);

            for (uint32_t i = 0; i < header[8]; i++) {
                HMODULE library = LoadLibraryA(reinterpret_cast<const char*>(this->m_image + imports[i * 2]));

                if (!library) {
                    __leave;
                }

                for (int32_t* thunk = reinterpret_cast<int32_t*>(this->m_image + imports[i * 2 + 1]); *thunk; thunk++) {
                    if (*thunk >= 0) {
                        *thunk = reinterpret_cast<int32_t>(GetProcAddress(library, reinterpret_cast<const char*>(this->m_image + *thunk)));
                    } else {
                        *thunk = reinterpret_cast<int32_t>(GetProcAddress(library, reinterpret_cast<const char*>(*thunk & 0x7FFFFFFF)));
                    }
                }

                this->m_loadedDlls++;
            }

            for (uint32_t i = 0; i < header[9]; i++) {
                uint8_t* address = this->m_image + source[3 * i + 10];
                uint32_t length = source[3 * i + 11];
                DWORD oldProtect;
                VirtualProtect(address, length, source[3 * i + 12], &oldProtect);

                if (source[3 * i + 12] & 0xF0) {
                    FlushInstructionCache(GetCurrentProcess(), address, length);
                }
            }

            uint32_t tail = header[2];

            if (tail < header[0]) {
                uint32_t start = (tail + 4095) & ~4095u;

                if (start >= tail && start < header[0]) {
                    VirtualFree(this->m_image + start, header[0] - start, MEM_DECOMMIT);
                }
            }

            failed = false;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    if (failed) {
        this->Unload();
    }

    return !failed;
}

#endif
