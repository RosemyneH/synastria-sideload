#ifndef WOW_SOURCE_OBJECT_ITEMTEXTCACHE_H
#define WOW_SOURCE_OBJECT_ITEMTEXTCACHE_H

#include <cstddef>
#include <cstdint>

class CDataStore;

class ItemTextCache_C {
    public:
    static const uint32_t s_cacheVersion = 1;

    /* 0000 */ char m_text[8000];

    ItemTextCache_C();
    void Pack(CDataStore* msg);
    void Unpack(CDataStore* msg);
};

#endif
