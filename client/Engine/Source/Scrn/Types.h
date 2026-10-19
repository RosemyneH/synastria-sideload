#ifndef ENGINE_SOURCE_SCRN_TYPES_H
#define ENGINE_SOURCE_SCRN_TYPES_H

#include <cstdint>

enum SCRNSTOCK {
    STOCK_SYSFONT = 0,
    STOCK_PERFFONT = 1,
    SCRNSTOCKOBJECTS = 2,
};

struct HLAYER__ {
    /* 0000 */ int32_t m_unused;
};

typedef HLAYER__* HLAYER;

#endif
