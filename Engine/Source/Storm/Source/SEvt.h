#ifndef ENGINE_SOURCE_STORM_SOURCE_SEVT_H
#define ENGINE_SOURCE_STORM_SOURCE_SEVT_H

#include <cstdint>
#include "Engine/Source/Storm/H/storm.h"

struct _IDHASHENTRY {
    /* 0000 */ uint32_t id;
    /* 0004 */ uint32_t sequence;
    /* 0008 */ void (STORMAPI* handler)(void*);
    /* 000C */ _IDHASHENTRY* next;
};

struct _IDHASHTABLE {
    /* 0000 */ _IDHASHENTRY** data;
    /* 0004 */ uint32_t size;
    /* 0008 */ uint32_t used;
    /* 000C */ _IDHASHTABLE* next;
};

struct _TYPEHASHENTRY {
    /* 0000 */ uint32_t type;
    /* 0004 */ uintptr_t subtype;
    /* 0008 */ uint32_t sequence;
    /* 000C */ _IDHASHTABLE* idhashtable;
    /* 0010 */ _TYPEHASHENTRY* next;
};

#endif
