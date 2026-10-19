#ifndef WOW_SOURCE_OBJECT_CONTAINER_H
#define WOW_SOURCE_OBJECT_CONTAINER_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/WGUID.h"

class CGContainerData;

class CGContainer {
    public:
    static const uint32_t s_mirrorFields[72];

    static uint32_t DescriptorToMirrorOffset(uint32_t offset, uint32_t size, int32_t local, uint32_t base);
    static uint32_t GetSavedIndexFromDataIndex(uint32_t index, uint32_t count, int32_t local);
    static int32_t TotalFieldsSaved();

    /* 0000 */ CGContainerData* m_container;
    /* 0004 */ void* m_containerMirror;
};

class CGContainerData {
    public:
    /* 0000 */ uint32_t m_containerFieldNumSlots;
    /* 0004 */ uint8_t m_containerAlignPad [4];
    /* 0008 */ WOWGUID m_containerFieldSlot1 [36];
};

#endif
