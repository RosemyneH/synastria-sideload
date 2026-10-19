#include "BLLLoader.h"

// INLINED
BLLLoader::~BLLLoader() {
    this->Unload();
}

// VERIFY: 41b5b6fb manual
// OFFSET: 0x00872320
void* BLLLoader::GetExport(int32_t ordinal) {
    uint8_t* image = this->m_image;

    if (!image) {
        return nullptr;
    }

    const uint32_t* header = reinterpret_cast<const uint32_t*>(image);
    uint32_t index = ordinal - header[6];

    if (index >= header[5]) {
        return nullptr;
    }

    return image + *reinterpret_cast<const uint32_t*>(image + header[4] + index * 4);
}
