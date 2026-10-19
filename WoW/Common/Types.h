#ifndef WOW_COMMON_TYPES_H
#define WOW_COMMON_TYPES_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Gx/Types.h"

class CAaBspNode {
    public:
    /* 0000 */ uint16_t m_flags;
    /* 0002 */ uint16_t m_negChild;
    /* 0004 */ uint16_t m_posChild;
    /* 0006 */ uint16_t m_nFaces;
    /* 0008 */ uint32_t m_faceStart;
    /* 000C */ float m_planeDist;
};

#endif
