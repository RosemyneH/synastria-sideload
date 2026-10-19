#ifndef ENGINE_SOURCE_OS_BLLLOADER_H
#define ENGINE_SOURCE_OS_BLLLOADER_H

#include <cstddef>
#include <cstdint>

class BLLLoader {
    public:
    /* 0000 */ uint8_t* m_image;
    /* 0004 */ uint32_t m_imageSize;
    /* 0008 */ int32_t m_loadedDlls;

    ~BLLLoader();
    void* GetExport(int32_t ordinal);
    bool Load(const void* data, uint32_t size);
    void Unload();
};

#endif
