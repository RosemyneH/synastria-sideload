#ifndef WOW_SOURCE_SOUNDINTERFACE_SOUNDINTERFACE2EMOTES_H
#define WOW_SOURCE_SOUNDINTERFACE_SOUNDINTERFACE2EMOTES_H

#include <cstddef>
#include <cstdint>

struct TEXTEMOTELOOKUP {
    /* 0000 */ int32_t m_soundIds[22][2];
    /* 00B0 */ int32_t m_emotesTextId;

    TEXTEMOTELOOKUP();
};

#endif
